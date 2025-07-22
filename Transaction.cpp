#include "Transaction.h"
#include <string>
#include "Logger.h"
#include <sstream>

Transaction::Transaction(std::string sender,std::string recipient,double amount,std::string signature)
    : sender(sender),recipient(recipient),amount(amount),signature(signature)
{

    if(sender.empty())
    {
        throw std::invalid_argument("the sender cannot be empty!");
    }
    if(recipient.empty())
    {
        throw std::invalid_argument("the recipient cannot be empty!");
    }
    if(amount<=0)
    {
         throw std::invalid_argument("Amount must be positive!");
    }
    if(signature.empty())
    {
        throw std::invalid_argument("the signature cannot be empty!");
    }
}

std::string Transaction::getSender()
{
    return sender;
}

std::string Transaction::getRecipient()
{
    return recipient;
}

double Transaction::getAmount()
{
    return amount;
}

std::string Transaction::getSignature()
{
    return signature;
}

std::string Transaction::toString() const
{
    std::stringstream stringTransaction;
    stringTransaction<<sender<< ":" <<recipient<< ":" <<amount<< ":" <<signature;
    Logger::getInstance().log("Create Transaction: ""Sender:"+ std::string(sender)+
                              " Recipient "+std::string(recipient)+ " Amount: "+ std::to_string(amount)+" Signature "+std::string(signature));
    return stringTransaction.str();

}
