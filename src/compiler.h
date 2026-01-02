#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

/**
 * @brief Compiler class for code generation
 * 
 * This class handles the compilation and code generation process,
 * transforming abstract syntax trees or intermediate representations
 * into executable code or other target formats.
 */
class Compiler {
public:
    /**
     * @brief Constructor for Compiler
     */
    Compiler();

    /**
     * @brief Destructor for Compiler
     */
    ~Compiler();

    /**
     * @brief Compile source code
     * @param source The source code string to compile
     * @return true if compilation was successful, false otherwise
     */
    bool compile(const std::string& source);

    /**
     * @brief Generate code from AST
     * @param ast Abstract syntax tree representation
     * @return Generated code as a string
     */
    std::string generateCode(const std::string& ast);

    /**
     * @brief Optimize the generated code
     * @param code The code to optimize
     * @return Optimized code
     */
    std::string optimize(const std::string& code);

    /**
     * @brief Get compilation errors
     * @return Vector of error messages
     */
    std::vector<std::string> getErrors() const;

    /**
     * @brief Get compilation warnings
     * @return Vector of warning messages
     */
    std::vector<std::string> getWarnings() const;

    /**
     * @brief Clear all errors and warnings
     */
    void clearDiagnostics();

    /**
     * @brief Set compiler optimization level (0-3)
     * @param level Optimization level (0=none, 3=maximum)
     */
    void setOptimizationLevel(int level);

    /**
     * @brief Get current optimization level
     * @return Current optimization level
     */
    int getOptimizationLevel() const;

private:
    // Symbol table for storing variable and function definitions
    std::unordered_map<std::string, std::string> symbolTable;

    // Storage for compilation errors
    std::vector<std::string> errors;

    // Storage for compilation warnings
    std::vector<std::string> warnings;

    // Optimization level (0-3)
    int optimizationLevel;

    /**
     * @brief Lexical analysis phase
     * @param source Source code to tokenize
     * @return Vector of tokens
     */
    std::vector<std::string> tokenize(const std::string& source);

    /**
     * @brief Syntax analysis phase
     * @param tokens Vector of tokens
     * @return Abstract syntax tree
     */
    std::string parseTokens(const std::vector<std::string>& tokens);

    /**
     * @brief Semantic analysis phase
     * @param ast Abstract syntax tree
     * @return true if semantic analysis passed, false otherwise
     */
    bool semanticAnalysis(const std::string& ast);

    /**
     * @brief Code generation phase
     * @param ast Abstract syntax tree
     * @return Generated code
     */
    std::string codeGen(const std::string& ast);

    /**
     * @brief Add error message
     * @param message Error message
     */
    void addError(const std::string& message);

    /**
     * @brief Add warning message
     * @param message Warning message
     */
    void addWarning(const std::string& message);

    /**
     * @brief Validate the source code
     * @param source Source code to validate
     * @return true if valid, false otherwise
     */
    bool validate(const std::string& source);
};

#endif // COMPILER_H
