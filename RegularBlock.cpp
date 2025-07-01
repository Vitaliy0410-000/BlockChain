#include "RegularBlock.h"
#include "Logger.h"
#include "openssl/sha.h"
#include "iomanip"
#include <string>
#include <sstream>





RegularBlock::RegularBlock(int index,long long timestamp,std::string data,std::string prevHash,int nonce)
:Block(index,timestamp,data,prevHash,nonce)
{
    Logger::getInstance().log("Create Regular block with index="+std::to_string(index));
};

    std::string RegularBlock::calculateHash()const
{
    std::stringstream ss;
    ss<<index<< ":" <<timestamp<< ":" <<data<< ":" <<prevHash<< ":" <<nonce;
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
    std::string hashStr = ss_hex.str();
    return ss_hex.str();

}

void RegularBlock::mineBlock(int difficulty)
{
    if (difficulty <= 0)
    {
        throw std::invalid_argument("Difficulty must be positive!");
    }
    std::string target(difficulty,'0');
    Logger::getInstance().log("Started mining Regular block with difficulty= " + std::to_string(difficulty));
    while(true)
    {
       std::string currentHash=calculateHash();
        if(currentHash.substr(0,difficulty)==target)
        {
            hash= currentHash;
            break;
        }
        this->nonce++;
    }
    Logger::getInstance().log("Mined Regular block: nonce=" + std::to_string(this->nonce) + " ,hash= "+hash);
}
