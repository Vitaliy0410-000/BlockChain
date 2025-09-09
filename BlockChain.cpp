#include "BlockChain.h"
#include <vector>
#include <memory>
#include "BlockFactory.h"
#include "Logger.h"
#include <ctime>
#include <string>
#include "RegularBlock.h"
#include <mutex>
#include <nlohmann/json.hpp>
#include "Transaction.h"
#include "Virtual_Machine.h"
#include "SmartContractParser.h"
#include "Node.h"

Blockchain::Blockchain(int difficulty,const std::string& host, unsigned short port)
{
    if (difficulty <= 0)
    {
        throw std::invalid_argument("Difficulty must be positive!");
    }
    this->difficulty = difficulty;
    auto genesis = GenesisFactory().createGenesis();
    if (!genesis)
    {
        throw std::runtime_error("Failed to create Genesis block");
    }
    genesis->mineBlockParallel(difficulty, 4);
    chain.push_back(std::move(genesis));
    Logger::getInstance().log("Initialized Blockchain with Genesis block");
    globalState["Alice"] = 10000; // Инициализируем в центах (100.0 * 100)
    globalState["Bob"] = 5000;   // Инициализируем в центах (50.0 * 100)
    node = new Node(host, port, *this);//создание ноды
    Logger::getInstance().log("Create node");
}

Blockchain* Blockchain::instance = nullptr;

std::mutex Blockchain::instanceMutex;
std::mutex Blockchain::chainMutex;

void Blockchain::addTransaction(const Transaction& tx)
{
    transactionPool.push_back(tx);
     Logger::getInstance().log("add transaction in transaction pool");
    node->broadcastTransaction(tx);
     if(transactionPool.size()>=2)
    {
        addBlock(transactionPool);
         transactionPool.clear();
    }

}

void Blockchain::addBlock(std::vector<Transaction> transactions)
{
    if (transactions.empty())
    {
        throw std::invalid_argument("Transactions list cannot be empty!");
    }
    Logger::getInstance().log("Chain size: " + std::to_string(chain.size()));

    // Создаём вектор для транзакций с парсированным байт-кодом
    std::vector<Transaction> parsedTransactions;
    for (const auto& tx : transactions)
    {
        if (!tx.getGltchCode().empty())
        {
            // Создаём контекст для парсера
            Virtual_Machine::Context ctx =
                {
                    tx.getSender(),
                    static_cast<int64_t>(tx.getAmount() * 100), // Конвертируем в центы
                    globalState[tx.getSender()],
                    std::time(nullptr),
                    static_cast<int>(chain.size())
                };
            // Парсим GLTCH код в байт-код
            try
            {
                SmartContractParser parser(ctx);
                std::vector<uint8_t> bytecode = parser.parse(tx.getGltchCode());
                // Создаём новую транзакцию с байт-кодом
                parsedTransactions.emplace_back(
                    tx.getSender(),
                    tx.getRecipient(),
                    tx.getAmount(),
                    tx.getSignature(),
                    bytecode,
                    tx.getGasLimit(),
                    tx.getGltchCode()
                    );
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().log("Parser error for transaction: " + tx.toString() + ", error: " + e.what());
                continue; // Пропускаем транзакцию при ошибке парсинга
            }
        }
        else
        {
            // Если GLTCH код пустой, используем исходную транзакцию
            parsedTransactions.push_back(tx);
        }
    }

    // Создаём блок с парсированными транзакциями
    auto block = RegularBlockFactory().createRegularBlock(chain.size(), std::time(nullptr), parsedTransactions, chain.back()->getHash(), 0);
    if (!block)
    {
        throw std::runtime_error("Failed to create RegularBlock");
    }
    block->mineBlockParallel(difficulty, 4);

    // Выполняем контракты и обновляем globalState
    for (const auto& tx : parsedTransactions)
    {
        if (!tx.getContractCode().empty())
        {
            Logger::getInstance().log("Executing contract for transaction: " + tx.toString());
            Virtual_Machine::Context ctx =
                {
                    tx.getSender(),
                    static_cast<int64_t>(tx.getAmount() * 100), // Конвертируем в центы
                    globalState[tx.getSender()],
                    std::time(nullptr),
                    static_cast<int>(chain.size())
                };
            Virtual_Machine vm(tx.getContractCode(), globalState, ctx, {}, tx.getGasLimit());
            bool success = vm.execute();
            if (success)
            {
                for (const auto& event : vm.getLog())
                {
                    Logger::getInstance().log("Event: " + event);
                }
            }
            else
            {
                Logger::getInstance().log("Contract execution failed for transaction: " + tx.toString());
            }
        }
        // Обновляем globalState на основе amount
        globalState[tx.getSender()] -= static_cast<int64_t>(tx.getAmount() * 100); // Уменьшаем баланс отправителя
        globalState[tx.getRecipient()] += static_cast<int64_t>(tx.getAmount() * 100); // Увеличиваем баланс получателя
    }

    chain.push_back(std::move(block));
    Logger::getInstance().log("Added block with index=" + std::to_string(chain.size() - 1));
}

bool Blockchain::isChainValid()
{
    Logger::getInstance().log("Validating chain with size: " + std::to_string(chain.size()));
    for (size_t i = 1; i < chain.size(); ++i)
    {
        if (chain[i]->getPrevHash() != chain[i-1]->getHash())
        {
            Logger::getInstance().log("Chain invalid at index=" + std::to_string(i));
            return false;
        }
    }
    if (chain.empty())
    {
        return false;
    }
    Logger::getInstance().log("Chain is valid");
    return true;
}

std::string Blockchain::getChainInfo() const
{
    std::lock_guard<std::mutex> lock(chainMutex);
    nlohmann::json array = nlohmann::json::array();
    Logger::getInstance().log("getChainInfo: chain size = " + std::to_string(chain.size()));
    for (const auto& block : chain)
    {
        Logger::getInstance().log("Processing block with index=" + std::to_string(block->getIndex()));
        nlohmann::json obj;
        obj["index"] = block->getIndex();
        obj["hash"] = block->getHash();
        nlohmann::json txArray = nlohmann::json::array();
        auto transactions = block->getTransactions();
        Logger::getInstance().log("Block has " + std::to_string(transactions.size()) + " transactions");
        for (const auto& tx : transactions)
        {
            nlohmann::json txObj;
            txObj["sender"] = tx.getSender();
            txObj["recipient"] = tx.getRecipient();
            txObj["amount"] = tx.getAmount();
            txObj["signature"] = tx.getSignature();
            if (!tx.getContractCode().empty())
            {
                std::stringstream ss;
                for (size_t i = 0; i < tx.getContractCode().size(); ++i)
                {
                    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tx.getContractCode()[i]);
                    if (i < tx.getContractCode().size() - 1) ss << " ";
                }
                txObj["contractCode"] = ss.str();
            }
            if (!tx.getGltchCode().empty())
            {
                txObj["gltchCode"] = tx.getGltchCode();
            }
            txArray.push_back(txObj);
        }
        obj["transactions"] = txArray;
        array.push_back(obj);
    }
    std::string result = array.dump(4);
    Logger::getInstance().log("Chain info: " + result);
    return result;
}

Blockchain& Blockchain::getInstance(int difficulty,const std::string& host, unsigned short port)
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr)
    {
        instance = new Blockchain(difficulty, host, port);
    }
    else if (instance->difficulty != difficulty)
    {
        Logger::getInstance().log("Warning: Blockchain instance already exists with different difficulty");
    }
    Logger::getInstance().log("Blockchain getInstance called");
    return *instance;
}

Blockchain::~Blockchain()
{
    chain.clear();
    delete node;
}

const std::map<std::string, int64_t>& Blockchain::getGlobalState() const
{
    return globalState;
}

void Blockchain::testBlockchain()
{
    std::vector<Transaction> transactions;
    // Тест 1: PUSH 100, HALT
    std::vector<uint8_t> bytecode1 = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};
    transactions.emplace_back("Alice", "Bob", 10.0, "sig1", bytecode1, 1000, "");
    // Тест 2: PUSH "Bob", PUSH 100, SSTORE
    std::vector<uint8_t> bytecode2 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x42, 0x6F, 0x62, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x0C};
    transactions.emplace_back("Bob", "Alice", 20.0, "sig2", bytecode2, 1000, "");
    // Тест 3: PUSH "Alice", PUSH 50, SSTORE
    std::vector<uint8_t> bytecode3 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x05, 0x41, 0x6C, 0x69, 0x63, 0x65, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x0C};
    transactions.emplace_back("Alice", "Bob", 15.0, "sig3", bytecode3, 1000, "");
    // Тест 4: PUSH "Bob", PUSH 10, SSTORE
    std::vector<uint8_t> bytecode4 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x42, 0x6F, 0x62, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0C};
    transactions.emplace_back("Bob", "Alice", 5.0, "sig4", bytecode4, 1000, "");

    Logger::getInstance().log("Running testBlockchain with " + std::to_string(transactions.size()) + " transactions");
    addBlock(transactions);
    Logger::getInstance().log("GlobalState after tests: Alice=" + std::to_string(globalState["Alice"] / 100.0) +
                              ", Bob=" + std::to_string(globalState["Bob"] / 100.0));
}

Transaction Blockchain::createTransactionWithGLTCH(std::string sender, std::string recipient, double amount,
                                                   std::string signature, std::string gltchCode, int64_t gasLimit, Virtual_Machine::Context ctx)
{
    // Создаём парсер с контекстом
    SmartContractParser parser(ctx);
    // Парсим GLTCH код в байт-код
    std::vector<uint8_t> bytecode;
    try
    {
        bytecode = parser.parse(gltchCode);
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().log("Parser error: " + std::string(e.what()));
        throw std::runtime_error("Failed to parse GLTCH code: " + std::string(e.what()));
    }
    // Создаём транзакцию с байт-кодом
    return Transaction(sender, recipient, amount, signature, bytecode, gasLimit, gltchCode);
}
