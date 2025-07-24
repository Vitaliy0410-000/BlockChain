#ifndef BLOCKFACTORY_H
#define BLOCKFACTORY_H
#include "RegularBlock.h"
#include "Block.h"
#include <memory>


class Blockchain;

class GenesisFactory
{
public:
    std::unique_ptr<Block> createGenesis() const;
};

class RegularBlockFactory
{
public:
    std::unique_ptr<Block> createRegularBlock(int index, long long timestamp, std::vector<Transaction> transactions, std::string prevHash, int nonce) const;
};

class BlockChainFactory
{
public:
    Blockchain& createBlockChain(int difficulty) const;
};

class GeneralFactory
{
private:
    GenesisFactory genesisFactory;
    RegularBlockFactory regularBlockFactory;
    BlockChainFactory blockchainFactory;
    static GeneralFactory* instance;
    GeneralFactory();
    ~GeneralFactory();
    GeneralFactory(const GeneralFactory&) = delete;
    GeneralFactory& operator=(const GeneralFactory&) = delete;
public:
    static GeneralFactory& getInstance();
    std::unique_ptr<Block> createGenesis() const;
    std::unique_ptr<Block> createRegularBlock(int index, long long timestamp, std::vector<Transaction> transactions, std::string prevHash, int nonce) const;
    Blockchain& createBlockchain(int difficulty) const;
};
GeneralFactory& getGeneralFactory();

#endif // BLOCKFACTORY_H

