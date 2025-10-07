#include "SmartContractParser.h"
#include <sstream>
#include <regex>
#include <stdexcept>
#include <algorithm>
#include <map>

SmartContractParser::SmartContractParser(const Virtual_Machine::Context& ctx)
    : tokens{}, currentTokenIndex{0}, ast{}, variables{}, context(ctx), bytecode{}, errorMessage{} {}

void SmartContractParser::tokenize(const std::string& code) {
    tokens.clear();
    std::stringstream ss(code);
    std::string word;
    int line = 1;

    const std::map<std::string, TokenType> keywords = {
        {"WHEN", TokenType::WHEN}, {"THEN", TokenType::THEN}, {"SET", TokenType::SET},
        {"STORE", TokenType::STORE}, {"LOAD", TokenType::LOAD}, {"EMIT", TokenType::EMIT},
        {"BALANCE", TokenType::BALANCE}, {"SENDER", TokenType::SENDER}, {"TXVALUE", TokenType::TXVALUE},
        {"TIME", TokenType::TIME}, {"BLOCKNUM", TokenType::BLOCKNUM}, {"IF", TokenType::IF},
        {"ELSE", TokenType::ELSE}, {"FOR", TokenType::FOR}, {"FUNC", TokenType::FUNC},
        {"RETURN", TokenType::RETURN}, {"ASSERT", TokenType::ASSERT}, {"CALL", TokenType::CALL},
        {"LET", TokenType::LET}
    };

    while (ss >> word) {
        if (word == ">=") {
            tokens.emplace_back(word, TokenType::GTE);
        } else if (word == "=") {
            tokens.emplace_back(word, TokenType::EQ);
        } else if (word == "+") {
            tokens.emplace_back(word, TokenType::PLUS);
        } else if (word == "-") {
            tokens.emplace_back(word, TokenType::MINUS);
        } else if (keywords.count(word)) {
            tokens.emplace_back(word, keywords.at(word));
        } else if (std::all_of(word.begin(), word.end(), ::isdigit)) {
            tokens.emplace_back(word, TokenType::NUMBER);
        } else {
            tokens.emplace_back(word, TokenType::IDENTIFIER);
        }
        if (ss.peek() == '\n') {
            line++;
            ss.get();
        }
    }
}

bool SmartContractParser::match(TokenType type) {
    if (currentTokenIndex < tokens.size()) {
        return tokens[currentTokenIndex].second == type;
    }
    return false;
}

std::pair<std::string, TokenType> SmartContractParser::nextToken() {
    if (currentTokenIndex < tokens.size()) {
        auto token = tokens[currentTokenIndex];
        currentTokenIndex++;
        return token;
    }
    reportError("Unexpected end of tokens");
    return {"", TokenType::IDENTIFIER};
}

void SmartContractParser::reportError(const std::string& message) {
    errorMessage = message;
    throw std::runtime_error(message);
}

std::unique_ptr<AstNode> SmartContractParser::parseExpression() {
    auto token = nextToken();
    int line = 1; // TODO: Добавить отслеживание строки

    if (token.second == TokenType::NUMBER) {
        return std::make_unique<AstNode>(AstNodeType::NUMBER, token.first, line);
    } else if (token.second == TokenType::IDENTIFIER || token.second == TokenType::BALANCE) {
        return std::make_unique<AstNode>(AstNodeType::IDENTIFIER, token.first, line);
    } else if (token.second == TokenType::PLUS || token.second == TokenType::MINUS) {
        auto exprNode = std::make_unique<AstNode>(AstNodeType::EXPR, token.first, line);
        auto left = parseExpression();
        auto right = parseExpression();
        exprNode->addChild(std::move(left));
        exprNode->addChild(std::move(right));
        return exprNode;
    } else {
        reportError("Invalid expression token: " + token.first);
        return nullptr;
    }
}

std::unique_ptr<AstNode> SmartContractParser::parseCondition() {
    auto conditionNode = std::make_unique<AstNode>(AstNodeType::CONDITION, "", 1);
    int line = 1;

    auto left = nextToken();
    if (left.second != TokenType::IDENTIFIER && left.second != TokenType::BALANCE) {
        reportError("Expected IDENTIFIER or BALANCE in condition, got: " + left.first);
    }
    conditionNode->addChild(std::make_unique<AstNode>(AstNodeType::IDENTIFIER, left.first, line));

    auto op = nextToken();
    if (op.second != TokenType::GTE) {
        reportError("Expected GTE in condition, got: " + op.first);
    }
    conditionNode->addChild(std::make_unique<AstNode>(AstNodeType::GTE, op.first, line));

    auto right = nextToken();
    if (right.second != TokenType::NUMBER) {
        reportError("Expected NUMBER in condition, got: " + right.first);
    }
    conditionNode->addChild(std::make_unique<AstNode>(AstNodeType::NUMBER, right.first, line));

    return conditionNode;
}

void SmartContractParser::buildAst() {
    ast.clear();
    int line = 1;

    while (currentTokenIndex < tokens.size()) {
        if (match(TokenType::WHEN)) {
            nextToken();
            auto whenNode = std::make_unique<AstNode>(AstNodeType::WHEN, "", line);

            auto condition = parseCondition();
            whenNode->addChild(std::move(condition));

            if (!match(TokenType::THEN)) {
                reportError("Expected THEN after condition");
            }
            nextToken();

            if (match(TokenType::STORE)) {
                nextToken();
                auto storeNode = std::make_unique<AstNode>(AstNodeType::STORE, "", line);

                auto id = nextToken();
                if (id.second != TokenType::IDENTIFIER) {
                    reportError("Expected IDENTIFIER after STORE, got: " + id.first);
                }
                storeNode->addChild(std::make_unique<AstNode>(AstNodeType::IDENTIFIER, id.first, line));

                if (!match(TokenType::EQ)) {
                    reportError("Expected EQ in STORE");
                }
                nextToken();

                auto expr = parseExpression();
                storeNode->addChild(std::move(expr));

                whenNode->addChild(std::move(storeNode));
            } else {
                reportError("Expected STORE after THEN");
            }

            ast.push_back(std::move(whenNode));
        } else {
            reportError("Expected WHEN, got: " + tokens[currentTokenIndex].first);
        }
    }
}

void SmartContractParser::generateExpressionBytecode(const std::unique_ptr<AstNode>& expr) {
    if (!expr) {
        reportError("Null expression node");
        return;
    }

    switch (expr->getType()) {
    case AstNodeType::NUMBER: {
        int64_t num = std::stoll(expr->getValue());
        bytecode.push_back(static_cast<uint8_t>(Opcode::PUSH));
        bytecode.push_back(0); // Тип: число
        for (int i = 7; i >= 0; --i) {
            bytecode.push_back((num >> (i * 8)) & 0xFF);
        }
        break;
    }
    case AstNodeType::IDENTIFIER: {
        uint32_t len = expr->getValue().size();
        bytecode.push_back(static_cast<uint8_t>(Opcode::PUSH));
        bytecode.push_back(1); // Тип: строка
        for (int i = 3; i >= 0; --i) {
            bytecode.push_back((len >> (i * 8)) & 0xFF);
        }
        for (char c : expr->getValue()) {
            bytecode.push_back(static_cast<uint8_t>(c));
        }
        bytecode.push_back(static_cast<uint8_t>(Opcode::SLOAD));
        break;
    }
    case AstNodeType::EXPR: {
        for (const auto& child : expr->getChildren()) {
            generateExpressionBytecode(child);
        }
        if (expr->getValue() == "-") {
            bytecode.push_back(static_cast<uint8_t>(Opcode::SUB));
        } else if (expr->getValue() == "+") {
            bytecode.push_back(static_cast<uint8_t>(Opcode::ADD));
        }
        break;
    }
    default:
        reportError("Unsupported expression type");
    }
}

std::vector<uint8_t> SmartContractParser::generateBytecode() {
    bytecode.clear();
    for (const auto& node : ast) {
        if (!node) {
            reportError("Null node in generateBytecode");
            continue;
        }

        switch (node->getType()) {
        case AstNodeType::WHEN: {
            auto& children = node->getChildren();
            if (children.size() < 2) {
                reportError("WHEN node must have condition and body");
                continue;
            }
            generateExpressionBytecode(children[0]);
            bytecode.push_back(static_cast<uint8_t>(Opcode::JUMP));
            size_t jumpIndex = bytecode.size();
            bytecode.push_back(0);
            generateExpressionBytecode(children[1]);
            bytecode[jumpIndex] = static_cast<uint8_t>(bytecode.size());
            break;
        }
        case AstNodeType::CONDITION: {
            auto& children = node->getChildren();
            if (children.size() != 3) {
                reportError("CONDITION node must have 3 children");
                continue;
            }
            generateExpressionBytecode(children[0]);
            generateExpressionBytecode(children[2]);
            if (children[1]->getType() == AstNodeType::GTE) {
                bytecode.push_back(static_cast<uint8_t>(Opcode::GTE));
            }
            break;
        }
        case AstNodeType::STORE: {
            auto& children = node->getChildren();
            if (children.size() != 2) {
                reportError("STORE node must have 2 children");
                continue;
            }
            generateExpressionBytecode(children[1]);
            bytecode.push_back(static_cast<uint8_t>(Opcode::SSTORE));
            break;
        }
        default:
            reportError("Unsupported node type");
        }
    }
    return bytecode;
}

std::vector<uint8_t> SmartContractParser::parse(const std::string& code) {
    tokenize(code);
    buildAst();
    return generateBytecode();
}

void SmartContractParser::setContext(const Virtual_Machine::Context& ctx) {
    context = ctx;
}

SmartContractParser::~SmartContractParser() = default;
