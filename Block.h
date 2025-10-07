#ifndef BLOCK_H
#define BLOCK_H
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include "Transaction.h"

class Block
{
protected:
    int index;//number block
    long long timestamp;//time create block
    std::vector<Transaction>transactions;//list of tranzaction
    std::string prevHash;//hash prev block
    std::string hash;//hash this block
    int nonce;//for proof and work
public:
    Block (int index,long long timestamp,std::vector<Transaction>transactions,std::string prevHash,int nonce)
        :    index(index),timestamp(timestamp),transactions(transactions),prevHash(prevHash),hash(""),nonce(nonce)
    {
        if(index<0)
        {
            throw std::invalid_argument("Index can't be less than 0!");
        }
        if(timestamp<1)
        {
            throw std::invalid_argument("block creation must be positive!");
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

    virtual std::string getPrevHash()const=0;
    virtual std::string calculateHash() const=0;//func for create hash on index, timestamp, data, prevHash и nonce
    virtual void mineBlockParallel(int difficulty, int numThreads) = 0;
    virtual void mineBlock(int difficulty)=0;
    virtual int getIndex()const =0;
    virtual std::string getHash()const =0;
    virtual const std::vector<Transaction>& getTransactions() const = 0; // get list of transactions
    void addTransaction(Transaction transaction);
};





class Genesis : public Block {
    friend class GenesisFactory;
private:
    Genesis(int index, long long timestamp, std::vector<Transaction>transactions, std::string prevHash, int nonce);
    Genesis(const Genesis&) = delete;
    Genesis& operator=(const Genesis&) = delete;

public:
    static const int index;
    static const long long timestamp;
    static constexpr std::string_view prevHash = "0";
    int getIndex() const override;
    std::string getHash() const override;
    std::string getPrevHash() const override ;

    static Genesis& getInstance();
    std::string calculateHash() const override;
   void mineBlockParallel(int difficulty, int numThreads) override;
    void  mineBlock(int difficulty)override;
   const std::vector<Transaction>& getTransactions() const override;
};
//int difficulty  complexity mineng
#endif // BLOCK_H
