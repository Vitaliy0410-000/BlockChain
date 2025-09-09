#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <vector>
#include <cstdint>

class Transaction
{
public:
    // Конструктор с gltchCode
    Transaction(std::string sender, std::string recipient, double amount,
                std::string signature, std::vector<uint8_t> contractCode,
                int64_t gasLimit, std::string gltchCode);
    // Новый конструктор без gltchCode (по умолчанию пустой)
    Transaction(std::string sender, std::string recipient, double amount,
                std::string signature, std::vector<uint8_t> contractCode,
                int64_t gasLimit);
    std::string getSender() const;
    std::string getRecipient() const;
    double getAmount() const;
    std::string getSignature() const;
    std::vector<uint8_t> getContractCode() const;
    int64_t getGasLimit() const;
    std::string getGltchCode() const;
    std::string toString() const;

private:
    std::string sender;
    std::string recipient;
    double amount;
    std::string signature;
    std::vector<uint8_t> contractCode;
    int64_t gasLimit;
    std::string gltchCode;
};

#endif
