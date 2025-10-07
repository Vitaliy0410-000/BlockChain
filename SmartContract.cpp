#include "SmartContract.h"
#include <sstream>
#include <iomanip>
#include "Logger.h"

SmartContract::SmartContract(const std::string& code, const std::map<std::string, double>& state)
    : code(code), state(state) {
    if (state.empty()) {
        this->state["Alice"] = 100.0;
        this->state["Bob"] = 50.0;
    }
}

std::string SmartContract::getNextWord(std::stringstream& ss) {
    std::string word;
    if (!(ss >> word)) {
        throw std::invalid_argument("Недостаточно слов в коде контракта.");
    }
    return word;
}

void SmartContract::execute() {
    Logger::getInstance().log("Executing contract: " + code);
    std::stringstream ss(code);

    // Шаг 1: Проверяем "if"
    std::string word = getNextWord(ss);
    if (word != "if") {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось 'if'. Найдено: '" + word + "'.");
    }

    // Шаг 2: Проверяем "balance[<sender>]" и извлекаем sender
    word = getNextWord(ss);
    if (word.rfind("balance[", 0) != 0 || word.back() != ']') {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось 'balance[<sender>]'. Найдено: '" + word + "'.");
    }
    std::string sender = word.substr(8, word.length() - 9);

    // Шаг 3: Проверяем ">= <amount>"
    word = getNextWord(ss);
    if (word != ">=") {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось '>='. Найдено: '" + word + "'.");
    }
    double amount;
    try {
        amount = std::stod(getNextWord(ss));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Ошибка синтаксиса: Неверный формат числа в '<amount>'.");
    }

    // Шаг 4: Проверяем "then transfer <amount> to <recipient>"
    word = getNextWord(ss);
    if (word != "then") {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось 'then'. Найдено: '" + word + "'.");
    }
    word = getNextWord(ss);
    if (word != "transfer") {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось 'transfer'. Найдено: '" + word + "'.");
    }
    double transferAmount;
    try {
        transferAmount = std::stod(getNextWord(ss));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Ошибка синтаксиса: Неверный формат числа в 'transfer <amount>'.");
    }
    if (transferAmount != amount) {
        throw std::invalid_argument("Ошибка синтаксиса: Суммы в условии и transfer не совпадают.");
    }
    word = getNextWord(ss);
    if (word != "to") {
        throw std::invalid_argument("Ошибка синтаксиса: Ожидалось 'to'. Найдено: '" + word + "'.");
    }
    std::string recipient = getNextWord(ss);

    // Шаг 5: Проверяем, что больше нет слов
    if (ss >> word) {
        throw std::invalid_argument("Ошибка синтаксиса: Лишние слова в коде контракта: '" + word + "'.");
    }

    // Шаг 6: Выполняем условие и действие
    if (state.find(sender) == state.end()) {
        Logger::getInstance().log("Ошибка: Отправитель '" + sender + "' не найден в состоянии.");
        return;
    }
    if (state[sender] >= amount) {
        state[sender] -= amount;
        state[recipient] += amount;
        std::stringstream logMsg;
        logMsg << std::fixed << std::setprecision(2) << "Transferred " << amount << " from " << sender << " to " << recipient;
        Logger::getInstance().log(logMsg.str());
    } else {
        Logger::getInstance().log("Insufficient balance for " + sender);
    }
}

std::string SmartContract::getCode() const {
    return code;
}

const std::map<std::string, double>& SmartContract::getState() const {
    return state;
}
