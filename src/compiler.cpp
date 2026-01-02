#include "compiler.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace rplus {

// Compiler Constructor
Compiler::Compiler() 
    : current_function_(nullptr), 
      next_register_(0), 
      next_label_(0) {
}

// Compile AST to bytecode
BytecodeModule Compiler::compile(const ASTNode& root) {
    BytecodeModule module;
    current_module_ = &module;
    
    try {
        // Process main program
        visitNode(root);
        
        // Finalize module
        module.finalize();
    } catch (const std::exception& e) {
        throw std::runtime_error("Compilation error: " + std::string(e.what()));
    }
    
    return module;
}

// Generic node visitor
void Compiler::visitNode(const ASTNode& node) {
    switch (node.type()) {
        case ASTNodeType::Program:
            visitProgram(static_cast<const ProgramNode&>(node));
            break;
        case ASTNodeType::FunctionDef:
            visitFunctionDef(static_cast<const FunctionDefNode&>(node));
            break;
        case ASTNodeType::Block:
            visitBlock(static_cast<const BlockNode&>(node));
            break;
        case ASTNodeType::BinaryOp:
            visitBinaryOp(static_cast<const BinaryOpNode&>(node));
            break;
        case ASTNodeType::UnaryOp:
            visitUnaryOp(static_cast<const UnaryOpNode&>(node));
            break;
        case ASTNodeType::Literal:
            visitLiteral(static_cast<const LiteralNode&>(node));
            break;
        case ASTNodeType::Identifier:
            visitIdentifier(static_cast<const IdentifierNode&>(node));
            break;
        case ASTNodeType::Assignment:
            visitAssignment(static_cast<const AssignmentNode&>(node));
            break;
        case ASTNodeType::IfStatement:
            visitIfStatement(static_cast<const IfStatementNode&>(node));
            break;
        case ASTNodeType::WhileLoop:
            visitWhileLoop(static_cast<const WhileLoopNode&>(node));
            break;
        case ASTNodeType::ForLoop:
            visitForLoop(static_cast<const ForLoopNode&>(node));
            break;
        case ASTNodeType::FunctionCall:
            visitFunctionCall(static_cast<const FunctionCallNode&>(node));
            break;
        case ASTNodeType::ReturnStatement:
            visitReturnStatement(static_cast<const ReturnStatementNode&>(node));
            break;
        case ASTNodeType::ArrayLiteral:
            visitArrayLiteral(static_cast<const ArrayLiteralNode&>(node));
            break;
        case ASTNodeType::IndexAccess:
            visitIndexAccess(static_cast<const IndexAccessNode&>(node));
            break;
        default:
            throw std::runtime_error("Unknown AST node type");
    }
}

// Visit Program node
void Compiler::visitProgram(const ProgramNode& node) {
    for (const auto& stmt : node.statements()) {
        visitNode(*stmt);
    }
}

// Visit Function Definition
void Compiler::visitFunctionDef(const FunctionDefNode& node) {
    // Create new function
    Function func(node.name(), node.parameters().size());
    func.setParameters(node.parameters());
    
    FunctionScope scope(node.name(), node.parameters());
    pushScope(scope);
    
    // Store current function
    Function* prev_func = current_function_;
    current_function_ = &func;
    
    // Compile function body
    visitBlock(static_cast<const BlockNode&>(node.body()));
    
    // Add return instruction if not present
    if (current_bytecode_.empty() || 
        current_bytecode_.back().opcode() != OpCode::Return) {
        emit(OpCode::LoadConst, {0}); // return null
        emit(OpCode::Return);
    }
    
    // Register function in module
    func.setBytecode(current_bytecode_);
    current_module_->registerFunction(func);
    
    // Restore previous function
    current_function_ = prev_func;
    current_bytecode_.clear();
    popScope();
}

// Visit Block
void Compiler::visitBlock(const BlockNode& node) {
    for (const auto& stmt : node.statements()) {
        visitNode(*stmt);
    }
}

// Visit Binary Operation
void Compiler::visitBinaryOp(const BinaryOpNode& node) {
    // Compile left operand
    visitNode(node.left());
    uint32_t left_reg = current_register_ - 1;
    
    // Compile right operand
    visitNode(node.right());
    uint32_t right_reg = current_register_ - 1;
    
    // Generate operation bytecode
    OpCode opcode = binaryOpToOpCode(node.op());
    emit(opcode, {left_reg, right_reg});
}

// Visit Unary Operation
void Compiler::visitUnaryOp(const UnaryOpNode& node) {
    // Compile operand
    visitNode(node.operand());
    uint32_t reg = current_register_ - 1;
    
    // Generate unary operation
    OpCode opcode = unaryOpToOpCode(node.op());
    emit(opcode, {reg});
}

// Visit Literal
void Compiler::visitLiteral(const LiteralNode& node) {
    uint32_t const_index = current_module_->addConstant(node.value());
    emit(OpCode::LoadConst, {const_index});
    allocateRegister();
}

// Visit Identifier
void Compiler::visitIdentifier(const IdentifierNode& node) {
    // Check if variable exists in current scope
    uint32_t var_index = lookupVariable(node.name());
    if (var_index != UINT32_MAX) {
        emit(OpCode::LoadVar, {var_index});
        allocateRegister();
    } else {
        throw std::runtime_error("Undefined variable: " + node.name());
    }
}

// Visit Assignment
void Compiler::visitAssignment(const AssignmentNode& node) {
    // Compile right-hand side
    visitNode(node.value());
    uint32_t value_reg = current_register_ - 1;
    
    // Allocate or lookup variable
    uint32_t var_index = lookupVariable(node.name());
    if (var_index == UINT32_MAX) {
        var_index = allocateVariable(node.name());
    }
    
    // Store variable
    emit(OpCode::StoreVar, {var_index, value_reg});
}

// Visit If Statement
void Compiler::visitIfStatement(const IfStatementNode& node) {
    // Compile condition
    visitNode(node.condition());
    uint32_t cond_reg = current_register_ - 1;
    
    // Jump if false
    uint32_t false_label = genLabel();
    emit(OpCode::JumpIfFalse, {cond_reg, false_label});
    
    // Compile then branch
    visitBlock(static_cast<const BlockNode&>(node.thenBranch()));
    
    // Unconditional jump to end
    uint32_t end_label = genLabel();
    emit(OpCode::Jump, {end_label});
    
    // Mark false label
    markLabel(false_label);
    
    // Compile else branch if present
    if (node.hasElseBranch()) {
        visitBlock(static_cast<const BlockNode&>(node.elseBranch()));
    }
    
    // Mark end label
    markLabel(end_label);
}

// Visit While Loop
void Compiler::visitWhileLoop(const WhileLoopNode& node) {
    // Mark loop start
    uint32_t loop_label = genLabel();
    markLabel(loop_label);
    
    // Compile condition
    visitNode(node.condition());
    uint32_t cond_reg = current_register_ - 1;
    
    // Jump to exit on false
    uint32_t exit_label = genLabel();
    emit(OpCode::JumpIfFalse, {cond_reg, exit_label});
    
    // Compile body
    visitBlock(static_cast<const BlockNode&>(node.body()));
    
    // Jump back to condition
    emit(OpCode::Jump, {loop_label});
    
    // Mark exit label
    markLabel(exit_label);
}

// Visit For Loop
void Compiler::visitForLoop(const ForLoopNode& node) {
    FunctionScope loop_scope("for_loop", {});
    pushScope(loop_scope);
    
    // Compile initialization
    visitNode(node.init());
    
    // Mark loop start
    uint32_t loop_label = genLabel();
    markLabel(loop_label);
    
    // Compile condition
    visitNode(node.condition());
    uint32_t cond_reg = current_register_ - 1;
    
    // Jump to exit on false
    uint32_t exit_label = genLabel();
    emit(OpCode::JumpIfFalse, {cond_reg, exit_label});
    
    // Compile body
    visitBlock(static_cast<const BlockNode&>(node.body()));
    
    // Compile update
    visitNode(node.update());
    
    // Jump back to condition
    emit(OpCode::Jump, {loop_label});
    
    // Mark exit label
    markLabel(exit_label);
    
    popScope();
}

// Visit Function Call
void Compiler::visitFunctionCall(const FunctionCallNode& node) {
    // Load arguments
    std::vector<uint32_t> arg_regs;
    for (const auto& arg : node.arguments()) {
        visitNode(*arg);
        arg_regs.push_back(current_register_ - 1);
    }
    
    // Get function index
    uint32_t func_index = current_module_->lookupFunction(node.name());
    if (func_index == UINT32_MAX) {
        throw std::runtime_error("Undefined function: " + node.name());
    }
    
    // Emit call instruction
    emit(OpCode::Call, {func_index, static_cast<uint32_t>(arg_regs.size())});
    allocateRegister();
}

// Visit Return Statement
void Compiler::visitReturnStatement(const ReturnStatementNode& node) {
    if (node.hasValue()) {
        visitNode(node.value());
        uint32_t ret_reg = current_register_ - 1;
        emit(OpCode::Return, {ret_reg});
    } else {
        emit(OpCode::LoadConst, {0}); // null value
        emit(OpCode::Return);
    }
}

// Visit Array Literal
void Compiler::visitArrayLiteral(const ArrayLiteralNode& node) {
    // Load each element
    for (const auto& elem : node.elements()) {
        visitNode(*elem);
    }
    
    // Emit array creation instruction
    emit(OpCode::NewArray, {static_cast<uint32_t>(node.elements().size())});
    allocateRegister();
}

// Visit Index Access
void Compiler::visitIndexAccess(const IndexAccessNode& node) {
    // Compile array expression
    visitNode(node.array());
    uint32_t array_reg = current_register_ - 1;
    
    // Compile index expression
    visitNode(node.index());
    uint32_t index_reg = current_register_ - 1;
    
    // Emit index access instruction
    emit(OpCode::IndexLoad, {array_reg, index_reg});
}

// Helper: Convert binary operators to opcodes
OpCode Compiler::binaryOpToOpCode(const std::string& op) {
    if (op == "+") return OpCode::Add;
    if (op == "-") return OpCode::Sub;
    if (op == "*") return OpCode::Mul;
    if (op == "/") return OpCode::Div;
    if (op == "%") return OpCode::Mod;
    if (op == "==") return OpCode::Equal;
    if (op == "!=") return OpCode::NotEqual;
    if (op == "<") return OpCode::Less;
    if (op == "<=") return OpCode::LessEqual;
    if (op == ">") return OpCode::Greater;
    if (op == ">=") return OpCode::GreaterEqual;
    if (op == "&&") return OpCode::And;
    if (op == "||") return OpCode::Or;
    
    throw std::runtime_error("Unknown binary operator: " + op);
}

// Helper: Convert unary operators to opcodes
OpCode Compiler::unaryOpToOpCode(const std::string& op) {
    if (op == "-") return OpCode::Neg;
    if (op == "!") return OpCode::Not;
    
    throw std::runtime_error("Unknown unary operator: " + op);
}

// Emit bytecode instruction
void Compiler::emit(OpCode opcode, const std::vector<uint32_t>& operands) {
    Instruction instr(opcode);
    for (uint32_t operand : operands) {
        instr.addOperand(operand);
    }
    current_bytecode_.push_back(instr);
}

// Allocate a register for result
void Compiler::allocateRegister() {
    if (current_register_ < MAX_REGISTERS) {
        current_register_++;
    } else {
        throw std::runtime_error("Register overflow: too many temporary values");
    }
}

// Generate unique label
uint32_t Compiler::genLabel() {
    return next_label_++;
}

// Mark label position in bytecode
void Compiler::markLabel(uint32_t label) {
    label_positions_[label] = current_bytecode_.size();
}

// Look up variable in current scope
uint32_t Compiler::lookupVariable(const std::string& name) {
    if (!scope_stack_.empty()) {
        return scope_stack_.back().lookupVariable(name);
    }
    return UINT32_MAX;
}

// Allocate new variable in current scope
uint32_t Compiler::allocateVariable(const std::string& name) {
    if (!scope_stack_.empty()) {
        return scope_stack_.back().allocateVariable(name);
    }
    throw std::runtime_error("No active scope for variable allocation");
}

// Push new scope
void Compiler::pushScope(const FunctionScope& scope) {
    scope_stack_.push_back(scope);
}

// Pop current scope
void Compiler::popScope() {
    if (!scope_stack_.empty()) {
        scope_stack_.pop_back();
    }
}

// Generate native code from bytecode
NativeCodeModule Compiler::generateNativeCode(const BytecodeModule& bytecode) {
    NativeCodeModule native_module;
    
    try {
        // Process each function in bytecode
        for (const auto& func : bytecode.functions()) {
            std::string native_code = compileToNative(func);
            native_module.addFunction(func.name(), native_code);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Native code generation error: " + std::string(e.what()));
    }
    
    return native_module;
}

// Compile bytecode function to native code
std::string Compiler::compileToNative(const Function& func) {
    std::stringstream ss;
    
    // Function prologue
    ss << "function " << func.name() << "() {\n";
    
    // Allocate local variables
    ss << "  var locals[" << func.parameterCount() << "] = {};\n";
    
    // Compile bytecode to native instructions
    for (size_t i = 0; i < func.bytecode().size(); ++i) {
        const Instruction& instr = func.bytecode()[i];
        ss << "  // " << i << ": " << opcodeToString(instr.opcode()) << "\n";
        
        switch (instr.opcode()) {
            case OpCode::LoadConst:
                ss << "  r0 = constants[" << instr.operand(0) << "];\n";
                break;
            case OpCode::LoadVar:
                ss << "  r0 = locals[" << instr.operand(0) << "];\n";
                break;
            case OpCode::StoreVar:
                ss << "  locals[" << instr.operand(0) << "] = r" 
                   << instr.operand(1) << ";\n";
                break;
            case OpCode::Add:
                ss << "  r0 = r" << instr.operand(0) << " + r" 
                   << instr.operand(1) << ";\n";
                break;
            case OpCode::Sub:
                ss << "  r0 = r" << instr.operand(0) << " - r" 
                   << instr.operand(1) << ";\n";
                break;
            case OpCode::Mul:
                ss << "  r0 = r" << instr.operand(0) << " * r" 
                   << instr.operand(1) << ";\n";
                break;
            case OpCode::Div:
                ss << "  r0 = r" << instr.operand(0) << " / r" 
                   << instr.operand(1) << ";\n";
                break;
            case OpCode::Call:
                ss << "  r0 = call_function(" << instr.operand(0) 
                   << ", " << instr.operand(1) << ");\n";
                break;
            case OpCode::Return:
                ss << "  return r0;\n";
                break;
            case OpCode::Jump:
                ss << "  goto label_" << instr.operand(0) << ";\n";
                break;
            case OpCode::JumpIfFalse:
                ss << "  if (!r" << instr.operand(0) << ") goto label_" 
                   << instr.operand(1) << ";\n";
                break;
            default:
                ss << "  // Unsupported opcode\n";
                break;
        }
    }
    
    ss << "}\n";
    return ss.str();
}

// Helper: Convert opcode to string
std::string Compiler::opcodeToString(OpCode opcode) {
    switch (opcode) {
        case OpCode::LoadConst: return "LoadConst";
        case OpCode::LoadVar: return "LoadVar";
        case OpCode::StoreVar: return "StoreVar";
        case OpCode::Add: return "Add";
        case OpCode::Sub: return "Sub";
        case OpCode::Mul: return "Mul";
        case OpCode::Div: return "Div";
        case OpCode::Mod: return "Mod";
        case OpCode::Equal: return "Equal";
        case OpCode::NotEqual: return "NotEqual";
        case OpCode::Less: return "Less";
        case OpCode::LessEqual: return "LessEqual";
        case OpCode::Greater: return "Greater";
        case OpCode::GreaterEqual: return "GreaterEqual";
        case OpCode::And: return "And";
        case OpCode::Or: return "Or";
        case OpCode::Neg: return "Neg";
        case OpCode::Not: return "Not";
        case OpCode::Jump: return "Jump";
        case OpCode::JumpIfFalse: return "JumpIfFalse";
        case OpCode::Call: return "Call";
        case OpCode::Return: return "Return";
        case OpCode::NewArray: return "NewArray";
        case OpCode::IndexLoad: return "IndexLoad";
        case OpCode::IndexStore: return "IndexStore";
        default: return "Unknown";
    }
}

// Optimize bytecode
void Compiler::optimizeBytecode(BytecodeModule& module) {
    // Perform constant folding
    performConstantFolding(module);
    
    // Remove dead code
    removeDeadCode(module);
    
    // Inline simple functions
    inlineSimpleFunctions(module);
}

// Perform constant folding optimization
void Compiler::performConstantFolding(BytecodeModule& module) {
    // Implementation for constant folding
    // This would analyze bytecode and combine constant operations
}

// Remove unreachable code
void Compiler::removeDeadCode(BytecodeModule& module) {
    // Implementation for dead code elimination
    // This would remove instructions that are never reached
}

// Inline simple function calls
void Compiler::inlineSimpleFunctions(BytecodeModule& module) {
    // Implementation for function inlining
    // This would replace calls to small functions with inline code
}

} // namespace rplus
