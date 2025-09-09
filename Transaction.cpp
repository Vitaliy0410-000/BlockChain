#include "Transaction.h"
#include <stdexcept>
#include <sstream>

Transaction::Transaction(std::string sender, std::string recipient, double amount,
                         std::string signature, std::vector<uint8_t> contractCode,
                         int64_t gasLimit, std::string gltchCode)
    : sender(sender), recipient(recipient), amount(amount), signature(signature),
    contractCode(contractCode), gasLimit(gasLimit), gltchCode(gltchCode)
{
    if (sender.empty())
        throw std::invalid_argument("Sender cannot be empty!");
    if (recipient.empty())
        throw std::invalid_argument("Recipient cannot be empty!");
    if (amount <= 0)
        throw std::invalid_argument("Amount must be positive!");
    if (signature.empty())
        throw std::invalid_argument("Signature cannot be empty!");
    if (gasLimit <= 0)
        throw std::invalid_argument("Gas limit must be positive!");
}

Transaction::Transaction(std::string sender, std::string recipient, double amount,
                         std::string signature, std::vector<uint8_t> contractCode,
                         int64_t gasLimit)
    : Transaction(sender, recipient, amount, signature, contractCode, gasLimit, "")
{
    // Делегируем вызов конструктору с gltchCode, передавая пустую строку
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
std::string Transaction::getSignature() const
{
    return signature;
}
std::vector<uint8_t> Transaction::getContractCode() const
{
    return contractCode;
}
int64_t Transaction::getGasLimit() const
{
    return gasLimit;
}
std::string Transaction::getGltchCode() const
{
    return gltchCode;
}

std::string Transaction::toString() const
{
    std::stringstream ss;
    ss << "Transaction(sender=" << sender << ", recipient=" << recipient
       << ", amount=" << amount << ", signature=" << signature
       << ", gasLimit=" << gasLimit << ")";
    return ss.str();
}
