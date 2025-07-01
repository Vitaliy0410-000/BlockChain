#include "BlockFactory.h"
#include "Block.h"
#include "Logger.h"
#include <iostream>
#include <memory>
#include "BlockChain.h"




int main(){
    try{
        Logger::getInstance().log("[2025-07-01 21:04:xx] === New Test Session ===");
    Logger::getInstance().log("Starting blockchain test");

    GeneralFactory &factory=getGeneralFactory();
    Blockchain &blockchain=factory.createBlockchain(2);
    blockchain.addBlock("First block data");
    blockchain.addBlock("Second block data");
    if (blockchain.isChainValid())
    {
        Logger::getInstance().log("[2025-07-01 20:45:xx] [INFO] Chain validation passed");
    }
    else
    {
        throw std::runtime_error("Chain is invalid"); }
    }

    catch(const std::exception &ex)
    {
        std::cout<<ex.what()<<"Eror"<<std::endl;
    }
    return 0;
}
