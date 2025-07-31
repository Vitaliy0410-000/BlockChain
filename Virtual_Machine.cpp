//HALT     | 0x00 | 0   | Остановить выполнение
//PUSH     | 0x01 | 3   | Поместить значение в стек
//POP      | 0x02 | 3   | Извлечь верхнее значение
//ADD      | 0x03 | 3   | Сложить два верхних значения
//SUB      | 0x04 | 3   | Вычесть два верхних значения
//MUL      | 0x05 | 6   | Умножить два верхних значения
//DIV      | 0x06 | 6   | Разделить два верхних значения
//EQ       | 0x07 | 3   | Сравнение на равенство
//JUMP     | 0x08 | 10  | Перейти по смещению
//JZ       | 0x09 | 10  | Перейти, если верхнее значение равно 0
//JNZ      | 0x0A | 10  | Перейти, если верхнее значение не равно 0
//SLOAD    | 0x0B | 20  | Загрузить из хранилища
//SSTORE   | 0x0C | 20  | 	Сохранить в хранилище
//CALL     | 0x0D | 15  | Вызвать функцию
//RET      | 0x0E | 3   | Вернуться из вызова
//DUP      | 0x0F | 3   | Дублировать верхнее значение
//SWAP     | 0x10 | 3   | Поменять местами два верхних значения
//GT       | 0x11 | 3   | Больше чем
//LT       | 0x12 | 3   | 	Меньше чем
//GTE      | 0x13 | 3   | Больше или равно
//LTE      | 0x14 | 3   | 	Меньше или равно
//EMIT     | 0x15 | 50  | Сгенерировать событие
//TIME     | 0x16 | 5   | Получить метку времени
//BLOCKNUM | 0x17 | 5   | Получить номер блока
//BALANCE  | 0x18 | 5   | Получить баланс
//SENDER   | 0x19 | 5   | Получить отправителя
//TXVALUE  | 0x1A | 5   | Получить стоимость транзакции
//ASSERT   | 0x1B | 5   | Утвердить, что верхнее значение истинно
//LABEL    | 0x1C | 10  | Метка



#include "Virtual_Machine.h"
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <stdexcept>

MyException::MyException(const std::string& msg)
    : message(msg) {};
const char* MyException::what() const noexcept
{
    return message.c_str();
}



Virtual_Machine::Virtual_Machine(std::vector<uint8_t>& bytecode,
    std::map<std::string, int64_t>& storage,
                                 Context context,std::vector<size_t> callStack,int64_t gasLimit)
    : bytecode(bytecode), storage(storage), context(context),callStack(callStack) ,gasLimit(gasLimit),
    gasUsed(0), programCounter(0), storageCopy(storage)
{
    if (bytecode.size() == 0) {
        throw std::runtime_error("Empty bytecode");
    }
}

bool  Virtual_Machine::execute()
{
    while(programCounter < bytecode.size())
    {

    }
}



//используется big-endian (старший байт первый)
template<typename T>
T Virtual_Machine::read_bytes_as_type()
{
    if (programCounter + sizeof(T) > bytecode.size())
    {
        throw std::runtime_error("Attempt to read past end of bytecode (read_bytes_as_type).");
    }
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        value = (value << 8) | bytecode[programCounter + i];
    }
    programCounter += sizeof(T);
    return value;
}
std::string Virtual_Machine::read_string(uint32_t length) {
    if (programCounter + length > bytecode.size())
    {
        throw MyException("Attempt to read past end of bytecode (read_string)");
    }
    std::string str(&bytecode[programCounter], &bytecode[programCounter + length]);
    programCounter += length;
    return str;
}


Virtual_Machine::Value Virtual_Machine::popValue()
{
    if (stack_memory.empty())
    {
        throw MyException("Stack underflow");
    }

    Value result = stack_memory.back();
    stack_memory.pop_back();

    return result;
}


void Virtual_Machine::executeOpcode(uint8_t opcodeValue) {
    Opcode opcode = static_cast<Opcode>(opcodeValue);
    switch (opcode) {
    case Opcode::HALT:
        gasUsed += 0;
        programCounter = bytecode.size();
        break;

    case Opcode::PUSH:
        gasUsed += 3;
        if (programCounter >= bytecode.size()) {
            throw MyException("Attempt to read past end of bytecode (PUSH type byte missing)");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            uint8_t type = bytecode[programCounter++];
            if (type == 0) {
                int64_t num_val = read_bytes_as_type<int64_t>();
                pushValue(Value(num_val));
            } else if (type == 1) {
                uint32_t string_length = read_bytes_as_type<uint32_t>();
                std::string str_val = read_string(string_length);
                pushValue(Value(str_val));
            } else {
                throw MyException("Unknown PUSH type: " + std::to_string(type));
            }
        }
        break;

    case Opcode::POP:
        gasUsed += 3;
        popValue();
        break;

    case Opcode::ADD:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for ADD: expected numbers");
            }
            pushValue(Value(a.num + b.num));
        }
        break;

    case Opcode::SUB:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for SUB: expected numbers");
            }
            pushValue(Value(a.num - b.num));
        }
        break;

    case Opcode::MUL:
        gasUsed += 6;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for MUL: expected numbers");
            }
            pushValue(Value(a.num * b.num));
        }
        break;

    case Opcode::DIV:
        gasUsed += 6;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for DIV: expected numbers");
            }
            if (b.num == 0) {
                throw MyException("Division by zero");
            }
            pushValue(Value(a.num / b.num));
        }
        break;

    case Opcode::EQ:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString != b.isString) {
                throw MyException("Invalid type for EQ: mismatched types");
            }
            if (a.isString) {
                pushValue(Value(a.str == b.str ? 1 : 0));
            } else {
                pushValue(Value(a.num == b.num ? 1 : 0));
            }
        }
        break;

    case Opcode::JUMP:
        gasUsed += 10;
        {
            int64_t offset = read_bytes_as_type<int64_t>();
            if (offset < 0 || static_cast<size_t>(offset) >= bytecode.size()) {
                throw MyException("Invalid JUMP offset");
            }
            programCounter = static_cast<size_t>(offset);
        }
        break;

    case Opcode::JZ:
        gasUsed += 10;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        {
            Value cond = popValue();
            if (cond.isString) {
                throw MyException("Invalid type for JZ: expected number");
            }
            int64_t offset = read_bytes_as_type<int64_t>();
            if (offset < 0 || static_cast<size_t>(offset) >= bytecode.size()) {
                throw MyException("Invalid JZ offset");
            }
            if (cond.num == 0) {
                programCounter = static_cast<size_t>(offset);
            }
        }
        break;

    case Opcode::JNZ:
        gasUsed += 10;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        {
            Value cond = popValue();
            if (cond.isString) {
                throw MyException("Invalid type for JNZ: expected number");
            }
            int64_t offset = read_bytes_as_type<int64_t>();
            if (offset < 0 || static_cast<size_t>(offset) >= bytecode.size()) {
                throw MyException("Invalid JNZ offset");
            }
            if (cond.num != 0) {
                programCounter = static_cast<size_t>(offset);
            }
        }
        break;

    case Opcode::SLOAD:
        gasUsed += 20;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value key = popValue();
            if (!key.isString) {
                throw MyException("Invalid type for SLOAD: expected string key");
            }
            auto it = storage.find(key.str);
            if (it == storage.end()) {
                pushValue(Value(0));
            } else {
                pushValue(Value(it->second));
            }
        }
        break;

    case Opcode::SSTORE:
        gasUsed += 20;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        {
            Value value = popValue();
            Value key = popValue();
            if (!key.isString) {
                throw MyException("Invalid type for SSTORE: expected string key");
            }
            if (value.isString) {
                throw MyException("Invalid type for SSTORE: expected number value");
            }
            storage[key.str] = value.num;
        }
        break;

    case Opcode::CALL:
        gasUsed += 15;
        {
            int64_t offset = read_bytes_as_type<int64_t>();
            if (offset < 0 || static_cast<size_t>(offset) >= bytecode.size()) {
                throw MyException("Invalid CALL offset");
            }
            callStack.push_back(programCounter);
            programCounter = static_cast<size_t>(offset);
        }
        break;

    case Opcode::RET:
        gasUsed += 3;
        if (callStack.empty()) {
            throw MyException("Call stack underflow");
        }
        programCounter = callStack.back();
        callStack.pop_back();
        break;

    case Opcode::DUP:
        gasUsed += 3;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(stack_memory.back());
        break;

    case Opcode::SWAP:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            pushValue(b);
            pushValue(a);
        }
        break;

    case Opcode::GT:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for GT: expected numbers");
            }
            pushValue(Value(a.num > b.num ? 1 : 0));
        }
        break;

    case Opcode::LT:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for LT: expected numbers");
            }
            pushValue(Value(a.num < b.num ? 1 : 0));
        }
        break;

    case Opcode::GTE:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for GTE: expected numbers");
            }
            pushValue(Value(a.num >= b.num ? 1 : 0));
        }
        break;

    case Opcode::LTE:
        gasUsed += 3;
        if (stack_memory.size() < 2) {
            throw MyException("Stack underflow");
        }
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        {
            Value b = popValue();
            Value a = popValue();
            if (a.isString || b.isString) {
                throw MyException("Invalid type for LTE: expected numbers");
            }
            pushValue(Value(a.num <= b.num ? 1 : 0));
        }
        break;

    case Opcode::EMIT:
        gasUsed += 50;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        {
            Value event = popValue();
            if (!event.isString) {
                throw MyException("Invalid type for EMIT: expected string");
            }
            log.push_back("Event:" + event.str);
        }
        break;

    case Opcode::TIME:
        gasUsed += 5;
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(Value(context.time));
        break;

    case Opcode::BLOCKNUM:
        gasUsed += 5;
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(Value(context.blockNum));
        break;

    case Opcode::BALANCE:
        gasUsed += 5;
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(Value(context.balance));
        break;

    case Opcode::SENDER:
        gasUsed += 5;
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(Value(context.sender));
        break;

    case Opcode::TXVALUE:
        gasUsed += 5;
        if (stack_memory.size() >= 1000) {
            throw MyException("Stack overflow");
        }
        pushValue(Value(context.txValue));
        break;

    case Opcode::ASSERT:
        gasUsed += 5;
        if (stack_memory.empty()) {
            throw MyException("Stack underflow");
        }
        {
            Value cond = popValue();
            if (cond.isString) {
                throw MyException("Invalid type for ASSERT: expected number");
            }
            if (cond.num == 0) {
                throw MyException("Assertion failed");
            }
        }
        break;

    case Opcode::LABEL:
        gasUsed += 10;
        break;

    default:
        throw MyException("Unknown opcode: " + std::to_string(opcodeValue));
    }
}
