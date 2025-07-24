#ifndef SMARTCONTRACT_H
#define SMARTCONTRACT_H
#include <string>
#include <map>

class SmartContract {
private:
    std::string code;
    std::map<std::string, double> state;
public:
    SmartContract(const std::string& code, const std::map<std::string, double>& state);
    void execute();
    std::string getCode() const;
    const std::map<std::string, double>& getState() const;
    std::string getNextWord(std::stringstream& ss);
};
#endif
