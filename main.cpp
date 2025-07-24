
#include "BlockChain.h"
#include "Transaction.h"
#include "SmartContract.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <map>

    int main() {
    try {
        Logger::getInstance().log(" === New Test Session ===");
        Logger::getInstance().log("Starting blockchain test");

        Blockchain& bc = Blockchain::getInstance(4);
        Transaction tx("Alice", "Bob", 10.5, "fake", "if balance[Alice] >= 10 then transfer 10 to Bob");
        std::vector<Transaction> txs = {tx};
        bc.addBlock(txs);
        std::cout << bc.getChainInfo() << std::endl;
        if (bc.isChainValid()) {
            std::cout << "Chain is valid" << std::endl;
        } else {
            std::cout << "Chain is invalid" << std::endl;
        }

        auto state = bc.getGlobalState();
        std::cout << "Alice balance: " << state.at("Alice") << std::endl;
        std::cout << "Bob balance: " << state.at("Bob") << std::endl;
    } catch (const std::exception& ex) {
        Logger::getInstance().log("Error: " + std::string(ex.what()));
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}

