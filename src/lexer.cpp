#include "lexer.h"
#include <cctype>
#include <string>
#include <unordered_map>

// Keywords map for C++ style tokenization
static const std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"return", TokenType::RETURN},
    {"function", TokenType::FUNCTION},
    {"var", TokenType::VAR},
    {"const", TokenType::CONST},
    {"class", TokenType::CLASS},
    {"struct", TokenType::STRUCT},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_TOKEN},
    {"void", TokenType::VOID},
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT},
    {"string", TokenType::STRING},
    {"bool", TokenType::BOOL},
};

Lexer::Lexer(const std::string& source) 
    : source(source), position(0), line(1), column(1) {
}

Token Lexer::nextToken() {
    skipWhitespaceAndComments();
    
    if (position >= source.length()) {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }
    
    char current = source[position];
    
    // Single character tokens
    if (current == '(') {
        return makeToken(TokenType::LEFT_PAREN, "(");
    }
    if (current == ')') {
        return makeToken(TokenType::RIGHT_PAREN, ")");
    }
    if (current == '{') {
        return makeToken(TokenType::LEFT_BRACE, "{");
    }
    if (current == '}') {
        return makeToken(TokenType::RIGHT_BRACE, "}");
    }
    if (current == '[') {
        return makeToken(TokenType::LEFT_BRACKET, "[");
    }
    if (current == ']') {
        return makeToken(TokenType::RIGHT_BRACKET, "]");
    }
    if (current == ',') {
        return makeToken(TokenType::COMMA, ",");
    }
    if (current == ';') {
        return makeToken(TokenType::SEMICOLON, ";");
    }
    if (current == ':') {
        return makeToken(TokenType::COLON, ":");
    }
    if (current == '?') {
        return makeToken(TokenType::QUESTION, "?");
    }
    if (current == '.') {
        return makeToken(TokenType::DOT, ".");
    }
    
    // Operators and two-character tokens
    if (current == '=') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::EQUAL_EQUAL, "==");
        }
        return makeToken(TokenType::EQUAL, "=");
    }
    
    if (current == '!') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::NOT_EQUAL, "!=");
        }
        return makeToken(TokenType::NOT, "!");
    }
    
    if (current == '<') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::LESS_EQUAL, "<=");
        }
        if (peek() == '<') {
            advance();
            return makeToken(TokenType::LEFT_SHIFT, "<<");
        }
        return makeToken(TokenType::LESS, "<");
    }
    
    if (current == '>') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::GREATER_EQUAL, ">=");
        }
        if (peek() == '>') {
            advance();
            return makeToken(TokenType::RIGHT_SHIFT, ">>");
        }
        return makeToken(TokenType::GREATER, ">");
    }
    
    if (current == '&') {
        if (peek() == '&') {
            advance();
            return makeToken(TokenType::AND_AND, "&&");
        }
        return makeToken(TokenType::AND, "&");
    }
    
    if (current == '|') {
        if (peek() == '|') {
            advance();
            return makeToken(TokenType::OR_OR, "||");
        }
        return makeToken(TokenType::OR, "|");
    }
    
    if (current == '+') {
        if (peek() == '+') {
            advance();
            return makeToken(TokenType::PLUS_PLUS, "++");
        }
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::PLUS_EQUAL, "+=");
        }
        return makeToken(TokenType::PLUS, "+");
    }
    
    if (current == '-') {
        if (peek() == '-') {
            advance();
            return makeToken(TokenType::MINUS_MINUS, "--");
        }
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::MINUS_EQUAL, "-=");
        }
        if (peek() == '>') {
            advance();
            return makeToken(TokenType::ARROW, "->");
        }
        return makeToken(TokenType::MINUS, "-");
    }
    
    if (current == '*') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::STAR_EQUAL, "*=");
        }
        return makeToken(TokenType::STAR, "*");
    }
    
    if (current == '/') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::SLASH_EQUAL, "/=");
        }
        return makeToken(TokenType::SLASH, "/");
    }
    
    if (current == '%') {
        if (peek() == '=') {
            advance();
            return makeToken(TokenType::PERCENT_EQUAL, "%=");
        }
        return makeToken(TokenType::PERCENT, "%");
    }
    
    if (current == '^') {
        return makeToken(TokenType::CARET, "^");
    }
    
    if (current == '~') {
        return makeToken(TokenType::TILDE, "~");
    }
    
    // String literals
    if (current == '"') {
        return scanString();
    }
    
    // Character literals
    if (current == '\'') {
        return scanCharacter();
    }
    
    // Numbers
    if (std::isdigit(current)) {
        return scanNumber();
    }
    
    // Identifiers and keywords
    if (std::isalpha(current) || current == '_') {
        return scanIdentifier();
    }
    
    // Unknown character
    Token token(TokenType::ERROR, std::string(1, current), line, column);
    advance();
    return token;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();
    
    while (token.type != TokenType::END_OF_FILE) {
        tokens.push_back(token);
        token = nextToken();
    }
    
    tokens.push_back(token); // Add EOF token
    return tokens;
}

// Private helper methods

void Lexer::advance() {
    if (position < source.length()) {
        if (source[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}

char Lexer::peek(int offset) const {
    size_t lookahead = position + offset;
    if (lookahead >= source.length()) {
        return '\0';
    }
    return source[lookahead];
}

void Lexer::skipWhitespaceAndComments() {
    while (position < source.length()) {
        char current = source[position];
        
        // Skip whitespace
        if (std::isspace(current)) {
            advance();
            continue;
        }
        
        // Skip single-line comments
        if (current == '/' && peek() == '/') {
            advance(); // skip first /
            advance(); // skip second /
            while (position < source.length() && source[position] != '\n') {
                advance();
            }
            if (position < source.length()) {
                advance(); // skip newline
            }
            continue;
        }
        
        // Skip multi-line comments
        if (current == '/' && peek() == '*') {
            advance(); // skip /
            advance(); // skip *
            while (position < source.length()) {
                if (source[position] == '*' && peek() == '/') {
                    advance(); // skip *
                    advance(); // skip /
                    break;
                }
                advance();
            }
            continue;
        }
        
        break;
    }
}

Token Lexer::scanString() {
    size_t start = position;
    int startLine = line;
    int startColumn = column;
    
    advance(); // skip opening quote
    std::string value;
    
    while (position < source.length() && source[position] != '"') {
        if (source[position] == '\\' && peek() != '\0') {
            advance();
            char escaped = source[position];
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '0': value += '\0'; break;
                default: value += escaped;
            }
            advance();
        } else {
            value += source[position];
            advance();
        }
    }
    
    if (position < source.length()) {
        advance(); // skip closing quote
    }
    
    return Token(TokenType::STRING, value, startLine, startColumn);
}

Token Lexer::scanCharacter() {
    size_t start = position;
    int startLine = line;
    int startColumn = column;
    
    advance(); // skip opening quote
    std::string value;
    
    if (position < source.length() && source[position] == '\\') {
        advance();
        char escaped = source[position];
        switch (escaped) {
            case 'n': value += '\n'; break;
            case 't': value += '\t'; break;
            case 'r': value += '\r'; break;
            case '\\': value += '\\'; break;
            case '\'': value += '\''; break;
            case '0': value += '\0'; break;
            default: value += escaped;
        }
        advance();
    } else if (position < source.length()) {
        value += source[position];
        advance();
    }
    
    if (position < source.length() && source[position] == '\'') {
        advance(); // skip closing quote
    }
    
    return Token(TokenType::CHAR, value, startLine, startColumn);
}

Token Lexer::scanNumber() {
    size_t start = position;
    int startLine = line;
    int startColumn = column;
    std::string value;
    
    // Handle hex numbers
    if (source[position] == '0' && (peek() == 'x' || peek() == 'X')) {
        value += source[position];
        advance();
        value += source[position];
        advance();
        while (position < source.length() && std::isxdigit(source[position])) {
            value += source[position];
            advance();
        }
        return Token(TokenType::NUMBER, value, startLine, startColumn);
    }
    
    // Handle decimal and float numbers
    while (position < source.length() && std::isdigit(source[position])) {
        value += source[position];
        advance();
    }
    
    // Check for floating point
    if (position < source.length() && source[position] == '.' && 
        position + 1 < source.length() && std::isdigit(source[position + 1])) {
        value += source[position];
        advance();
        while (position < source.length() && std::isdigit(source[position])) {
            value += source[position];
            advance();
        }
        
        // Handle scientific notation
        if (position < source.length() && (source[position] == 'e' || source[position] == 'E')) {
            value += source[position];
            advance();
            if (position < source.length() && (source[position] == '+' || source[position] == '-')) {
                value += source[position];
                advance();
            }
            while (position < source.length() && std::isdigit(source[position])) {
                value += source[position];
                advance();
            }
        }
        
        return Token(TokenType::FLOAT, value, startLine, startColumn);
    }
    
    return Token(TokenType::NUMBER, value, startLine, startColumn);
}

Token Lexer::scanIdentifier() {
    size_t start = position;
    int startLine = line;
    int startColumn = column;
    std::string value;
    
    while (position < source.length() && 
           (std::isalnum(source[position]) || source[position] == '_')) {
        value += source[position];
        advance();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, value, startLine, startColumn);
    }
    
    return Token(TokenType::IDENTIFIER, value, startLine, startColumn);
}

Token Lexer::makeToken(TokenType type, const std::string& value) {
    Token token(type, value, line, column);
    advance();
    return token;
}
