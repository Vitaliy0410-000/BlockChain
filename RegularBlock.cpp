#include "Logger.h"
#include "openssl/sha.h"
#include "iomanip"
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <vector>
#include "RegularBlock.h"



RegularBlock::RegularBlock(int index,long long timestamp,std::vector<Transaction> transactions,std::string prevHash,int nonce)
    :Block(index,timestamp,transactions,prevHash,nonce)
{
    if(transactions.empty())
    {
        throw std::invalid_argument ("I can't create a block. No transactions!");
    }
    Logger::getInstance().log("Create Regular block with index="+std::to_string(index));
};

std::string RegularBlock::calculateHash()const
{
    std::string transactionsStr;
    for(auto it= transactions.begin();it!=transactions.end();it++)
    {
        std::string currentTransactionString = it->toString();
        if (!transactionsStr.empty())
        {
            transactionsStr += ";";
        }
        transactionsStr += currentTransactionString;
    }



    std::stringstream ss;
    ss << index << ":" << timestamp << ":" << transactionsStr << ":" << prevHash << ":" << nonce;
    std::string blockData=ss.str();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256,blockData.c_str(),blockData.length());
    SHA256_Final(hash,&sha256);
    std::stringstream ss_hex;
    for(int i=0;i<SHA256_DIGEST_LENGTH;i++)
    {
        ss_hex<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)hash[i];
    }


    return ss_hex.str();

}

void RegularBlock::mineBlockParallel(int difficulty, int numThreads) {
    if (difficulty <= 0) {
        throw std::invalid_argument("Difficulty must be positive!");
    }

    std::atomic<bool> found(false); // Волшебный звонок: "нашли ключ?"
    std::string target(difficulty, '0'); // Образец: "00" для 2 нулей
    int foundNonce = 0; // Коробка для номера ключа
    std::string foundHash; // Коробка для хеша

    Logger::getInstance().log("Started mining Regular block in " + std::to_string(numThreads) + " threads");

    auto mineRange = [this, &found, &target, &foundNonce, &foundHash, difficulty](int start, int step) {
        while (!found) {
            this->nonce = start; // Проверяем число
            std::string currentHash = calculateHash(); // Вычисляем хеш
            if (currentHash.substr(0, difficulty) == target) { // Если хеш подходит
                foundNonce = start; // Сохраняем номер
                foundHash = currentHash; // Сохраняем хеш
                found = true; // Звоним в звонок: "Стоп!"
                return;
            }
            start += step; // Берём следующее число (0,4,8,...)
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(mineRange, i, numThreads); // Нанимаем 4 рабочих
    }

    for (auto& t : threads) {
        t.join(); // Ждём, пока все закончат
    }

    this->nonce = foundNonce; // Сохраняем номер ключа
    this->hash = foundHash; // Сохраняем хеш

    Logger::getInstance().log("Mined Regular block: nonce=" + std::to_string(this->nonce) + ", hash=" + this->hash);
}
const std::vector<Transaction>& RegularBlock::getTransactions() const
{
    return transactions;
}


int RegularBlock::getIndex()const
{
    return index;
}
std::string RegularBlock::getHash()const
{
    return hash;
}
std::string RegularBlock::getPrevHash() const {
    return prevHash;
}

void RegularBlock::mineBlock(int difficulty) {
    if (difficulty <= 0) {
        throw std::invalid_argument("Difficulty must be positive!");
    }
    std::string target(difficulty, '0');
    Logger::getInstance().log("Started mining Regular block with difficulty= " + std::to_string(difficulty));
    while(true) {
        std::string currentHash = calculateHash();
        if(currentHash.substr(0, difficulty) == target) {
            hash = currentHash;
            break;
        }
        this->nonce++;
    }
    Logger::getInstance().log("Mined Regular block: nonce=" + std::to_string(this->nonce) + ", hash=" + hash);
}
