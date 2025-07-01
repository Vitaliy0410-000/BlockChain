#ifndef REGULARBLOCK_H
#define REGULARBLOCK_H
#include "Block.h"
#include <string>
#include "Logger.h"

class RegularBlock:public Block
{
    friend class RegularBlockFactory;
public:
    RegularBlock(int index,long long timestamp,std::string data,std::string prevHash,int nonce);

    std::string calculateHash() const override;
    void mineBlock(int difficulty) override;
};

#endif // REGULARBLOCK_H
