#ifndef REGULARBLOCK_H
#define REGULARBLOCK_H
#include "Block.h"
#include <string>
#include "Logger.h"
#include <thread>
#include <atomic>
#include <vector>


class RegularBlock:public Block
{
    friend class RegularBlockFactory;
public:
    RegularBlock(int index,long long timestamp,std::string data,std::string prevHash,int nonce);

    std::string calculateHash() const override;
    void mineBlockParallel(int difficulty, int numThreads) override;
    void mineBlock(int difficulty)override;
    int getIndex() const override;
    std::string getHash()const override;
};

#endif // REGULARBLOCK_H
