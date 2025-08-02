#include "Transaction.h"
#include <string>
#include "Logger.h"
#include <sstream>
#include <iomanip>


Transaction::Transaction(const std::string& sender, const std::string& recipient, double amount,
                         const std::string& signature,const std::vector<uint8_t> contractCode, int64_t gasLimit)
    : sender(sender), recipient(recipient), amount(amount), contractCode(contractCode), signature(signature),gasLimit(gasLimit)
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
    if (gasLimit <= 0)
    {
        throw std::invalid_argument("Gas limit must be positive!");
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

 const std::vector<uint8_t>& Transaction::getContractCode() const
{
    return contractCode;
}

std::string Transaction::toString() const
{
    std::stringstream ss;
    ss << sender << ":" << recipient << ":" << std::fixed << std::setprecision(2) << amount << ":" << signature;
    if (!contractCode.empty()) {
        ss << ":contract{";
        for (size_t i = 0; i < contractCode.size(); ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(contractCode[i]);
            if (i < contractCode.size() - 1) ss << " ";
        }
        ss << "}";
    }
    return ss.str();
}

std::string Transaction::getSignature() const
{
    return signature;
}
int64_t Transaction::getGasLimit() const
{
    return gasLimit;
}
