#ifndef TRANSACTION_H
#define TRANSACTION_H
#include <string>
class Transaction
{
private:
    std::string sender;//отправитель
    std::string recipient;//получатель
    double amount;
    std::string signature;//подпись пока заглушка
public:
    std::string getSender();
    std::string getRecipient();
    double getAmount();
    std::string getSignature();
    Transaction(std::string sender,std::string recipient,double amount,std::string signature);
    std::string toString()const;
};

#endif // TRANSACTION_H
