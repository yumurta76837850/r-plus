#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @enum TokenType
 * @brief Enumeration of all token types recognized by the lexer
 */
enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    IDENTIFIER,
    
    // Keywords
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_RETURN,
    KEYWORD_FUNCTION,
    KEYWORD_VAR,
    KEYWORD_CONST,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_NOT,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    BITWISE_NOT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    INCREMENT,
    DECREMENT,
    
    // Delimiters
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    ARROW,
    
    // Special
    END_OF_FILE,
    UNKNOWN,
    WHITESPACE
};

/**
 * @struct Token
 * @brief Represents a single token with its type, value, line, and column information
 */
struct Token {
    TokenType type;      ///< The type of the token
    std::string value;   ///< The lexeme/value of the token
    int line;            ///< Line number where the token appears
    int column;          ///< Column number where the token starts
    
    /**
     * @brief Constructor for Token
     * @param t Token type
     * @param v Token value/lexeme
     * @param l Line number
     * @param c Column number
     */
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
    
    /**
     * @brief Default constructor
     */
    Token() : type(TokenType::UNKNOWN), value(""), line(0), column(0) {}
};

/**
 * @class Lexer
 * @brief Lexical analyzer for tokenizing source code
 * 
 * The Lexer class is responsible for breaking down source code into tokens.
 * It handles keywords, operators, identifiers, literals, and maintains position
 * information for error reporting.
 */
class Lexer {
private:
    std::string source;                                    ///< The source code to tokenize
    size_t current;                                        ///< Current position in source
    int line;                                              ///< Current line number
    int column;                                            ///< Current column number
    std::unordered_map<std::string, TokenType> keywords;   ///< Map of keywords to token types
    
    /**
     * @brief Initialize the keywords map
     */
    void initializeKeywords();
    
    /**
     * @brief Check if we're at the end of the source
     * @return true if at end of file, false otherwise
     */
    bool isAtEnd() const;
    
    /**
     * @brief Peek at the current character without consuming it
     * @return Current character, or null character if at end
     */
    char peek() const;
    
    /**
     * @brief Peek at the next character without consuming it
     * @return Next character, or null character if at end
     */
    char peekNext() const;
    
    /**
     * @brief Consume and return the current character
     * @return Current character
     */
    char advance();
    
    /**
     * @brief Check if a character matches the expected character
     * @param expected The expected character
     * @return true if current character matches, false otherwise
     */
    bool match(char expected);
    
    /**
     * @brief Check if a character is a digit
     * @param c Character to check
     * @return true if character is a digit (0-9)
     */
    bool isDigit(char c) const;
    
    /**
     * @brief Check if a character is alphabetic or underscore
     * @param c Character to check
     * @return true if character is a letter or underscore
     */
    bool isAlpha(char c) const;
    
    /**
     * @brief Check if a character is alphanumeric or underscore
     * @param c Character to check
     * @return true if character is alphanumeric or underscore
     */
    bool isAlphaNumeric(char c) const;
    
    /**
     * @brief Check if a character is whitespace
     * @param c Character to check
     * @return true if character is whitespace
     */
    bool isWhitespace(char c) const;
    
    /**
     * @brief Scan a number literal
     * @return Token representing the number
     */
    Token scanNumber();
    
    /**
     * @brief Scan a string literal
     * @param quote The quote character (single or double)
     * @return Token representing the string
     */
    Token scanString(char quote);
    
    /**
     * @brief Scan an identifier or keyword
     * @return Token representing the identifier or keyword
     */
    Token scanIdentifier();
    
    /**
     * @brief Scan whitespace and return appropriate token
     * @return Token representing whitespace or consumed whitespace
     */
    Token scanWhitespace();
    
    /**
     * @brief Scan a single character token or operator
     * @return Token for the character or operator sequence
     */
    Token scanOperatorOrDelimiter();

public:
    /**
     * @brief Constructor for Lexer
     * @param src The source code to tokenize
     */
    explicit Lexer(const std::string& src);
    
    /**
     * @brief Destructor for Lexer
     */
    ~Lexer() = default;
    
    /**
     * @brief Tokenize the entire source code
     * @return Vector of tokens
     */
    std::vector<Token> tokenize();
    
    /**
     * @brief Get the next token from the source
     * @return The next token
     */
    Token nextToken();
    
    /**
     * @brief Get current line number
     * @return Current line number
     */
    int getCurrentLine() const { return line; }
    
    /**
     * @brief Get current column number
     * @return Current column number
     */
    int getCurrentColumn() const { return column; }
};

#endif // LEXER_H
