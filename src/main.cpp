#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

/**
 * @file main.cpp
 * @brief Entry point for the R+ programming language compiler
 * 
 * This is the main program that orchestrates:
 * - Lexical analysis (tokenization)
 * - Syntax analysis (parsing)
 * - Code generation (compilation)
 * - Bytecode/native code output
 */

// Forward declarations
void printUsage(const char* programName);
void printVersion();
std::string readFile(const std::string& filename);
bool compileFile(const std::string& inputFile, const std::string& outputFile);
bool compileString(const std::string& source, const std::string& outputFile);

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    // Print header
    std::cout << "========================================" << std::endl;
    std::cout << "  R+ Programming Language Compiler v1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return 0;
    }
    
    if (command == "-v" || command == "--version") {
        printVersion();
        return 0;
    }
    
    if (command == "compile" || command == "-c") {
        if (argc < 3) {
            std::cerr << "Error: No input file specified" << std::endl;
            std::cerr << "Usage: " << argv[0] << " compile <input.rp> [output]" << std::endl;
            return 1;
        }
        
        std::string inputFile = argv[2];
        std::string outputFile = (argc > 3) ? argv[3] : "output.rpx";
        
        std::cout << "Compiling: " << inputFile << std::endl;
        std::cout << "Output: " << outputFile << std::endl;
        
        if (!compileFile(inputFile, outputFile)) {
            std::cerr << "Compilation failed!" << std::endl;
            return 1;
        }
        
        std::cout << "Compilation successful!" << std::endl;
        return 0;
    }
    
    if (command == "interactive" || command == "-i") {
        std::cout << "R+ Interactive Mode" << std::endl;
        std::cout << "Type 'exit' to quit, 'help' for help" << std::endl;
        std::cout << std::endl;
        
        std::string input;
        Compiler compiler;
        int lineNumber = 0;
        
        while (true) {
            std::cout << "rp> ";
            std::getline(std::cin, input);
            
            if (input == "exit" || input == "quit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            
            if (input == "help") {
                std::cout << "Available commands:" << std::endl;
                std::cout << "  exit/quit    - Exit the interpreter" << std::endl;
                std::cout << "  help         - Show this help message" << std::endl;
                std::cout << "  clear        - Clear the screen" << std::endl;
                std::cout << std::endl;
                continue;
            }
            
            if (input == "clear") {
                system("clear || cls");
                continue;
            }
            
            if (input.empty()) {
                continue;
            }
            
            // Try to compile and execute
            try {
                if (compiler.compile(input)) {
                    std::cout << "OK" << std::endl;
                } else {
                    std::cout << "Error during compilation" << std::endl;
                    auto errors = compiler.getErrors();
                    for (const auto& error : errors) {
                        std::cout << "  " << error << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "Exception: " << e.what() << std::endl;
            }
            
            lineNumber++;
        }
        
        return 0;
    }
    
    // If no recognized command, treat as input file
    std::string inputFile = command;
    std::string outputFile = (argc > 2) ? argv[2] : "output.rpx";
    
    std::cout << "Compiling: " << inputFile << std::endl;
    std::cout << "Output: " << outputFile << std::endl;
    
    if (!compileFile(inputFile, outputFile)) {
        std::cerr << "Compilation failed!" << std::endl;
        return 1;
    }
    
    std::cout << "Compilation successful!" << std::endl;
    return 0;
}

/**
 * @brief Print usage information
 */
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [command] [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  compile <file.rp> [output]  Compile R+ source file" << std::endl;
    std::cout << "  interactive                 Run interactive interpreter" << std::endl;
    std::cout << "  -v, --version               Show version information" << std::endl;
    std::cout << "  -h, --help                  Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " compile hello.rp" << std::endl;
    std::cout << "  " << programName << " hello.rp output.rpx" << std::endl;
    std::cout << "  " << programName << " interactive" << std::endl;
}

/**
 * @brief Print version information
 */
void printVersion() {
    std::cout << "R+ Programming Language Compiler" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << std::endl;
    std::cout << "Supports:" << std::endl;
    std::cout << "  - Bytecode compilation" << std::endl;
    std::cout << "  - Native code generation" << std::endl;
    std::cout << "  - Interactive interpreter" << std::endl;
    std::cout << "  - C++ style syntax" << std::endl;
}

/**
 * @brief Read file contents
 */
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief Compile source file
 */
bool compileFile(const std::string& inputFile, const std::string& outputFile) {
    try {
        // Read source file
        std::cout << "[1/5] Reading source file..." << std::endl;
        std::string source = readFile(inputFile);
        std::cout << "  OK - " << source.length() << " bytes" << std::endl;
        
        // Lexical analysis
        std::cout << "[2/5] Lexical analysis..." << std::endl;
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        std::cout << "  OK - " << tokens.size() << " tokens" << std::endl;
        
        // Syntax analysis
        std::cout << "[3/5] Syntax analysis (parsing)..." << std::endl;
        Parser parser(tokens);
        auto ast = parser.parse();
        std::cout << "  OK - AST generated" << std::endl;
        
        // Code generation
        std::cout << "[4/5] Code generation..." << std::endl;
        Compiler compiler;
        compiler.setOptimizationLevel(2);
        std::string code = compiler.generateCode(source);
        std::cout << "  OK - Code generated" << std::endl;
        
        // Write output
        std::cout << "[5/5] Writing output file..." << std::endl;
        std::ofstream outfile(outputFile, std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << "Error: Cannot open output file" << std::endl;
            return false;
        }
        outfile << code;
        outfile.close();
        std::cout << "  OK - " << outputFile << " written" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Compile from string
 */
bool compileString(const std::string& source, const std::string& outputFile) {
    try {
        // Lexical analysis
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        
        // Syntax analysis
        Parser parser(tokens);
        auto ast = parser.parse();
        
        // Code generation
        Compiler compiler;
        std::string code = compiler.generateCode(source);
        
        // Write output
        std::ofstream outfile(outputFile, std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }
        outfile << code;
        outfile.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}
