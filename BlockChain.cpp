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

Blockchain::Blockchain(int difficulty)
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
    globalState["Alice"] = 100.0;
    globalState["Bob"] = 50.0;
}

Blockchain* Blockchain::instance = nullptr;

std::mutex Blockchain::instanceMutex;
std::mutex Blockchain::chainMutex;

void Blockchain::addBlock(std::vector<Transaction> transactions)
{
    if (transactions.empty())
    {
        throw std::invalid_argument("Transactions list cannot be empty!");
    }
    Logger::getInstance().log("Chain size: " + std::to_string(chain.size()));
    auto block = RegularBlockFactory().createRegularBlock(chain.size(), std::time(nullptr), transactions, chain.back()->getHash(), 0);
    if (!block)
    {
        throw std::runtime_error("Failed to create RegularBlock");
    }
    block->mineBlockParallel(difficulty, 4);
    for (const auto& tx : transactions) {
        if (!tx.getContractCode().empty()) {
            Logger::getInstance().log("Executing contract for transaction: " + tx.toString());
            SmartContract contract(tx.getContractCode(), globalState);
            contract.execute();
            globalState = contract.getState();
        }
    }
    chain.push_back(std::move(block));
    Logger::getInstance().log("Added block with index=" + std::to_string(chain.size() - 1));
}

bool Blockchain::isChainValid()
{
    Logger::getInstance().log("Validating chain with size: " + std::to_string(chain.size()));
    for (size_t i = 1; i < chain.size(); ++i)
    {
        if (chain[i]->getPrevHash() != chain[i - 1]->getHash())
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

std::string Blockchain::getChainInfo() const {
    std::lock_guard<std::mutex> lock(chainMutex);
    nlohmann::json array = nlohmann::json::array();
    for (const auto& block : chain) {
        nlohmann::json obj;
        obj["index"] = block->getIndex();
        obj["hash"] = block->getHash();
        nlohmann::json txArray = nlohmann::json::array();
        for (const auto& tx : block->getTransactions()) {
            nlohmann::json txObj;
            txObj["sender"] = tx.getSender();
            txObj["recipient"] = tx.getRecipient();
            txObj["amount"] = tx.getAmount();
            txObj["signature"] = tx.getSignature();
            if (!tx.getContractCode().empty()) {
                txObj["contractCode"] = tx.getContractCode();
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

Blockchain& Blockchain::getInstance(int difficulty)
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr)
    {
        instance = new Blockchain(difficulty);
    }
    else if (instance->difficulty != difficulty)
    {
        Logger::getInstance().log("Warning: Blockchain instance already exists with different difficulty");
    }
    Logger::getInstance().log("Blockchain getInstance called");
    return *instance;
}

Blockchain::~Blockchain() {
    chain.clear();
}

const std::map<std::string, double>& Blockchain::getGlobalState() const {
    return globalState;
}
