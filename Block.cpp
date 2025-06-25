#include "Block.h"
#include "openssl/sha.h"
#include "iomanip"
#include <string>
#include <sstream>
#include <string_view>
#include "Logger.h"
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
        if (difficulty < 0)
        {
        throw std::invalid_argument("Difficulty must be positive!");
        }
        string target(difficulty,'0');
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

