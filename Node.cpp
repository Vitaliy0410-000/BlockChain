#include "Node.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using json = nlohmann::json;
using boost::asio::ip::tcp;

// Конструктор
Node::Node(std::string host, unsigned short port, Blockchain& blockchain)
    : host(host), port(port), blockchain(blockchain) {
    Logger::getInstance().log("Node initialized at " + host + ":" + std::to_string(port));
}

// Запуск сервера
void Node::start() {
    Logger::getInstance().log("Node starting at " + host + ":" + std::to_string(port));

    // Создаём acceptor для прослушивания входящих соединений
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

    // Явно определяем тип для doAccept
    std::function<void(std::shared_ptr<tcp::socket>)> doAccept;
    doAccept = [this, &acceptor, &doAccept](std::shared_ptr<tcp::socket> socket) {
        acceptor.async_accept(*socket,
                              [this, socket, &acceptor, &doAccept](const boost::system::error_code& error) {
                                  if (!error) {
                                      Logger::getInstance().log("Accepted connection from " + socket->remote_endpoint().address().to_string());
                                      handleConnection(socket);
                                  } else {
                                      Logger::getInstance().log("Accept error: " + error.message());
                                  }
                                  // Повторно вызываем doAccept для следующего соединения
                                  doAccept(std::make_shared<tcp::socket>(io_context));
                              });
    };

    // Запускаем первый цикл принятия соединений
    doAccept(std::make_shared<tcp::socket>(io_context));

    // Запускаем io_context в отдельном потоке
    server_thread = std::thread([this]() { io_context.run(); });
    Logger::getInstance().log("Node started at " + host + ":" + std::to_string(port));
}

// Обработка соединения
void Node::handleConnection(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();

    // Явно определяем тип для doRead
    std::function<void(const boost::system::error_code&, std::size_t)> doRead;
    doRead = [this, socket, buffer, &doRead](const boost::system::error_code& error, std::size_t /* bytes_transferred */) {
        if (!error) {
            std::istream is(buffer.get());
            std::string message;
            std::getline(is, message);
            Logger::getInstance().log("Received message: " + message);

            handleMessage(message);

            // Повторно вызываем async_read_until для следующего сообщения
            boost::asio::async_read_until(*socket, *buffer, '\n', doRead);
        } else {
            Logger::getInstance().log("Read error: " + error.message());
        }
    };

    // Запускаем асинхронное чтение
    boost::asio::async_read_until(*socket, *buffer, '\n', doRead);
}

// Обработка сообщения
void Node::handleMessage(const std::string& message) {
    try {
        json parsed = json::parse(message);
        std::string type = parsed["type"];

        if (type == "transaction") {
            auto data = parsed["data"];
            std::string sender = data["sender"];
            std::string recipient = data["recipient"];
            double amount = data["amount"];
            std::string signature = data["signature"];
            json contractCode = data["contractCode"];
            int gasLimit = data["gasLimit"];

            Transaction tx(sender, recipient, amount, signature, contractCode, gasLimit);
            blockchain.addBlock(std::vector<Transaction>{tx});

            Logger::getInstance().log("Processed transaction: " + sender + " -> " + recipient + ", amount: " + std::to_string(amount));
        } else if (type == "block") {
            // TODO: Обработка блока (добавить в blockchain после проверки)
            Logger::getInstance().log("Received block (not implemented)");
        } else {
            Logger::getInstance().log("Unknown message type: " + type);
        }
    } catch (const json::exception& e) {
        Logger::getInstance().log("JSON parse error: " + std::string(e.what()));
    }
}

// Подключение к пиру
void Node::connectToPeer(std::string host, unsigned short port) {
    std::string peer = host + ":" + std::to_string(port);
    {
        std::lock_guard<std::mutex> lock(peers_mutex);
        peers.push_back(peer);
    }
    Logger::getInstance().log("Connected to peer: " + peer);
}

// Отправка транзакции всем пирам
void Node::broadcastTransaction(const Transaction& tx) {
    json message;
    message["type"] = "transaction";
    message["data"] = {
        {"sender", tx.getSender()},
        {"recipient", tx.getRecipient()},
        {"amount", tx.getAmount()},
        {"signature", tx.getSignature()},
        {"contractCode", tx.getContractCode()},
        {"gasLimit", tx.getGasLimit()}
    };
    std::string message_str = message.dump() + "\n";

    std::vector<std::string> current_peers;
    {
        std::lock_guard<std::mutex> lock(peers_mutex);
        current_peers = peers;
    }

    for (const auto& peer : current_peers) {
        std::string peer_host;
        unsigned short peer_port;
        std::stringstream ss(peer);
        std::getline(ss, peer_host, ':');
        ss >> peer_port;

        auto socket = std::make_shared<tcp::socket>(io_context);
        boost::system::error_code ec;
        tcp::endpoint endpoint(boost::asio::ip::make_address(peer_host, ec), peer_port);

        if (ec) {
            Logger::getInstance().log("Invalid address for peer " + peer + ": " + ec.message());
            continue;
        }

        socket->async_connect(endpoint, [this, socket, message_str, peer](const boost::system::error_code& error) {
            if (!error) {
                boost::asio::async_write(*socket, boost::asio::buffer(message_str),
                                         [this, peer](const boost::system::error_code& error, std::size_t /* bytes_transferred */) {
                                             if (!error) {
                                                 Logger::getInstance().log("Sent transaction to " + peer);
                                             } else {
                                                 Logger::getInstance().log("Write error to " + peer + ": " + error.message());
                                             }
                                         });
            } else {
                Logger::getInstance().log("Connect error to " + peer + ": " + error.message());
            }
        });
    }
}

// Отправка блока всем пирам
void Node::broadcastBlock(const std::unique_ptr<Block>& block) {
    json message;
    message["type"] = "block";
    json txs;
    for (const auto& tx : block->getTransactions()) {
        txs.push_back({
            {"sender", tx.getSender()},
            {"recipient", tx.getRecipient()},
            {"amount", tx.getAmount()},
            {"signature", tx.getSignature()},
            {"contractCode", tx.getContractCode()},
            {"gasLimit", tx.getGasLimit()}
        });
    }
    message["data"] = {
        {"index", block->getIndex()},
        {"hash", block->getHash()},
        {"prevHash", block->getPrevHash()},
        {"transactions", txs}
    };
    std::string message_str = message.dump() + "\n";

    std::vector<std::string> current_peers;
    {
        std::lock_guard<std::mutex> lock(peers_mutex);
        current_peers = peers;
    }

    for (const auto& peer : current_peers) {
        std::string peer_host;
        unsigned short peer_port;
        std::stringstream ss(peer);
        std::getline(ss, peer_host, ':');
        ss >> peer_port;

        auto socket = std::make_shared<tcp::socket>(io_context);
        boost::system::error_code ec;
        tcp::endpoint endpoint(boost::asio::ip::make_address(peer_host, ec), peer_port);

        if (ec) {
            Logger::getInstance().log("Invalid address for peer " + peer + ": " + ec.message());
            continue;
        }

        socket->async_connect(endpoint, [this, socket, message_str, peer](const boost::system::error_code& error) {
            if (!error) {
                boost::asio::async_write(*socket, boost::asio::buffer(message_str),
                                         [this, peer](const boost::system::error_code& error, std::size_t /* bytes_transferred */) {
                                             if (!error) {
                                                 Logger::getInstance().log("Sent block to " + peer);
                                             } else {
                                                 Logger::getInstance().log("Write error to " + peer + ": " + error.message());
                                             }
                                         });
            } else {
                Logger::getInstance().log("Connect error to " + peer + ": " + error.message());
            }
        });
    }
}
