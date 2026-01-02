#include "parser.h"
#include <stdexcept>
#include <iostream>
#include <memory>

Parser::Parser(const std::vector<Token>& tokens) 
    : tokens(tokens), current(0) {}

std::unique_ptr<ASTNode> Parser::parse() {
    try {
        if (tokens.empty()) {
            throw std::runtime_error("No tokens provided");
        }
        return parseProgram();
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        throw;
    }
}

std::unique_ptr<ASTNode> Parser::parseProgram() {
    auto statements = std::make_unique<std::vector<std::unique_ptr<ASTNode>>>();
    
    while (!isAtEnd()) {
        statements->push_back(parseStatement());
    }
    
    return std::make_unique<ProgramNode>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    // Skip newlines
    while (match(TokenType::NEWLINE)) {
        // Continue
    }
    
    if (isAtEnd()) {
        throw std::runtime_error("Unexpected end of input");
    }
    
    switch (peek().type) {
        case TokenType::IF:
            return parseIfStatement();
        case TokenType::WHILE:
            return parseWhileStatement();
        case TokenType::FOR:
            return parseForStatement();
        case TokenType::FUNCTION:
            return parseFunctionDeclaration();
        case TokenType::RETURN:
            return parseReturnStatement();
        case TokenType::LEFT_BRACE:
            return parseBlock();
        default:
            return parseExpressionStatement();
    }
}

std::unique_ptr<ASTNode> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    
    // Consume optional semicolon or newline
    if (match(TokenType::SEMICOLON) || match(TokenType::NEWLINE)) {
        // Statement terminator consumed
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    consume(TokenType::IF, "Expected 'if'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    
    auto condition = parseExpression();
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto thenBranch = parseStatement();
    
    std::unique_ptr<ASTNode> elseBranch = nullptr;
    if (match(TokenType::ELSE)) {
        elseBranch = parseStatement();
    }
    
    return std::make_unique<IfNode>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

std::unique_ptr<ASTNode> Parser::parseWhileStatement() {
    consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    
    auto condition = parseExpression();
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = parseStatement();
    
    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseForStatement() {
    consume(TokenType::FOR, "Expected 'for'");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");
    
    std::unique_ptr<ASTNode> init = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        init = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for loop initializer");
    
    std::unique_ptr<ASTNode> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for loop condition");
    
    std::unique_ptr<ASTNode> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = parseExpression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses");
    
    auto body = parseStatement();
    
    return std::make_unique<ForNode>(
        std::move(init),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

std::unique_ptr<ASTNode> Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNCTION, "Expected 'function'");
    
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    
    auto params = std::make_unique<std::vector<std::string>>();
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params->push_back(param.value);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    
    auto body = parseBlock();
    
    return std::make_unique<FunctionNode>(
        name.value,
        std::move(params),
        std::move(body)
    );
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    consume(TokenType::RETURN, "Expected 'return'");
    
    std::unique_ptr<ASTNode> value = nullptr;
    if (!check(TokenType::SEMICOLON) && !check(TokenType::NEWLINE) && !isAtEnd()) {
        value = parseExpression();
    }
    
    if (match(TokenType::SEMICOLON) || match(TokenType::NEWLINE)) {
        // Statement terminator consumed
    }
    
    return std::make_unique<ReturnNode>(std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    consume(TokenType::LEFT_BRACE, "Expected '{'");
    
    auto statements = std::make_unique<std::vector<std::unique_ptr<ASTNode>>>();
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements->push_back(parseStatement());
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}'");
    
    return std::make_unique<BlockNode>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    
    if (match(TokenType::ASSIGN)) {
        auto value = parseAssignment();
        if (auto* var = dynamic_cast<VariableNode*>(expr.get())) {
            return std::make_unique<AssignmentNode>(
                var->name,
                std::move(value)
            );
        }
        throw std::runtime_error("Invalid assignment target");
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    auto expr = parseRelational();
    
    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::NOT_EQUAL)) {
        Token op = previous();
        auto right = parseRelational();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseRelational() {
    auto expr = parseAdditive();
    
    while (match(TokenType::LESS) || match(TokenType::LESS_EQUAL) ||
           match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL)) {
        Token op = previous();
        auto right = parseAdditive();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseAdditive() {
    auto expr = parseMultiplicative();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = parseMultiplicative();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicative() {
    auto expr = parseUnary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE) || match(TokenType::MODULO)) {
        Token op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryOpNode>(
            std::move(expr),
            op.type,
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS)) {
        Token op = previous();
        auto expr = parseUnary();
        return std::make_unique<UnaryOpNode>(op.type, std::move(expr));
    }
    
    return parsePostfix();
}

std::unique_ptr<ASTNode> Parser::parsePostfix() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::LEFT_PAREN)) {
            // Function call
            auto args = std::make_unique<std::vector<std::unique_ptr<ASTNode>>>();
            
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args->push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            
            consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
            
            expr = std::make_unique<CallNode>(std::move(expr), std::move(args));
        } else if (match(TokenType::LEFT_BRACKET)) {
            // Array access
            auto index = parseExpression();
            consume(TokenType::RIGHT_BRACKET, "Expected ']' after array index");
            
            expr = std::make_unique<IndexNode>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (match(TokenType::NUMBER)) {
        Token value = previous();
        return std::make_unique<NumberNode>(std::stod(value.value));
    }
    
    if (match(TokenType::STRING)) {
        Token value = previous();
        return std::make_unique<StringNode>(value.value);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        Token name = previous();
        return std::make_unique<VariableNode>(name.value);
    }
    
    if (match(TokenType::TRUE)) {
        return std::make_unique<BooleanNode>(true);
    }
    
    if (match(TokenType::FALSE)) {
        return std::make_unique<BooleanNode>(false);
    }
    
    if (match(TokenType::NULL_TOKEN)) {
        return std::make_unique<NullNode>();
    }
    
    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (match(TokenType::LEFT_BRACKET)) {
        // Array literal
        auto elements = std::make_unique<std::vector<std::unique_ptr<ASTNode>>>();
        
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements->push_back(parseExpression());
            } while (match(TokenType::COMMA));
        }
        
        consume(TokenType::RIGHT_BRACKET, "Expected ']' after array elements");
        
        return std::make_unique<ArrayNode>(std::move(elements));
    }
    
    throw std::runtime_error(std::string("Unexpected token: ") + peek().value);
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return current >= tokens.size() || peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() const {
    if (current >= tokens.size()) {
        static Token eof{TokenType::END_OF_FILE, "", 0};
        return eof;
    }
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(message + " at line " + std::to_string(peek().line));
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        
        switch (peek().type) {
            case TokenType::FUNCTION:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        
        advance();
    }
}
