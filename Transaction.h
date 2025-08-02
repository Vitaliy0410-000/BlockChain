#ifndef TRANSACTION_H
#define TRANSACTION_H
#include <string>
#include <cstdint>
#include <vector>
class Transaction
{
private:
    std::string sender;//отправитель
    std::string recipient;//получатель
    double amount;
    std::vector<uint8_t> contractCode;
    std::string signature;//подпись пока заглушка
    int64_t gasLimit;
public:
    std::string getSender()const;
    std::string getRecipient()const;
    double getAmount()const;
    std::string getSignature()const;
   Transaction(const std::string& sender, const std::string& recipient, double amount, const std::string& signature,
                std::vector<uint8_t> contractCode, int64_t gasLimit);
    std::string toString()const;
   const std::vector<uint8_t>& getContractCode() const;
    int64_t getGasLimit() const;
};
#endif
