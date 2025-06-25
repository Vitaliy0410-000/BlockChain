#ifndef BLOCK_H
#define BLOCK_H
#include <string>
#include <iostream>

class Block
{
protected:
    int index;//number block
    long long timestamp;//time create block
    std::string data;//simpl string for data tranzaction
    std::string prevHash;//hash prev block
    std::string hash;//hash this block
    int nonce;//for proof and work
public:
    Block (int index,long long timestamp,std::string data,std::string prevHash,int nonce)
        :    index(index),timestamp(timestamp),data(data),prevHash(prevHash),hash(""),nonce(nonce)
    {
        if(index<0)
        {
            throw std::invalid_argument("Index can't be less than 0!");
        }
        if(timestamp<1)
        {
            throw std::invalid_argument("block creation must be positive!");
        }
        if(data.empty())
        {
            throw std::invalid_argument ("the transaction date must not be blank!");
        }
        if(prevHash.empty())
        {
             throw std::invalid_argument ("theprevHash must not be blank!");
        }
        if(nonce<0)
        {
             throw std::invalid_argument("nonce must be positive!");
        }

    };
    virtual std::string calculateHash() const=0;//func for create hash on index, timestamp, data, prevHash и nonce
    virtual void mineBlock(int difficulty )=0;
};





class Genesis : public Block {
    friend class BlockFactory;
private:
    Genesis(int index, long long timestamp, std::string data, std::string prevHash, int nonce);
    Genesis(const Genesis&) = delete;
    Genesis& operator=(const Genesis&) = delete;

public:
    static const int index;
    static const long long timestamp;
    static constexpr std::string_view data = "Genesis Block";
    static constexpr std::string_view prevHash = "0";

    static Genesis& getInstance();
    std::string calculateHash() const override;
    void mineBlock(int difficulty) override;
};
//int difficulty  complexity mineng
#endif // BLOCK_H
