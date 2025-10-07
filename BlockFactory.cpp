#include "Block.h"
#include "Logger.h"
#include "RegularBlock.h"
#include "BlockChain.h"
#include <memory>
#include <ctime>
#include "BlockFactory.h"
#include "Node.h"
std::unique_ptr<Block> GenesisFactory::createGenesis() const
{
    Logger::getInstance().log("Created block type: GENESIS BLOCK");
    return std::unique_ptr<Block>(new Genesis(0, std::time(nullptr), std::vector<Transaction>{}, "0", 0));
}

// Реализация RegularBlockFactory
std::unique_ptr<Block> RegularBlockFactory::createRegularBlock(int index, long long timestamp, std::vector<Transaction> transactions, std::string prevHash, int nonce) const
{
    Logger::getInstance().log("Created block type: REGULAR BLOCK");
    return std::unique_ptr<Block>(new RegularBlock(index, timestamp, transactions, prevHash, nonce));
}

// Реализация BlockChainFactory
Blockchain& BlockChainFactory::createBlockChain(int difficulty,const std::string& host, unsigned short port) const
{
    Logger::getInstance().log("Created block type: BLOCKCHAIN");
    return Blockchain::getInstance(difficulty, host, port);
}


GeneralFactory* GeneralFactory::instance = nullptr;

GeneralFactory::GeneralFactory() {}
GeneralFactory::~GeneralFactory() {}

GeneralFactory& GeneralFactory::getInstance()
{
    if (instance == nullptr)
    {
        instance = new GeneralFactory();
    }
    return *instance;
}

std::unique_ptr<Block> GeneralFactory::createGenesis() const
{
    return genesisFactory.createGenesis();
}

std::unique_ptr<Block> GeneralFactory::createRegularBlock(int index, long long timestamp, std::vector<Transaction> transactions, std::string prevHash, int nonce) const
{
    return regularBlockFactory.createRegularBlock(index, timestamp, transactions, prevHash, nonce);
}

Blockchain& GeneralFactory::createBlockchain(int difficulty,const std::string& host, unsigned short port) const
{
    return blockchainFactory.createBlockChain(difficulty,host,port);
}


GeneralFactory& getGeneralFactory() {
    return GeneralFactory::getInstance();
}
