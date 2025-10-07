#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include "BlockChain.h"

class Node {
public:
    Node(std::string host, unsigned short port, Blockchain& blockchain);
    ~Node();
    void start();
    void connectToPeer(std::string host, unsigned short port);
    void broadcastTransaction(const Transaction& tx);
    void broadcastBlock(const std::unique_ptr<Block>& block);

private:
    std::string host;
    unsigned short port;
    Blockchain& blockchain;
    std::vector<std::string> peers;
    std::mutex peers_mutex;
    boost::asio::io_context io_context;
    std::thread server_thread;
    boost::asio::ip::tcp::acceptor acceptor; // Добавляем acceptor как поле

    void handleConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handleMessage(const std::string& message);
};

#endif // NODE_H
