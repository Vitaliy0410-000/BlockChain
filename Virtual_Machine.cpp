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
#include <exception>




class MyException : public std::exception {
private:
    std::string message;
public:
    MyException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};



Virtual_Machine::Virtual_Machine(std::vector<uint8_t>& bytecode,
    std::map<std::string, int64_t>& storage,
    Context context,int64_t gasLimit)
    : bytecode(bytecode), storage(storage), context(context), gasLimit(gasLimit),
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


enum class Opcode:uint8_t
{
    HALT=0x00,
    PUSH=0x01,
    POP=0x02,
    ADD=0x03,
    SUB=0x04,
    MUL=0x05,
    DIV=0x06,
    EQ=0x07,
    JUMP=0x08,
    JZ=0x09,
    JNZ=0x0A,
    SLOAD=0x0B,
    SSTORE=0x0C,
    CALL=0x0D,
    RET=0x0E,
    DUP=0x0F,
    SWAP=0x10,
    GT=0x11,
    LT=0x12,
    GTE=0x13,
    LTE=0x14,
    EMIT=0x15,
    TIME=0x16,
    BLOCKNUM=0x17,
    BALANCE=0x18,
    SENDER=0x19,
    TXVALUE=0x1A ,
    ASSERT=0x1B,
    LABEL=0x1C,
};
//используется big-endian (старший байт первый)
template<typename T>
T read_bytes_as_type()
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
    if (programCounter + length > bytecode.size()) {
        throw MyException("Attempt to read past end of bytecode (read_string)");
    }
    std::string str(&bytecode[programCounter], &bytecode[programCounter + length]);
    programCounter += length;
    return str;
}


void Virtual_Machine::executeOpcode(uint8_t opcodeValue)
{
      Opcode opcode = static_cast<Opcode>(opcodeValue);
    switch(opcode) {
      case Opcode:: HALT:
        programCounter = bytecode.size();
        gasUsed += 0;
        break;
      case Opcode::PUSH:
          gasUsed += 3;
          if (programCounter >= bytecode.size()) {
              throw MyException("Attempt to read past end of bytecode (PUSH type byte missing)");
          }
          if (stack_memory.size() >= 1000) {
              throw MyException("Stack overflow");
          }
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
          break;

      case Opcode:: POP:
          break;
      case Opcode:: ADD:
          break;
      case Opcode:: SUB:
          break;
      case Opcode:: MUL:
          break;
      case Opcode:: DIV:
          break;
      case Opcode:: EQ:
          break;
      case Opcode:: JUMP:
          break;
      case Opcode:: JZ:
          break;
      case Opcode:: JNZ:
          break;
      case Opcode:: SLOAD:
          break;
      case Opcode:: SSTORE:
          break;
      case Opcode:: CALL:
          break;
      case Opcode:: RET:
          break;
      case Opcode:: DUP:
          break;
      case Opcode:: SWAP:
          break;
      case Opcode:: GT:
          break;
      case Opcode:: LT:
          break;
      case Opcode:: GTE:
          break;
      case Opcode:: LTE:
          break;
      case Opcode:: EMIT:
          break;
      case Opcode:: TIME:
          break;
      case Opcode:: BLOCKNUM:
          break;
      case Opcode:: BALANCE:
          break;
      case Opcode:: SENDER:
          break;
      case Opcode:: TXVALUE:
          break;
      case Opcode:: ASSERT:
          break;
      case Opcode:: LABEL:
          break;

    default:
        std::runtime_error("Unknown opcode");
        break;
    }

}
