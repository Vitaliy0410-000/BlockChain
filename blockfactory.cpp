#include "blockfactory.h"
#include "Block.h"
#include "Logger.h"
#include <memory>
std::unique_ptr<Block> BlockFactory::createBlock(BlockType type)const
{
    if(type==BlockType::GENESIS)
    {
        Logger::getInstance().log("Created block type: GENESIS");
        return std::unique_ptr<Block>(new Genesis(Genesis::index, Genesis::timestamp, std::string(Genesis::data), std::string(Genesis::prevHash), 0));
    }
    if(type==BlockType::REGULAR)
    {
        return nullptr;
    }
return nullptr;
}
