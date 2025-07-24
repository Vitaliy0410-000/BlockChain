#include "Transaction.h"
#include <string>
#include "Logger.h"
#include <sstream>
#include <iomanip>
Transaction::Transaction(const std::string& sender, const std::string& recipient, double amount,const std::string& signature,const std::string& contractCode)
    : sender(sender), recipient(recipient), amount(amount), contractCode(contractCode), signature(signature)
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

std::string Transaction::getSender() const
{
    return sender;
}

std::string Transaction::getRecipient() const
{
    return recipient;
}

double Transaction::getAmount() const
{
    return amount;
}

std::string Transaction::getContractCode() const
{
    return contractCode;
}

std::string Transaction::toString() const
{
    std::stringstream ss;
    ss << sender << ":" << recipient << ":" << std::fixed << std::setprecision(2) << amount << ":" << signature;
    if (!contractCode.empty()) {
        ss << ":contract{" << contractCode << "}";
    }
    return ss.str();
}

std::string Transaction::getSignature() const
{
    return signature;
}
