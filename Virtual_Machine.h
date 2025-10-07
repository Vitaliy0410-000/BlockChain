#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <stdexcept>


class MyException: public std::exception{
private:
    std::string message;
public:
    MyException(const std::string& msg);
    const char* what() const noexcept override;
};

enum class Opcode : uint8_t {
    HALT = 0x00, PUSH = 0x01, POP = 0x02, ADD = 0x03, SUB = 0x04,
    MUL = 0x05, DIV = 0x06, EQ = 0x07, JUMP = 0x08, JZ = 0x09,
    JNZ = 0x0A, SLOAD = 0x0B, SSTORE = 0x0C, CALL = 0x0D, RET = 0x0E,
    DUP = 0x0F, SWAP = 0x10, GT = 0x11, LT = 0x12, GTE = 0x13,
    LTE = 0x14, EMIT = 0x15, TIME = 0x16, BLOCKNUM = 0x17, BALANCE = 0x18,
    SENDER = 0x19, TXVALUE = 0x1A, ASSERT = 0x1B, LABEL = 0x1C
};

class Virtual_Machine
{
private:
    struct Value {
        bool isString;  // ✅ Переносим сюда

        union {
            int64_t num;
            std::string str;
        };

        Value(int64_t n) : isString(false), num(n) {}
        Value(const std::string& s) : isString(true) { new (&str) std::string(s); }

        Value(const Value& other) : isString(other.isString) {
            if (isString) new (&str) std::string(other.str);
            else num = other.num;
        }

        Value(Value&& other) noexcept : isString(other.isString) {
            if (isString) new (&str) std::string(std::move(other.str));
            else num = other.num;
        }

        ~Value() {
            if (isString) str.~basic_string();
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
    std::vector< Value>stack_memory;//Стек для временных вычислений (например, для PUSH, ADD, SLOAD)
    std::map<std::string, int64_t>&storage;//Зачем: Ссылка на globalState блокчейна, хранит постоянные данные контрактов (например, "balance[Alice]": 100).
    //Роль: Используется для SLOAD (чтение) и SSTORE (запись).
    std::vector<size_t> callStack;
    std::vector<std::string>log;//Зачем: Хранит события от EMIT (например, "Transfer:Alice:Bob:100").
    int64_t gasUsed;//Отслеживает, сколько газа потрачено на выполнение opcodes.
    int64_t gasLimit;//Зачем: Максимальный газ для транзакции, берётся из Transaction::gasLimit.
    size_t programCounter;//Зачем: Указывает на текущий байт в bytecode (индекс)
    std::vector<uint8_t> bytecode;//Зачем: Ссылка на байт-код контракта из Transaction::contractCode
    std::map<std::string, int64_t> storageCopy;//Зачем: Копия storage для отката при ошибке.
public:
    std::vector<std::string>& getLog();
    struct Context//Зачем: Хранит контекст транзакции.
    {
        std::string sender;
        int64_t txValue;
        int64_t balance;
        int64_t time;
        int blockNum;
    }context;
    template<typename T>
    T read_bytes_as_type();
    Virtual_Machine(const std::vector<uint8_t>& bytecode,
                                     std::map<std::string, int64_t>& storage,
                    Context context,std::vector<size_t> callStack,int64_t gasLimit);
    bool execute();
    //Зачем: Выполняет байт-код контракта.
    //Роль:Читает bytecode[programCounter], вызывает executeOpcode для каждого opcode.
    //Проверяет gasUsed <= gasLimit.
    //Возвращает true (успех) или false (ошибка, вызывает revert()).
    void revert();//Зачем: Откатывает изменения при ошибке.
    void pushValue(Value);//Зачем: Добавляет значение (int64_t или std::string) в stack_memory.
    Value popValue();//Зачем: Извлекает и возвращает верхнее значение из stack_memory.
    void executeOpcode(uint8_t);//Зачем: Выполняет один opcode (PUSH, ADD, SLOAD, SSTORE, EMIT, JUMPI).
    std::string read_string(uint32_t length);//вспомогательная функция для чтения строк


};

#endif // VIRTUAL_MACHINE_H
