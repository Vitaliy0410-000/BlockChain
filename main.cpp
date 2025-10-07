#include "BlockChain.h"
#include "Transaction.h"
#include "Logger.h"
#include <iostream>
#include <vector>
#include <map>
#include "SmartContractParser.h"
#include "Virtual_Machine.h"
#include <thread>
#include <chrono>

int main() {
    try {
        Logger::getInstance().log(" === New Test Session (Node 1) ===");
        Logger::getInstance().log("Starting blockchain test");
        Logger::getInstance("log_node1.txt").log("Node 1 is running. Press Ctrl+C to stop.");
        Blockchain& bc = Blockchain::getInstance(4, "127.0.0.1", 12346);
        Logger::getInstance().log("Before starting node");
        bc.startNode(); // Запускаем сервер первой ноды
        Logger::getInstance().log("After starting node");

        // Подключаемся ко второй ноде
        Logger::getInstance().log("Connecting to peer 127.0.0.1:12347");
        bc.connectToPeer("127.0.0.1", 12347);

        // Тестовая транзакция 1: PUSH 100, HALT
        std::vector<uint8_t> bytecode1 = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};
        Transaction tx1("Alice", "Bob", 10.5, "fake", bytecode1, 1000, "");
        std::vector<Transaction> txs = {tx1};

        // Тестовая транзакция 2: PUSH 100, SSTORE
        std::vector<uint8_t> bytecode2 = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x03};
        Logger::getInstance().log("Creating transaction with bytecode: PUSH 100, SSTORE");
        Transaction tx2("Alice", "Bob", 10.5, "sig5", bytecode2, 1000, "");
        txs.push_back(tx2);

        Logger::getInstance().log("Adding block with " + std::to_string(txs.size()) + " transactions");
        bc.addBlock(txs);
        Logger::getInstance().log("Block added, getting chain info");
        std::cout << bc.getChainInfo() << std::endl;
        if (bc.isChainValid()) {
            std::cout << "Chain is valid" << std::endl;
        } else {
            std::cout << "Chain is invalid" << std::endl;
        }

        auto state = bc.getGlobalState();
        std::cout << "Alice balance: " << state.at("Alice") / 100.0 << std::endl;
        std::cout << "Bob balance: " << state.at("Bob") / 100.0 << std::endl;
        if (state.count("Alice")) {
            std::cout << "Contract balance (Alice): " << state.at("Alice") << std::endl;
        }
std::this_thread::sleep_for(std::chrono::seconds(5));
        // Вызов тестов из Blockchain
        Logger::getInstance().log("Running testBlockchain");
        bc.testBlockchain();

        // Бесконечный цикл для поддержания работы ноды
        Logger::getInstance().log("Node 1 is running. Press Ctrl+C to stop.");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& ex) {
        Logger::getInstance().log("Error: " + std::string(ex.what()));
        std::cout << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
