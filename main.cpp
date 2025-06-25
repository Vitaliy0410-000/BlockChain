#include "blockfactory.h"
#include "Block.h"
#include "Logger.h"
#include <iostream>
#include <memory>
int main(){
    try{
        Logger::getInstance().log("=== New Test Session ===");
    Logger::getInstance().log("Starting blockchain test");
    std::unique_ptr<Block>block=BlockFactory().createBlock(GENESIS);
    block->mineBlock(3);
    }
    catch(const std::exception &ex)
    {
        std::cout<<ex.what()<<"Eror"<<std::endl;
    }
    return 0;
}
