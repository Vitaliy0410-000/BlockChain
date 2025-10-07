#include "BlockChain.h"
#include "Transaction.h"
#include "Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        Logger::getInstance().log(" === New Test Session (Node 2) ===");
        Blockchain& bc = Blockchain::getInstance(4, "127.0.0.1", 12347);
        Logger::getInstance().log("Before starting node");
        bc.startNode(); // Запускаем сервер второй ноды
        Logger::getInstance().log("After starting node");
    Logger::getInstance("log_node2.txt").log("Starting testChain2");
        // Подключаемся к первой ноде
        Logger::getInstance().log("Connecting to peer 127.0.0.1:12346");
        bc.connectToPeer("127.0.0.1", 12346);

        // Тестовая транзакция для отправки
        std::vector<uint8_t> bytecode = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00};
        Transaction tx("Charlie", "Dave", 5.0, "sig6", bytecode, 1000, "");
        Logger::getInstance().log("Sending transaction: Charlie -> Dave, amount: 5.0");
        bc.addTransaction(tx); // Это вызовет broadcastTransaction
std::this_thread::sleep_for(std::chrono::seconds(2));
        // Бесконечный цикл для поддержания работы ноды
        Logger::getInstance().log("Node 2 is running. Press Ctrl+C to stop.");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& ex) {
        Logger::getInstance().log("Error: " + std::string(ex.what()));
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
