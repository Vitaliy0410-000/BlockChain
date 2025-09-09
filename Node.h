#ifndef NODE_H
#define NODE_H
#include <string>
#include "BlockChain.h"
#include <vector>
#include <boost/asio.hpp>
#include <mutex>
#include <thread>


class Node
{
private:
    std::string host;
    unsigned short port;
    Blockchain &blockchain;
    std::vector<std::string> peers;//вектор строк для адресов пиров
    boost::asio::io_context io_context;
    std::mutex peers_mutex;
    std::thread server_thread;
    void handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handleMessage(const std::string& message);
public:
    Node(std::string host,unsigned short port,Blockchain &blockchain);
    void start();//старт работы сервера
    void connectToPeer(std::string host,unsigned short port) ;// подключается к другой ноде.
    void broadcastTransaction(const Transaction& tx) ;// отправляет транзакцию всем пирам.
    void broadcastBlock(const std::unique_ptr<Block>& block); // отправляет блок всем пирам.,
    ~Node() {
        if (server_thread.joinable()) {
            io_context.stop();
            server_thread.join();
            Logger::getInstance().log("Node stopped");
        }
    }

};

#endif
