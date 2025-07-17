#include "Block.h"
#include "openssl/sha.h"
#include "iomanip"
#include <string>
#include <sstream>
#include <string_view>
#include "Logger.h"
#include <thread>
#include <atomic>
#include <vector>



using std::string;
using std::cout;
using std::endl;


    Genesis::Genesis(int index,long long timestamp,string data,string prevHash,int nonce)
        :Block(index,timestamp,data,prevHash,nonce)
    {
        Logger::getInstance().log("Create Genesis block with index=0");
    };


   Genesis& Genesis::getInstance()
    {
        static Genesis instance(index,timestamp,string(data),string(prevHash),0);
        return instance;
    }
    const int Genesis:: index=0;
     const long long Genesis:: timestamp = 21062025;
    constexpr std::string_view Genesis:: data;
     constexpr std::string_view Genesis:: prevHash;

    string Genesis::calculateHash() const
    {
        std::stringstream ss;
        ss<<index<< ":" <<timestamp<< ":" <<data<< ":" <<prevHash<< ":" <<nonce;
        string blockData=ss.str();
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
         std::string hashStr = ss_hex.str();
        return ss_hex.str();

     }
    void Genesis::mineBlock(int difficulty)
    {
        std::string target(difficulty, '0');
        if (difficulty < 0)
        {
        throw std::invalid_argument("Difficulty must be positive!");
        }

        Logger::getInstance().log("Started mining Genesis block with difficulty= " + std::to_string(difficulty));
        while(true)
        {
        string currentHash=calculateHash();
            if(currentHash.substr(0,difficulty)==target)
        {
            hash= currentHash;
                break;
            }
            this->nonce++;
        }
        Logger::getInstance().log("Mined Genesis block: nonce=" + std::to_string(this->nonce) + " ,hash= "+hash);

    }

    void Genesis::mineBlockParallel(int difficulty, int numThreads) {
        if (difficulty <= 0) {
            throw std::invalid_argument("Difficulty must be positive!");
        }

        std::atomic<bool> found(false); // Волшебный звонок: "нашли ключ?"
        std::string target(difficulty, '0'); // Образец: "00" для 2 нулей
        int foundNonce = 0; // Коробка для номера ключа
        std::string foundHash; // Коробка для хеша

        Logger::getInstance().log("Started mining Genesis block in " + std::to_string(numThreads) + " threads");

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

        Logger::getInstance().log("Mined Genesis block: nonce=" + std::to_string(this->nonce) + ", hash=" + this->hash);
    }
    int Genesis::getIndex() const {
        return index;
    }

    std::string Genesis::getHash() const {
        return hash;
    }

    std::string Genesis::getPrevHash() const {
        return std::string(prevHash);
    }
