#ifndef BLOCKFACTORY_H
#define BLOCKFACTORY_H
#include "Block.h"

#include <memory>


enum BlockType {GENESIS,REGULAR};

class ABlockFactory
{
public:
    virtual std::unique_ptr<Block> createBlock(BlockType type)const=0;


    virtual ~ABlockFactory()=default;
};

class BlockFactory:public ABlockFactory
{
public:

    std::unique_ptr<Block> createBlock(BlockType type)const override;
};

#endif // BLOCKFACTORY_H
