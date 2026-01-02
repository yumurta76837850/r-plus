#ifndef VM_H
#define VM_H

#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <array>

namespace rplus {

// Forward declarations
class Value;
class Chunk;

// Bytecode instruction opcodes
enum class OpCode : uint8_t {
    // Constants and variables
    OP_CONSTANT = 0,
    OP_DEFINE_GLOBAL = 1,
    OP_GET_GLOBAL = 2,
    OP_SET_GLOBAL = 3,
    OP_GET_LOCAL = 4,
    OP_SET_LOCAL = 5,
    
    // Arithmetic operations
    OP_ADD = 10,
    OP_SUBTRACT = 11,
    OP_MULTIPLY = 12,
    OP_DIVIDE = 13,
    OP_MODULO = 14,
    OP_NEGATE = 15,
    
    // Comparison operations
    OP_EQUAL = 20,
    OP_NOT_EQUAL = 21,
    OP_LESS = 22,
    OP_LESS_EQUAL = 23,
    OP_GREATER = 24,
    OP_GREATER_EQUAL = 25,
    
    // Logical operations
    OP_AND = 30,
    OP_OR = 31,
    OP_NOT = 32,
    
    // Control flow
    OP_JUMP = 40,
    OP_JUMP_IF_FALSE = 41,
    OP_JUMP_IF_TRUE = 42,
    OP_LOOP = 43,
    
    // Function calls
    OP_CALL = 50,
    OP_RETURN = 51,
    
    // Stack operations
    OP_POP = 60,
    OP_DUP = 61,
    
    // End of execution
    OP_EXIT = 255
};

// Runtime value representation
class Value {
public:
    enum class Type : uint8_t {
        NIL,
        BOOL,
        NUMBER,
        STRING
    };
    
    Value();
    explicit Value(bool b);
    explicit Value(double n);
    explicit Value(const std::string& s);
    
    Type type() const { return type_; }
    
    bool as_bool() const;
    double as_number() const;
    const std::string& as_string() const;
    
    bool is_nil() const { return type_ == Type::NIL; }
    bool is_bool() const { return type_ == Type::BOOL; }
    bool is_number() const { return type_ == Type::NUMBER; }
    bool is_string() const { return type_ == Type::STRING; }
    
    std::string to_string() const;
    bool is_truthy() const;
    bool equals(const Value& other) const;
    
private:
    Type type_;
    bool bool_value_;
    double number_value_;
    std::string string_value_;
};

// Bytecode chunk containing instructions and constants
class Chunk {
public:
    Chunk() = default;
    ~Chunk() = default;
    
    void write_byte(uint8_t byte, int line);
    void write_constant(const Value& value);
    
    const std::vector<uint8_t>& code() const { return code_; }
    const std::vector<Value>& constants() const { return constants_; }
    const std::vector<int>& lines() const { return lines_; }
    
    uint8_t get_byte(size_t offset) const;
    const Value& get_constant(size_t index) const;
    int get_line(size_t offset) const;
    
    size_t size() const { return code_.size(); }
    void clear();
    
private:
    std::vector<uint8_t> code_;
    std::vector<Value> constants_;
    std::vector<int> lines_;
};

// Virtual Machine for bytecode execution
class VirtualMachine {
public:
    static constexpr size_t STACK_MAX = 256;
    
    VirtualMachine();
    ~VirtualMachine();
    
    // Execute bytecode
    void execute(const Chunk& chunk);
    
    // Stack operations
    void push(const Value& value);
    Value pop();
    const Value& peek(size_t distance = 0) const;
    void clear_stack();
    
    // VM state queries
    bool is_running() const { return running_; }
    size_t stack_size() const { return stack_top_; }
    
    // Error handling
    void set_error(const std::string& message);
    const std::string& get_error() const { return error_message_; }
    bool has_error() const { return !error_message_.empty(); }
    
    // Debug utilities
    void enable_trace(bool enable) { trace_enabled_ = enable; }
    bool is_trace_enabled() const { return trace_enabled_; }
    
private:
    // Stack management
    std::array<Value, STACK_MAX> stack_;
    size_t stack_top_;
    
    // Execution state
    bool running_;
    std::string error_message_;
    
    // Bytecode execution
    const Chunk* current_chunk_;
    size_t instruction_pointer_;
    
    // Debug
    bool trace_enabled_;
    
    // Instruction handlers
    void execute_instruction(OpCode op);
    void handle_constant(uint8_t index);
    void handle_add();
    void handle_subtract();
    void handle_multiply();
    void handle_divide();
    void handle_modulo();
    void handle_negate();
    void handle_equal();
    void handle_not_equal();
    void handle_less();
    void handle_less_equal();
    void handle_greater();
    void handle_greater_equal();
    void handle_and();
    void handle_or();
    void handle_not();
    void handle_jump(uint16_t offset);
    void handle_jump_if_false(uint16_t offset);
    void handle_jump_if_true(uint16_t offset);
    void handle_loop(uint16_t offset);
    
    // Utility methods
    uint16_t read_short();
    uint8_t read_byte();
    void trace_instruction();
    
    // Arithmetic helpers
    void binary_op(OpCode op);
    void comparison_op(OpCode op);
};

// Exception class for VM runtime errors
class VMException : public std::runtime_error {
public:
    explicit VMException(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace rplus

#endif // VM_H
