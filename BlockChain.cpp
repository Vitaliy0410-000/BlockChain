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

using json = nlohmann::json;

Blockchain* Blockchain::instance = nullptr;
std::mutex Blockchain::instanceMutex;
std::mutex Blockchain::chainMutex;

Blockchain::Blockchain(int difficulty, const std::string& host, unsigned short port)
    : difficulty(difficulty) {
    if (difficulty <= 0) {
        throw std::invalid_argument("Difficulty must be positive!");
    }
    auto genesis = GenesisFactory().createGenesis();
    if (!genesis) {
        throw std::runtime_error("Failed to create Genesis block");
    }
    genesis->mineBlockParallel(difficulty, 4);
    chain.push_back(std::move(genesis));
    Logger::getInstance().log("Initialized Blockchain with Genesis block");
    globalState["Alice"] = 10000; // Инициализируем в центах (100.0 * 100)
    globalState["Bob"] = 5000;   // Инициализируем в центах (50.0 * 100)
    node = new Node(host, port, *this); // Создание ноды
    Logger::getInstance().log("Create node");
}

Blockchain::~Blockchain() {
    try {
        if (node) {
            Logger::getInstance().log("Deleting node in Blockchain destructor");
            delete node;
            node = nullptr;
        }
        chain.clear();
        Logger::getInstance().log("Blockchain resources cleaned up");
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error in Blockchain destructor: " + std::string(e.what()));
    }
}

Blockchain& Blockchain::getInstance(int difficulty, const std::string& host, unsigned short port) {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new Blockchain(difficulty, host, port);
    } else if (instance->difficulty != difficulty) {
        Logger::getInstance().log("Warning: Blockchain instance already exists with different difficulty");
    }
    Logger::getInstance().log("Blockchain getInstance called");
    return *instance;
}

void Blockchain::startNode() {
    if (node) {
        Logger::getInstance().log("Starting node from Blockchain");
        node->start();
    } else {
        throw std::runtime_error("Node is not initialized");
    }
}

void Blockchain::connectToPeer(const std::string& host, unsigned short port) {
    if (node) {
        Logger::getInstance().log("Connecting to peer from Blockchain: " + host + ":" + std::to_string(port));
        node->connectToPeer(host, port);
    } else {
        throw std::runtime_error("Node is not initialized");
    }
}

void Blockchain::addTransaction(const Transaction& tx) {
    std::lock_guard<std::mutex> lock(chainMutex);
    transactionPool.push_back(tx);
    Logger::getInstance().log("Added transaction to pool: " + tx.toString());
    if (node) {
        node->broadcastTransaction(tx);
    }
    if (transactionPool.size() >= 2) {
        addBlock(transactionPool);
        transactionPool.clear();
    }
}

void Blockchain::addBlock(std::vector<Transaction> transactions) {
    std::lock_guard<std::mutex> lock(chainMutex);
    if (transactions.empty()) {
        throw std::invalid_argument("Transactions list cannot be empty!");
    }
    Logger::getInstance().log("Chain size: " + std::to_string(chain.size()));

    // Создаём вектор для транзакций с парсированным байт-кодом
    std::vector<Transaction> parsedTransactions;
    for (const auto& tx : transactions) {
        if (!tx.getGltchCode().empty()) {
            Virtual_Machine::Context ctx = {
                tx.getSender(),
                static_cast<int64_t>(tx.getAmount() * 100),
                globalState[tx.getSender()],
                std::time(nullptr),
                static_cast<int>(chain.size())
            };
            try {
                SmartContractParser parser(ctx);
                std::vector<uint8_t> bytecode = parser.parse(tx.getGltchCode());
                parsedTransactions.emplace_back(
                    tx.getSender(),
                    tx.getRecipient(),
                    tx.getAmount(),
                    tx.getSignature(),
                    bytecode,
                    tx.getGasLimit(),
                    tx.getGltchCode()
                    );
            } catch (const std::exception& e) {
                Logger::getInstance().log("Parser error for transaction: " + tx.toString() + ", error: " + e.what());
                continue;
            }
        } else {
            parsedTransactions.push_back(tx);
        }
    }

    auto block = RegularBlockFactory().createRegularBlock(chain.size(), std::time(nullptr), parsedTransactions, chain.back()->getHash(), 0);
    if (!block) {
        throw std::runtime_error("Failed to create RegularBlock");
    }
    block->mineBlockParallel(difficulty, 4);

    for (const auto& tx : parsedTransactions) {
        if (!tx.getContractCode().empty()) {
            Logger::getInstance().log("Executing contract for transaction: " + tx.toString());
            Virtual_Machine::Context ctx = {
                tx.getSender(),
                static_cast<int64_t>(tx.getAmount() * 100),
                globalState[tx.getSender()],
                std::time(nullptr),
                static_cast<int>(chain.size())
            };
            Virtual_Machine vm(tx.getContractCode(), globalState, ctx, {}, tx.getGasLimit());
            bool success = vm.execute();
            if (success) {
                for (const auto& event : vm.getLog()) {
                    Logger::getInstance().log("Event: " + event);
                }
            } else {
                Logger::getInstance().log("Contract execution failed for transaction: " + tx.toString());
            }
        }
        globalState[tx.getSender()] -= static_cast<int64_t>(tx.getAmount() * 100);
        globalState[tx.getRecipient()] += static_cast<int64_t>(tx.getAmount() * 100);
    }

    chain.push_back(std::move(block));
    Logger::getInstance().log("Added block with index=" + std::to_string(chain.size() - 1));
    if (node) {
        node->broadcastBlock(chain.back());
    }
}

bool Blockchain::isChainValid() {
    std::lock_guard<std::mutex> lock(chainMutex);
    Logger::getInstance().log("Validating chain with size: " + std::to_string(chain.size()));
    for (size_t i = 1; i < chain.size(); ++i) {
        if (chain[i]->getPrevHash() != chain[i - 1]->getHash()) {
            Logger::getInstance().log("Chain invalid at index=" + std::to_string(i));
            return false;
        }
    }
    if (chain.empty()) {
        return false;
    }
    Logger::getInstance().log("Chain is valid");
    return true;
}

std::string Blockchain::getChainInfo() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    json array = json::array();
    Logger::getInstance().log("getChainInfo: chain size = " + std::to_string(chain.size()));
    for (const auto& block : chain) {
        Logger::getInstance().log("Processing block with index=" + std::to_string(block->getIndex()));
        json obj;
        obj["index"] = block->getIndex();
        obj["hash"] = block->getHash();
        json txArray = json::array();
        auto transactions = block->getTransactions();
        Logger::getInstance().log("Block has " + std::to_string(transactions.size()) + " transactions");
        for (const auto& tx : transactions) {
            json txObj;
            txObj["sender"] = tx.getSender();
            txObj["recipient"] = tx.getRecipient();
            txObj["amount"] = tx.getAmount();
            txObj["signature"] = tx.getSignature();
            if (!tx.getContractCode().empty()) {
                std::stringstream ss;
                for (size_t i = 0; i < tx.getContractCode().size(); ++i) {
                    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tx.getContractCode()[i]);
                    if (i < tx.getContractCode().size() - 1) ss << " ";
                }
                txObj["contractCode"] = ss.str();
            }
            if (!tx.getGltchCode().empty()) {
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

std::map<std::string, int64_t> Blockchain::getGlobalState() {
    return globalState;
}

void Blockchain::testBlockchain() {
    std::vector<Transaction> transactions;
    std::vector<uint8_t> bytecode1 = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};
    transactions.emplace_back("Alice", "Bob", 10.0, "sig1", bytecode1, 1000, "");
    std::vector<uint8_t> bytecode2 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x42, 0x6F, 0x62, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x0C};
    transactions.emplace_back("Bob", "Alice", 20.0, "sig2", bytecode2, 1000, "");
    std::vector<uint8_t> bytecode3 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x05, 0x41, 0x6C, 0x69, 0x63, 0x65, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x0C};
    transactions.emplace_back("Alice", "Bob", 15.0, "sig3", bytecode3, 1000, "");
    std::vector<uint8_t> bytecode4 = {0x01, 0x01, 0x00, 0x00, 0x00, 0x03, 0x42, 0x6F, 0x62, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0C};
    transactions.emplace_back("Bob", "Alice", 5.0, "sig4", bytecode4, 1000, "");

    Logger::getInstance().log("Running testBlockchain with " + std::to_string(transactions.size()) + " transactions");
    addBlock(transactions);
    Logger::getInstance().log("GlobalState after tests: Alice=" + std::to_string(globalState["Alice"] / 100.0) +
                              ", Bob=" + std::to_string(globalState["Bob"] / 100.0));
}

Transaction Blockchain::createTransactionWithGLTCH(std::string sender, std::string recipient, double amount,
                                                   std::string signature, std::string gltchCode, int64_t gasLimit,
                                                   Virtual_Machine::Context ctx) {
    SmartContractParser parser(ctx);
    std::vector<uint8_t> bytecode;
    try {
        bytecode = parser.parse(gltchCode);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Parser error: " + std::string(e.what()));
        throw std::runtime_error("Failed to parse GLTCH code: " + std::string(e.what()));
    }
    return Transaction(sender, recipient, amount, signature, bytecode, gasLimit, gltchCode);
}
