#include "BlockChain.h"
#include <vector>
#include <memory>
#include "BlockFactory.h"
#include "Logger.h"
#include <ctime>
#include <string>

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
    genesis->mineBlock(difficulty);
    chain.push_back(std::move(genesis));
    Logger::getInstance().log("[INFO] Initialized Blockchain with Genesis block");
}
Blockchain* Blockchain::instance = nullptr;
void Blockchain::addBlock(std::string data)
{
    if (data.empty())
    {
        throw std::invalid_argument("Data cannot be empty");
    }
    Logger::getInstance().log("[2025-07-01 20:57:xx] [INFO] Chain size: " + std::to_string(chain.size()));
    auto block = RegularBlockFactory().createRegularBlock(chain.size(), std::time(nullptr), data, chain.back()->getHash(), 0);
    if(chain.empty())
    {
        throw std::runtime_error("Chain is empty, cannot add block");
    }
    if (!block)
    {
        throw std::runtime_error("Failed to create RegularBlock");
    }
    block->mineBlock(difficulty);
    chain.push_back(std::move(block));
    Logger::getInstance().log("[INFO] Added block with index=" + std::to_string(chain.size() - 1));
}

bool Blockchain::isChainValid()
{
    Logger::getInstance().log("[2025-07-01 20:57:xx] [INFO] Validating chain with size: " + std::to_string(chain.size()));
    for (size_t i = 1; i < chain.size(); ++i)
    {
        if (chain[i]->getPrevHash() != chain[i - 1]->getHash())
        {
            Logger::getInstance().log("[ERROR] Chain invalid at index=" + std::to_string(i));
            return false;
        }
    }
    if (chain.empty())
    {
        return false;
    }
    Logger::getInstance().log("[INFO] Chain is valid");
    return true;
}

std::string Blockchain::getChainInfo()
{
    return "Chain size=" + std::to_string(chain.size()) + ", last hash=" + chain.back()->getHash();
}

Blockchain& Blockchain::getInstance(int difficulty)
{
    if(instance == nullptr)
    {
       instance = new Blockchain(difficulty);
    }
    Logger::getInstance().log("[2025-07-01 21:14:xx] [DEBUG] Blockchain getInstance called");
    return *instance;
}
