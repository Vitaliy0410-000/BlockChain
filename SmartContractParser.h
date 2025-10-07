#ifndef SMARTCONTRACTPARSER_H
#define SMARTCONTRACTPARSER_H
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "AstNode.h"
#include "Virtual_Machine.h"

enum class TokenType
{
    WHEN, THEN, SET, STORE, LOAD, EMIT, BALANCE, SENDER, TXVALUE,
    TIME, BLOCKNUM, IF, ELSE, FOR, FUNC, NUMBER, IDENTIFIER, GTE, EQ,
    PLUS, MINUS, RETURN, ASSERT, CALL, LET
};

class SmartContractParser
{
private:
    std::vector<std::pair<std::string, TokenType>> tokens;
    size_t currentTokenIndex;
    std::vector<std::unique_ptr<AstNode>> ast;
    std::map<std::string, int64_t> variables;
    Virtual_Machine::Context context;
    std::vector<uint8_t> bytecode;
    std::string errorMessage;

    std::unique_ptr<AstNode> parseExpression();
    std::unique_ptr<AstNode> parseCondition();

public:
    SmartContractParser(const Virtual_Machine::Context& ctx);
    std::vector<uint8_t> parse(const std::string& code);
    void tokenize(const std::string& code);
    void buildAst();
    std::vector<uint8_t> generateBytecode();
    bool match(TokenType type);
    std::pair<std::string, TokenType> nextToken();
    void reportError(const std::string& message);
    void generateExpressionBytecode(const std::unique_ptr<AstNode>& expr);
    void setContext(const Virtual_Machine::Context& ctx);
    ~SmartContractParser();
};

#endif // SMARTCONTRACTPARSER_H
