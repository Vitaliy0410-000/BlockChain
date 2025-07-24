#ifndef TRANSACTION_H
#define TRANSACTION_H
#include <string>
class Transaction
{
private:
    std::string sender;//отправитель
    std::string recipient;//получатель
    double amount;
    std::string contractCode;
    std::string signature;//подпись пока заглушка
public:
    std::string getSender()const;
    std::string getRecipient()const;
    double getAmount()const;
    std::string getSignature()const;
   Transaction(const std::string& sender, const std::string& recipient, double amount, const std::string& signature, const std::string& contractCode = "");
    std::string toString()const;
    std::string getContractCode() const;
};
#endif
