#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>
#include <memory>
#include <string>
#include "Block.h"
#include "BlockFactory.h"
#include "Logger.h"
#include "Block.h"

class Blockchain
{
public:
    static Blockchain& getInstance(int difficulty = 2);

    void addBlock(std::string data);
    bool isChainValid();
    std::string getChainInfo();

private:
    Blockchain(int difficulty);
    Blockchain(const Blockchain&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    static Blockchain* instance;
    std::vector<std::unique_ptr<Block>> chain;
    int difficulty;
    ~Blockchain();
};

#endif // BLOCKCHAIN_H
