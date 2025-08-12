#ifndef ASTNODE_H
#define ASTNODE_H
#include <string>
#include <vector>
#include <memory>
//AstNode (Abstract Syntax Tree Node) — это узел абстрактного синтаксического дерева (AST), которое представляет структуру твоего кода на GLTCH Lang в виде дерева. AST — это промежуточное представление программы, которое создаётся после разбора (парсинга) кода и используется для генерации байт-кода для твоей Virtual_Machine.
//Простыми словами:Твой код на GLTCH Lang (например, WHEN BALANCE >= 100 THEN STORE balance = balance - 100) — это текст.
//Парсер разбивает его на токены (WHEN, BALANCE, >=, 100, и т.д.).
//Затем парсер строит дерево (AstNode), где каждая конструкция (например, WHEN, STORE, выражение balance - 100) — это узел.
//Из этого дерева ты генерируешь байт-код (PUSH, SLOAD, SSTORE, JUMPI, и т.д.) для Virtual_Machine.

// value Назначение: Хранит значение узла (например, "balance" для идентификатора, "100" для числа, "Transfer" для EMIT).
//Зачем: Используется для генерации байт-кода (например, PUSH "balance" для SLOAD).

//children.
//Назначение: Хранит указатели на дочерние узлы (например, для WHEN — узлы условия и тела).
//Зачем: Поддерживает вложенные конструкции (например, тело WHEN, выражение balance - 100).

// line.
//Назначение: Указывает строку в исходном коде для отладки.
//Зачем: Помогает в сообщениях об ошибках (например, "Syntax error at line 5").


//    AstNode* clone() const;Назначение: Создаёт глубокую копию узла и его children.
//Зачем: Может понадобиться, если будешь копировать части AST (например, для функций FUNC).

//void addChild(AstNode* child)Назначение: Добавляет дочерний узел в children.
//Зачем: Удобно для построения AST в buildAst (например, добавление условия и тела к WHEN).


enum class AstNodeType
{
    WHEN, SET, STORE, LOAD, EMIT, IF, ELSE, FOR, FUNC,
    RETURN, ASSERT, CALL, LET, CONDITION, EXPR, NUMBER, IDENTIFIER,PROGRAM,GTE
};
class AstNode
{
private:
    AstNodeType type;
    std::string value;
    std::vector<std::unique_ptr<AstNode>>children;
    int line;
public:
    AstNode(const AstNode&) = delete;
    AstNode& operator=(const AstNode&) = delete;
    AstNode(AstNodeType type, const std::string& value = "", int line = 0);
    AstNodeType getType() const;
    std::string getValue() const;
    const std::vector<std::unique_ptr<AstNode>>& getChildren() const;
    void addChild(std::unique_ptr<AstNode> child);
    std::unique_ptr<AstNode> clone() const;
    virtual ~AstNode() = default;
};

#endif // ASTNODE_H
