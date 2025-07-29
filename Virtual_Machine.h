#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H
#include <vector>
#include <string>
#include <map>
#include <cstdint>

class Virtual_Machine
{
private:
    union Value {
        int64_t num;
        std::string str;
        bool isString;

        Value(bool isString = false) : isString(isString) {
            if (isString) new (&str) std::string();
            else num = 0;
        }

        ~Value() {
            if (isString) str.~basic_string();
        }

        Value(const Value& other) : isString(other.isString) {
            if (isString) new (&str) std::string(other.str);
            else num = other.num;
        }

        Value(Value&& other) noexcept : isString(other.isString) {
            if (isString) new (&str) std::string(std::move(other.str));
            else num = other.num;
        }

        Value& operator=(const Value& other) {
            if (this != &other) {
                if (isString) str.~basic_string();
                isString = other.isString;
                if (isString) new (&str) std::string(other.str);
                else num = other.num;
            }
            return *this;
        }

        Value& operator=(Value&& other) noexcept {
            if (this != &other) {
                if (isString) str.~basic_string();
                isString = other.isString;
                if (isString) new (&str) std::string(std::move(other.str));
                else num = other.num;
            }
            return *this;
        }
    };
    //Зачем: Хранит значение в стеке (stack_memory), которое может быть числом (int64_t) или строкой (std::string) — например, 100 или "Alice".
   // Роль: Поддерживает типы данных для PUSH, SLOAD, SENDER (строки для адресов/ключей, числа для балансов).
    // isString указывает, активно ли поле str или num.
    std::vector<union Value>stack_memory;//Стек для временных вычислений (например, для PUSH, ADD, SLOAD)
    std::map<std::string, int64_t>&storage;//Зачем: Ссылка на globalState блокчейна, хранит постоянные данные контрактов (например, "balance[Alice]": 100).
    //Роль: Используется для SLOAD (чтение) и SSTORE (запись).
    struct Context//Зачем: Хранит контекст транзакции.
    {
        std::string sender;
        int64_t txValue;
        int64_t balance;
        int64_t time;
        int blockNum;
    };
    std::vector<std::string>log;//Зачем: Хранит события от EMIT (например, "Transfer:Alice:Bob:100").
    int64_t gasUsed;//Отслеживает, сколько газа потрачено на выполнение opcodes.
    int64_t gasLimit;//Зачем: Максимальный газ для транзакции, берётся из Transaction::gasLimit.
    size_t programCounter;//Зачем: Указывает на текущий байт в bytecode (индекс)
    std::vector<uint8_t>& bytecode;//Зачем: Ссылка на байт-код контракта из Transaction::contractCode
    std::map<std::string, int64_t> storageCopy;//Зачем: Копия storage для отката при ошибке.
public:
    Virtual_Machine(std::vector<uint8_t>& bytecode, std::map<std::string, int64_t>& storage, Context context, int64_t gasLimit);
    bool execute();
    //Зачем: Выполняет байт-код контракта.
    //Роль:Читает bytecode[programCounter], вызывает executeOpcode для каждого opcode.
    //Проверяет gasUsed <= gasLimit.
    //Возвращает true (успех) или false (ошибка, вызывает revert()).
    void revert();//Зачем: Откатывает изменения при ошибке.
    void pushValue(Value);//Зачем: Добавляет значение (int64_t или std::string) в stack_memory.
    Value popValue();//Зачем: Извлекает и возвращает верхнее значение из stack_memory.
    void executeOpcode(uint8_t);//Зачем: Выполняет один opcode (PUSH, ADD, SLOAD, SSTORE, EMIT, JUMPI).



};

#endif // VIRTUAL_MACHINE_H
