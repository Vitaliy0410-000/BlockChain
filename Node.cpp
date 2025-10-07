#include "Node.h"
#include "BlockChain.h"
#include "BlockFactory.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

using json = nlohmann::json;
using boost::asio::ip::tcp;

Node::Node(std::string host, unsigned short port, Blockchain& blockchain)
    : host(host), port(port), blockchain(blockchain), acceptor(io_context) {
    try {
        std::cerr << "Host initialized: " << host << std::endl;
        Logger::getInstance().log("Host initialized: " + host);
        std::cerr << "Port initialized: " << port << std::endl;
        Logger::getInstance().log("Port initialized: " + std::to_string(port));
        std::cerr << "Blockchain reference initialized" << std::endl;
        Logger::getInstance().log("Blockchain reference initialized");
        Logger::getInstance().log("Node initialized at " + host + ":" + std::to_string(port));
    } catch (const std::exception& e) {
        std::cerr << "Error in Node constructor: " << e.what() << std::endl;
        Logger::getInstance().log("Error in Node constructor: " + std::string(e.what()));
        throw;
    }
}

Node::~Node() {
    try {
        if (acceptor.is_open()) acceptor.close();
        io_context.stop();
        if (server_thread.joinable()) server_thread.join();
        Logger::getInstance().log("Node resources cleaned up");
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error in Node destructor: " + std::string(e.what()));
    }
}

void Node::start() {
    Logger::getInstance().log("Node starting at " + host + ":" + std::to_string(port));
    try {
        acceptor.open(tcp::v4());
        acceptor.set_option(tcp::acceptor::reuse_address(true));
        acceptor.bind(tcp::endpoint(tcp::v4(), port));
        acceptor.listen();
        std::function<void(std::shared_ptr<tcp::socket>)> doAccept;
        doAccept = [this, &doAccept](std::shared_ptr<tcp::socket> socket) {
            acceptor.async_accept(*socket,
                                  [this, socket, &doAccept](const boost::system::error_code& error) {
                                      if (!error) {
                                          Logger::getInstance().log("Accepted connection from " + socket->remote_endpoint().address().to_string());
                                          handleConnection(socket);
                                          doAccept(std::make_shared<tcp::socket>(io_context));
                                      } else if (error == boost::asio::error::operation_aborted) {
                                          Logger::getInstance().log("Accept operation aborted");
                                      } else {
                                          Logger::getInstance().log("Accept error: " + error.message());
                                          doAccept(std::make_shared<tcp::socket>(io_context));
                                      }
                                  });
        };
        doAccept(std::make_shared<tcp::socket>(io_context));
        server_thread = std::thread([this]() {
            try {
                io_context.run();
            } catch (const std::exception& e) {
                Logger::getInstance().log("io_context.run() error: " + std::string(e.what()));
            }
        });
        Logger::getInstance().log("Node started at " + host + ":" + std::to_string(port));
    } catch (const std::exception& e) {
        Logger::getInstance().log("Error starting node: " + std::string(e.what()));
        throw;
    }
}

void Node::handleConnection(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    std::function<void(const boost::system::error_code&, std::size_t)> doRead;
    doRead = [this, socket, buffer, &doRead](const boost::system::error_code& error, std::size_t bytes_transferred) {
        if (!error) {
            if (bytes_transferred == 0) {
                Logger::getInstance().log("No data received, skipping read");
                return;
            }
            std::istream is(buffer.get());
            std::string message;
            std::getline(is, message);
            if (message.empty()) {
                Logger::getInstance().log("Received empty message, continuing to read");
                boost::asio::async_read_until(*socket, *buffer, '\n', doRead);
                return;
            }
            Logger::getInstance().log("Received message: " + message);
            try {
                handleMessage(message);
            } catch (const std::exception& e) {
                Logger::getInstance().log("Error processing message: " + std::string(e.what()));
            }
            boost::asio::async_read_until(*socket, *buffer, '\n', doRead);
        } else if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
            Logger::getInstance().log("Connection closed by peer: " + error.message());
        } else {
            Logger::getInstance().log("Read error: " + error.message());
        }
    };
    boost::asio::async_read_until(*socket, *buffer, '\n', doRead);
}

void Node::handleMessage(const std::string& message) {
    try {
        if (!json::accept(message)) {
            Logger::getInstance().log("Invalid JSON received: " + message);
            return;
        }
        json parsed = json::parse(message);
        if (!parsed.contains("type")) {
            Logger::getInstance().log("Message missing 'type' field: " + message);
            return;
        }
        std::string type = parsed["type"];
        if (type == "transaction") {
            if (!parsed.contains("data")) {
                Logger::getInstance().log("Transaction message missing 'data' field: " + message);
                return;
            }
            auto data = parsed["data"];
            std::string sender = data.value("sender", "");
            std::string recipient = data.value("recipient", "");
            double amount = data.value("amount", 0.0);
            std::string signature = data.value("signature", "");
            std::vector<uint8_t> contractCode = data.contains("contractCode") ?
                                                    data["contractCode"].get<std::vector<uint8_t>>() : std::vector<uint8_t>();
            std::string gltchCode = data.contains("gltchCode") ? data["gltchCode"].get<std::string>() : "";
            int64_t gasLimit = data.value("gasLimit", 0);
            if (sender.empty() || recipient.empty()) {
                Logger::getInstance().log("Invalid transaction: sender or recipient empty");
                return;
            }
            Transaction tx(sender, recipient, amount, signature, contractCode, gasLimit, gltchCode);
            blockchain.addTransaction(tx);
            Logger::getInstance().log("Processed transaction: " + sender + " -> " + recipient + ", amount: " + std::to_string(amount));
        } else if (type == "block") {
            if (!parsed.contains("data")) {
                Logger::getInstance().log("Block message missing 'data' field: " + message);
                return;
            }
            auto data = parsed["data"];
            int index = data.value("index", -1);
            std::string prevHash = data.value("prevHash", "");
            std::vector<Transaction> transactions;
            if (data.contains("transactions")) {
                for (const auto& tx_data : data["transactions"]) {
                    std::string tx_sender = tx_data.value("sender", "");
                    std::string tx_recipient = tx_data.value("recipient", "");
                    double tx_amount = tx_data.value("amount", 0.0);
                    std::string tx_signature = tx_data.value("signature", "");
                    std::vector<uint8_t> tx_contractCode = tx_data.contains("contractCode") ?
                                                               tx_data["contractCode"].get<std::vector<uint8_t>>() : std::vector<uint8_t>();
                    std::string tx_gltchCode = tx_data.contains("gltchCode") ? tx_data["gltchCode"].get<std::string>() : "";
                    int64_t tx_gasLimit = tx_data.value("gasLimit", 0);
                    if (tx_sender.empty() || tx_recipient.empty()) {
                        Logger::getInstance().log("Invalid transaction in block: sender or recipient empty");
                        continue;
                    }
                    transactions.emplace_back(tx_sender, tx_recipient, tx_amount, tx_signature,
                                              tx_contractCode, tx_gasLimit, tx_gltchCode);
                }
            }
            if (index < 0 || prevHash.empty()) {
                Logger::getInstance().log("Invalid block: index=" + std::to_string(index) + ", prevHash=" + prevHash);
                return;
            }
            auto block = RegularBlockFactory().createRegularBlock(index, std::time(nullptr), transactions, prevHash, 0);
            blockchain.addBlock(transactions);
            Logger::getInstance().log("Processed block with index: " + std::to_string(index));
        } else {
            Logger::getInstance().log("Unknown message type: " + type);
        }
    } catch (const json::exception& e) {
        Logger::getInstance().log("JSON parse error: " + std::string(e.what()) + ", message: " + message);
    } catch (const std::exception& e) {
        Logger::getInstance().log("Unexpected error in handleMessage: " + std::string(e.what()));
    }
}

void Node::connectToPeer(std::string host, unsigned short port) {
    for (int i = 0; i < 5; ++i) {
        try {
            Logger::getInstance().log("Connecting to peer from Blockchain: " + host + ":" + std::to_string(port));
            auto socket = std::make_shared<tcp::socket>(io_context);
            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            boost::asio::connect(*socket, endpoints);
            Logger::getInstance().log("Connected to peer: " + host + ":" + std::to_string(port));
            std::string peer = host + ":" + std::to_string(port);
            std::lock_guard<std::mutex> lock(peers_mutex);
            peers.push_back(peer);
            break;
        } catch (const std::exception& e) {
            Logger::getInstance().log("Connect error to " + host + ":" + std::to_string(port) + ": " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}

void Node::broadcastTransaction(const Transaction& tx) {
    json message;
    message["type"] = "transaction";
    message["data"] = {
        {"sender", tx.getSender()},
        {"recipient", tx.getRecipient()},
        {"amount", tx.getAmount()},
        {"signature", tx.getSignature()},
        {"contractCode", tx.getContractCode()},
        {"gltchCode", tx.getGltchCode()},
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
            {"gltchCode", tx.getGltchCode()},
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
