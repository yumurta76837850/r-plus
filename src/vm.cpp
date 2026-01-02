#include "vm.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

/**
 * Virtual Machine Implementation
 * Handles instruction execution, memory management, and runtime state
 */

// ============================================================================
// VM Constructor and Initialization
// ============================================================================

VM::VM(size_t heap_size, size_t stack_size)
    : heap_size_(heap_size),
      stack_size_(stack_size),
      pc_(0),
      sp_(0),
      fp_(0),
      halt_flag_(false) {
    // Allocate memory regions
    heap_ = new uint8_t[heap_size_];
    stack_ = new uint8_t[stack_size_];
    
    if (!heap_ || !stack_) {
        throw std::runtime_error("Failed to allocate VM memory");
    }
    
    // Initialize memory to zero
    std::memset(heap_, 0, heap_size_);
    std::memset(stack_, 0, stack_size_);
    
    // Initialize registers
    std::memset(registers_, 0, sizeof(registers_));
    
    // Initialize call stack
    call_stack_.reserve(256);
}

// ============================================================================
// VM Destructor and Cleanup
// ============================================================================

VM::~VM() {
    if (heap_) {
        delete[] heap_;
        heap_ = nullptr;
    }
    if (stack_) {
        delete[] stack_;
        stack_ = nullptr;
    }
}

// ============================================================================
// Memory Management
// ============================================================================

/**
 * Allocates memory on the heap
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory (as uint32_t address)
 */
uint32_t VM::allocate(size_t size) {
    if (heap_alloc_ptr_ + size > heap_size_) {
        throw std::runtime_error("Heap allocation failed: out of memory");
    }
    
    uint32_t addr = static_cast<uint32_t>(heap_alloc_ptr_);
    heap_alloc_ptr_ += size;
    
    // Zero-initialize allocated memory
    std::memset(heap_ + addr, 0, size);
    
    return addr;
}

/**
 * Frees allocated heap memory
 * @param addr Address to free
 * @param size Size of memory to free
 */
void VM::deallocate(uint32_t addr, size_t size) {
    if (addr >= heap_size_ || addr + size > heap_size_) {
        throw std::runtime_error("Invalid deallocation: address out of bounds");
    }
    
    // Zero out freed memory for security
    std::memset(heap_ + addr, 0, size);
}

/**
 * Gets a value from heap memory
 * @param addr Address to read from
 * @param size Number of bytes to read
 * @return Value as 64-bit integer
 */
uint64_t VM::read_memory(uint32_t addr, size_t size) {
    if (addr + size > heap_size_) {
        throw std::runtime_error("Memory read out of bounds");
    }
    
    uint64_t value = 0;
    std::memcpy(&value, heap_ + addr, std::min(size, sizeof(uint64_t)));
    return value;
}

/**
 * Writes a value to heap memory
 * @param addr Address to write to
 * @param value Value to write
 * @param size Number of bytes to write
 */
void VM::write_memory(uint32_t addr, uint64_t value, size_t size) {
    if (addr + size > heap_size_) {
        throw std::runtime_error("Memory write out of bounds");
    }
    
    std::memcpy(heap_ + addr, &value, std::min(size, sizeof(uint64_t)));
}

// ============================================================================
// Stack Operations
// ============================================================================

/**
 * Pushes a value onto the stack
 * @param value Value to push
 * @param size Size of value in bytes
 */
void VM::push(uint64_t value, size_t size) {
    if (sp_ + size > stack_size_) {
        throw std::runtime_error("Stack overflow");
    }
    
    std::memcpy(stack_ + sp_, &value, std::min(size, sizeof(uint64_t)));
    sp_ += size;
}

/**
 * Pops a value from the stack
 * @param size Size of value in bytes
 * @return Popped value
 */
uint64_t VM::pop(size_t size) {
    if (sp_ < size) {
        throw std::runtime_error("Stack underflow");
    }
    
    sp_ -= size;
    uint64_t value = 0;
    std::memcpy(&value, stack_ + sp_, std::min(size, sizeof(uint64_t)));
    return value;
}

/**
 * Reads value from stack without popping
 * @param offset Offset from stack pointer
 * @param size Size of value in bytes
 * @return Value read from stack
 */
uint64_t VM::peek_stack(size_t offset, size_t size) {
    if (sp_ < offset + size) {
        throw std::runtime_error("Stack read out of bounds");
    }
    
    uint64_t value = 0;
    std::memcpy(&value, stack_ + sp_ - offset - size, std::min(size, sizeof(uint64_t)));
    return value;
}

// ============================================================================
// Register Operations
// ============================================================================

/**
 * Reads a register value
 * @param reg Register index (0-15)
 * @return Register value
 */
uint64_t VM::read_register(uint8_t reg) {
    if (reg >= NUM_REGISTERS) {
        throw std::runtime_error("Invalid register index");
    }
    return registers_[reg];
}

/**
 * Writes to a register
 * @param reg Register index (0-15)
 * @param value Value to write
 */
void VM::write_register(uint8_t reg, uint64_t value) {
    if (reg >= NUM_REGISTERS) {
        throw std::runtime_error("Invalid register index");
    }
    registers_[reg] = value;
}

// ============================================================================
// Instruction Execution
// ============================================================================

/**
 * Executes a single instruction
 * @param instr Instruction to execute
 */
void VM::execute_instruction(const Instruction& instr) {
    switch (instr.opcode) {
        // Arithmetic Operations
        case OpCode::ADD:
            execute_add(instr);
            break;
        case OpCode::SUB:
            execute_sub(instr);
            break;
        case OpCode::MUL:
            execute_mul(instr);
            break;
        case OpCode::DIV:
            execute_div(instr);
            break;
        case OpCode::MOD:
            execute_mod(instr);
            break;
        
        // Bitwise Operations
        case OpCode::AND:
            execute_and(instr);
            break;
        case OpCode::OR:
            execute_or(instr);
            break;
        case OpCode::XOR:
            execute_xor(instr);
            break;
        case OpCode::SHL:
            execute_shl(instr);
            break;
        case OpCode::SHR:
            execute_shr(instr);
            break;
        
        // Memory Operations
        case OpCode::LOAD:
            execute_load(instr);
            break;
        case OpCode::STORE:
            execute_store(instr);
            break;
        case OpCode::LOADIMM:
            execute_loadimm(instr);
            break;
        
        // Stack Operations
        case OpCode::PUSH:
            execute_push(instr);
            break;
        case OpCode::POP:
            execute_pop(instr);
            break;
        
        // Control Flow
        case OpCode::JMP:
            execute_jmp(instr);
            break;
        case OpCode::JZ:
            execute_jz(instr);
            break;
        case OpCode::JNZ:
            execute_jnz(instr);
            break;
        case OpCode::JLT:
            execute_jlt(instr);
            break;
        case OpCode::JLE:
            execute_jle(instr);
            break;
        case OpCode::JGT:
            execute_jgt(instr);
            break;
        case OpCode::JGE:
            execute_jge(instr);
            break;
        case OpCode::CALL:
            execute_call(instr);
            break;
        case OpCode::RET:
            execute_ret(instr);
            break;
        
        // Comparison
        case OpCode::CMP:
            execute_cmp(instr);
            break;
        
        // Other Operations
        case OpCode::NOP:
            // No operation
            break;
        case OpCode::HALT:
            halt_flag_ = true;
            break;
        
        default:
            throw std::runtime_error("Unknown opcode");
    }
    
    pc_++;
}

// ============================================================================
// Arithmetic Operation Handlers
// ============================================================================

void VM::execute_add(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a + b;
}

void VM::execute_sub(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a - b;
}

void VM::execute_mul(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a * b;
}

void VM::execute_div(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    
    if (b == 0) {
        throw std::runtime_error("Division by zero");
    }
    
    registers_[instr.dest] = a / b;
}

void VM::execute_mod(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    
    if (b == 0) {
        throw std::runtime_error("Modulo by zero");
    }
    
    registers_[instr.dest] = a % b;
}

// ============================================================================
// Bitwise Operation Handlers
// ============================================================================

void VM::execute_and(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a & b;
}

void VM::execute_or(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a | b;
}

void VM::execute_xor(const Instruction& instr) {
    uint64_t a = registers_[instr.operand1];
    uint64_t b = registers_[instr.operand2];
    registers_[instr.dest] = a ^ b;
}

void VM::execute_shl(const Instruction& instr) {
    uint64_t value = registers_[instr.operand1];
    uint64_t shift = registers_[instr.operand2];
    registers_[instr.dest] = value << shift;
}

void VM::execute_shr(const Instruction& instr) {
    uint64_t value = registers_[instr.operand1];
    uint64_t shift = registers_[instr.operand2];
    registers_[instr.dest] = value >> shift;
}

// ============================================================================
// Memory Operation Handlers
// ============================================================================

void VM::execute_load(const Instruction& instr) {
    uint32_t addr = static_cast<uint32_t>(registers_[instr.operand1]);
    uint64_t value = read_memory(addr, 8);
    registers_[instr.dest] = value;
}

void VM::execute_store(const Instruction& instr) {
    uint32_t addr = static_cast<uint32_t>(registers_[instr.operand1]);
    uint64_t value = registers_[instr.operand2];
    write_memory(addr, value, 8);
}

void VM::execute_loadimm(const Instruction& instr) {
    registers_[instr.dest] = instr.immediate;
}

// ============================================================================
// Stack Operation Handlers
// ============================================================================

void VM::execute_push(const Instruction& instr) {
    uint64_t value = registers_[instr.operand1];
    push(value, 8);
}

void VM::execute_pop(const Instruction& instr) {
    uint64_t value = pop(8);
    registers_[instr.dest] = value;
}

// ============================================================================
// Control Flow Handlers
// ============================================================================

void VM::execute_jmp(const Instruction& instr) {
    pc_ = instr.immediate - 1;  // -1 because pc_ will be incremented
}

void VM::execute_jz(const Instruction& instr) {
    if (registers_[instr.operand1] == 0) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_jnz(const Instruction& instr) {
    if (registers_[instr.operand1] != 0) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_jlt(const Instruction& instr) {
    int64_t a = static_cast<int64_t>(registers_[instr.operand1]);
    int64_t b = static_cast<int64_t>(registers_[instr.operand2]);
    if (a < b) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_jle(const Instruction& instr) {
    int64_t a = static_cast<int64_t>(registers_[instr.operand1]);
    int64_t b = static_cast<int64_t>(registers_[instr.operand2]);
    if (a <= b) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_jgt(const Instruction& instr) {
    int64_t a = static_cast<int64_t>(registers_[instr.operand1]);
    int64_t b = static_cast<int64_t>(registers_[instr.operand2]);
    if (a > b) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_jge(const Instruction& instr) {
    int64_t a = static_cast<int64_t>(registers_[instr.operand1]);
    int64_t b = static_cast<int64_t>(registers_[instr.operand2]);
    if (a >= b) {
        pc_ = instr.immediate - 1;
    }
}

void VM::execute_call(const Instruction& instr) {
    // Push return address onto call stack
    call_stack_.push_back(pc_);
    
    // Jump to function
    pc_ = instr.immediate - 1;
}

void VM::execute_ret(const Instruction& instr) {
    if (call_stack_.empty()) {
        throw std::runtime_error("Return from empty call stack");
    }
    
    pc_ = call_stack_.back();
    call_stack_.pop_back();
}

void VM::execute_cmp(const Instruction& instr) {
    int64_t a = static_cast<int64_t>(registers_[instr.operand1]);
    int64_t b = static_cast<int64_t>(registers_[instr.operand2]);
    
    // Set comparison flags in a flag register (register 15)
    if (a == b) {
        registers_[15] = 0;  // Zero flag
    } else if (a < b) {
        registers_[15] = 1;  // Less than flag
    } else {
        registers_[15] = 2;  // Greater than flag
    }
}

// ============================================================================
// VM Execution Loop
// ============================================================================

/**
 * Runs the VM with the given program
 * @param program Vector of instructions to execute
 */
void VM::run(const std::vector<Instruction>& program) {
    program_ = program;
    pc_ = 0;
    halt_flag_ = false;
    
    while (!halt_flag_ && pc_ < program_.size()) {
        try {
            execute_instruction(program_[pc_]);
        } catch (const std::exception& e) {
            std::cerr << "Runtime error at PC " << pc_ << ": " << e.what() << std::endl;
            throw;
        }
    }
}

// ============================================================================
// Debugging and State Inspection
// ============================================================================

/**
 * Dumps the current state of all registers
 */
void VM::dump_registers() const {
    std::cout << "=== Register State ===" << std::endl;
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << "R" << i << ": 0x" << std::hex << registers_[i] << std::dec << std::endl;
    }
    std::cout << "PC: " << pc_ << ", SP: " << sp_ << ", FP: " << fp_ << std::endl;
}

/**
 * Dumps a portion of heap memory
 * @param start Starting address
 * @param size Number of bytes to dump
 */
void VM::dump_heap(uint32_t start, size_t size) const {
    if (start + size > heap_size_) {
        std::cerr << "Heap dump out of bounds" << std::endl;
        return;
    }
    
    std::cout << "=== Heap Memory [" << start << "-" << (start + size - 1) << "] ===" << std::endl;
    for (size_t i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            std::cout << "0x" << std::hex << (start + i) << ": ";
        }
        std::cout << std::hex << static_cast<int>(heap_[start + i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::dec << std::endl;
        }
    }
    std::cout << std::dec << std::endl;
}

/**
 * Dumps a portion of stack memory
 * @param size Number of bytes to dump from top
 */
void VM::dump_stack(size_t size) const {
    size_t dump_size = std::min(size, static_cast<size_t>(sp_));
    
    std::cout << "=== Stack Memory (top " << dump_size << " bytes) ===" << std::endl;
    for (size_t i = 0; i < dump_size; ++i) {
        if (i % 16 == 0) {
            std::cout << "0x" << std::hex << (sp_ - dump_size + i) << ": ";
        }
        std::cout << std::hex << static_cast<int>(stack_[sp_ - dump_size + i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::dec << std::endl;
        }
    }
    std::cout << std::dec << std::endl;
}

/**
 * Gets current VM state
 * @return VMState structure with current state
 */
VMState VM::get_state() const {
    VMState state;
    state.pc = pc_;
    state.sp = sp_;
    state.fp = fp_;
    state.halted = halt_flag_;
    std::memcpy(state.registers, registers_, sizeof(registers_));
    return state;
}

/**
 * Restores VM state
 * @param state VMState structure to restore
 */
void VM::set_state(const VMState& state) {
    pc_ = state.pc;
    sp_ = state.sp;
    fp_ = state.fp;
    halt_flag_ = state.halted;
    std::memcpy(registers_, state.registers, sizeof(registers_));
}
