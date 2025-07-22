#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>
#include <memory>
#include <string>
#include "BlockFactory.h"
#include "Logger.h"
#include "Block.h"
#include <mutex>
#include <thread>

class Blockchain
{
public:
    static std::mutex instanceMutex;
    static std::mutex chainMutex;
    static Blockchain& getInstance(int difficulty = 2);

    void addBlock(std::string data);
    bool isChainValid();
    std::string getChainInfo()const;

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
