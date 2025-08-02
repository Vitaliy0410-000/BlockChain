#include "BlockChain.h"
#include "Transaction.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <map>

int main() {
    try {
        Logger::getInstance().log(" === New Test Session ===");
        Logger::getInstance().log("Starting blockchain test");

        Blockchain& bc = Blockchain::getInstance(4);
        // Тестовая транзакция с байт-кодом: PUSH 100, HALT
        std::vector<uint8_t> bytecode = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};
        Transaction tx("Alice", "Bob", 10.5, "fake", bytecode, 1000);
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

        // Вызов тестов из Blockchain
        Logger::getInstance().log("Running testBlockchain");
        bc.testBlockchain();
    } catch (const std::exception& ex) {
        Logger::getInstance().log("Error: " + std::string(ex.what()));
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}

