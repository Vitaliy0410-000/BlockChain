#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H


#include <vector>
#include <memory>
#include <string>
#include "BlockFactory.h"
#include "Logger.h"
#include "Block.h"
#include "SmartContract.h"
#include <mutex>
#include <thread>
#include <map>
#include <cstdint>
#include "SmartContractParser.h"
#include "Transaction.h"
class Node;

class Blockchain
{
public:
    void addTransaction(const Transaction& tx);
    static std::mutex instanceMutex;
    static std::mutex chainMutex;
    static Blockchain& getInstance(int difficulty, const std::string& host, unsigned short port);
    void addBlock(std::vector<Transaction> transactions);
    bool isChainValid();
    std::string getChainInfo() const;
    const std::map<std::string, int64_t>& getGlobalState() const;
    void testBlockchain();
    // Исправленная сигнатура: возвращает Transaction вместо void
    Transaction createTransactionWithGLTCH(std::string sender, std::string recipient, double amount,
                                           std::string signature, std::string gltchCode, int64_t gasLimit, Virtual_Machine::Context ctx);
private:
    std::vector<Transaction> transactionPool;
    Node* node;
    Blockchain(int difficulty,const std::string& host, unsigned short port);
    Blockchain(const Blockchain&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    static Blockchain* instance;
    std::vector<std::unique_ptr<Block>> chain;
    int difficulty;
    std::map<std::string, int64_t> globalState;
    ~Blockchain();
};

#endif
