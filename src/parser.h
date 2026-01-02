#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>

/**
 * @class Parser
 * @brief A parser class for processing and analyzing input strings
 * 
 * This class provides functionality for tokenizing and parsing
 * input data into a structured format.
 */
class Parser {
public:
    /**
     * @brief Constructor for the Parser class
     */
    Parser();
    
    /**
     * @brief Destructor for the Parser class
     */
    virtual ~Parser();
    
    /**
     * @brief Parse the input string
     * @param input The input string to parse
     * @return true if parsing was successful, false otherwise
     */
    bool parse(const std::string& input);
    
    /**
     * @brief Get the parsing result
     * @return A vector of parsed tokens or elements
     */
    std::vector<std::string> getResult() const;
    
    /**
     * @brief Reset the parser state
     */
    void reset();
    
    /**
     * @brief Check if the parser has valid state
     * @return true if the parser is valid, false otherwise
     */
    bool isValid() const;

private:
    std::vector<std::string> tokens_;
    bool valid_;
    
    /**
     * @brief Tokenize the input string
     * @param input The input string to tokenize
     * @return A vector of tokens
     */
    std::vector<std::string> tokenize(const std::string& input);
};

#endif // PARSER_H
