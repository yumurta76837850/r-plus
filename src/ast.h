#ifndef RPLUS_AST_H
#define RPLUS_AST_H

#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace rplus {

// Forward declarations
struct ASTNode;
struct Expression;
struct Statement;
struct Literal;
struct Identifier;
struct BinaryOp;
struct UnaryOp;
struct FunctionCall;
struct ArrayAccess;
struct PropertyAccess;
struct Assignment;
struct IfStatement;
struct WhileStatement;
struct ForStatement;
struct FunctionDeclaration;
struct ClassDeclaration;
struct ReturnStatement;
struct BlockStatement;
struct ExpressionStatement;
struct VariableDeclaration;
struct ArrayLiteral;
struct ObjectLiteral;
struct LambdaExpression;
struct ConditionalExpression;
struct ThisExpression;
struct NewExpression;
struct BreakStatement;
struct ContinueStatement;
struct SwitchStatement;
struct TryStatement;
struct ThrowStatement;

// Base AST Node
struct ASTNode {
    virtual ~ASTNode() = default;
    int line;
    int column;
};

// Literals
enum class LiteralType {
    NUMBER,
    STRING,
    BOOLEAN,
    NULL_TYPE,
    UNDEFINED
};

struct Literal : public ASTNode {
    LiteralType type;
    std::string value;
};

// Identifier
struct Identifier : public ASTNode {
    std::string name;
};

// Binary Operations
enum class BinaryOperator {
    // Arithmetic
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    POWER,
    
    // Comparison
    EQUAL,
    NOT_EQUAL,
    STRICT_EQUAL,
    STRICT_NOT_EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    
    // Logical
    AND,
    OR,
    
    // Bitwise
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    UNSIGNED_RIGHT_SHIFT,
    
    // Assignment variants
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    MULTIPLY_ASSIGN,
    DIVIDE_ASSIGN,
    MODULO_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    LEFT_SHIFT_ASSIGN,
    RIGHT_SHIFT_ASSIGN,
    
    // Other
    IN,
    INSTANCEOF,
    DOT,
    COMMA
};

struct BinaryOp : public ASTNode {
    BinaryOperator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

// Unary Operations
enum class UnaryOperator {
    PLUS,
    MINUS,
    NOT,
    BITWISE_NOT,
    TYPEOF,
    VOID,
    DELETE,
    INCREMENT,
    DECREMENT,
    POSTFIX_INCREMENT,
    POSTFIX_DECREMENT
};

struct UnaryOp : public ASTNode {
    UnaryOperator op;
    std::unique_ptr<Expression> operand;
    bool isPrefix;
};

// Function Call
struct FunctionCall : public ASTNode {
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// Array Access
struct ArrayAccess : public ASTNode {
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;
};

// Property Access
struct PropertyAccess : public ASTNode {
    std::unique_ptr<Expression> object;
    std::string property;
    bool computed;  // true for obj[prop], false for obj.prop
};

// Assignment Expression
struct Assignment : public ASTNode {
    std::unique_ptr<Expression> target;
    std::unique_ptr<Expression> value;
    BinaryOperator assignmentType;  // For compound assignments
};

// Conditional Expression (ternary)
struct ConditionalExpression : public ASTNode {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> consequent;
    std::unique_ptr<Expression> alternate;
};

// Array Literal
struct ArrayLiteral : public ASTNode {
    std::vector<std::unique_ptr<Expression>> elements;
};

// Object Property
struct ObjectProperty {
    std::string key;
    std::unique_ptr<Expression> value;
    bool computed;  // true for [key]: value, false for key: value
    bool shorthand;  // true for { x } instead of { x: x }
};

// Object Literal
struct ObjectLiteral : public ASTNode {
    std::vector<ObjectProperty> properties;
};

// Lambda/Arrow Function
struct LambdaExpression : public ASTNode {
    std::vector<std::string> parameters;
    std::unique_ptr<ASTNode> body;  // Can be expression or block
    bool isAsync;
};

// This Expression
struct ThisExpression : public ASTNode {
};

// New Expression
struct NewExpression : public ASTNode {
    std::unique_ptr<Expression> constructor;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// Base Expression type
struct Expression : public ASTNode {
    virtual ~Expression() = default;
};

// Concrete expression implementations
struct LiteralExpr : public Expression {
    std::unique_ptr<Literal> literal;
};

struct IdentifierExpr : public Expression {
    std::unique_ptr<Identifier> identifier;
};

struct BinaryOpExpr : public Expression {
    std::unique_ptr<BinaryOp> binaryOp;
};

struct UnaryOpExpr : public Expression {
    std::unique_ptr<UnaryOp> unaryOp;
};

struct FunctionCallExpr : public Expression {
    std::unique_ptr<FunctionCall> call;
};

struct ArrayAccessExpr : public Expression {
    std::unique_ptr<ArrayAccess> access;
};

struct PropertyAccessExpr : public Expression {
    std::unique_ptr<PropertyAccess> access;
};

struct AssignmentExpr : public Expression {
    std::unique_ptr<Assignment> assignment;
};

struct ConditionalExpr : public Expression {
    std::unique_ptr<ConditionalExpression> conditional;
};

struct ArrayLiteralExpr : public Expression {
    std::unique_ptr<ArrayLiteral> array;
};

struct ObjectLiteralExpr : public Expression {
    std::unique_ptr<ObjectLiteral> object;
};

struct LambdaExpr : public Expression {
    std::unique_ptr<LambdaExpression> lambda;
};

struct ThisExpr : public Expression {
    std::unique_ptr<ThisExpression> thisExpr;
};

struct NewExpr : public Expression {
    std::unique_ptr<NewExpression> newExpr;
};

// Statements
struct Statement : public ASTNode {
    virtual ~Statement() = default;
};

// Block Statement
struct BlockStatement : public Statement {
    std::vector<std::unique_ptr<ASTNode>> statements;
};

// Expression Statement
struct ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expression;
};

// Variable Declaration
enum class VariableKind {
    VAR,
    LET,
    CONST
};

struct VariableDeclarator {
    std::string id;
    std::unique_ptr<Expression> init;  // Can be nullptr
};

struct VariableDeclaration : public Statement {
    VariableKind kind;
    std::vector<VariableDeclarator> declarations;
};

// If Statement
struct IfStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> consequent;
    std::unique_ptr<Statement> alternate;  // Can be nullptr for no else
};

// While Statement
struct WhileStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
};

// Do-While Statement
struct DoWhileStatement : public Statement {
    std::unique_ptr<Statement> body;
    std::unique_ptr<Expression> condition;
};

// For Statement
struct ForStatement : public Statement {
    std::unique_ptr<ASTNode> init;  // VariableDeclaration or Expression
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> update;
    std::unique_ptr<Statement> body;
};

// For-In Statement
struct ForInStatement : public Statement {
    std::unique_ptr<ASTNode> left;  // VariableDeclaration or Identifier
    std::unique_ptr<Expression> right;
    std::unique_ptr<Statement> body;
};

// For-Of Statement
struct ForOfStatement : public Statement {
    std::unique_ptr<ASTNode> left;  // VariableDeclaration or Identifier
    std::unique_ptr<Expression> right;
    std::unique_ptr<Statement> body;
    bool isAwait;
};

// Switch Case
struct SwitchCase {
    std::unique_ptr<Expression> test;  // nullptr for default case
    std::vector<std::unique_ptr<Statement>> consequent;
};

// Switch Statement
struct SwitchStatement : public Statement {
    std::unique_ptr<Expression> discriminant;
    std::vector<SwitchCase> cases;
};

// Break Statement
struct BreakStatement : public Statement {
    std::string label;  // Optional label
};

// Continue Statement
struct ContinueStatement : public Statement {
    std::string label;  // Optional label
};

// Return Statement
struct ReturnStatement : public Statement {
    std::unique_ptr<Expression> argument;  // Can be nullptr
};

// Throw Statement
struct ThrowStatement : public Statement {
    std::unique_ptr<Expression> argument;
};

// Catch Clause
struct CatchClause {
    std::string param;  // Can be empty string for no binding
    std::unique_ptr<BlockStatement> body;
};

// Try Statement
struct TryStatement : public Statement {
    std::unique_ptr<BlockStatement> block;
    std::unique_ptr<CatchClause> handler;  // Can be nullptr
    std::unique_ptr<BlockStatement> finalizer;  // Can be nullptr
};

// Function Parameter
struct FunctionParameter {
    std::string name;
    std::unique_ptr<Expression> defaultValue;  // Can be nullptr
    bool isRest;  // true for ...rest parameter
};

// Function Declaration
struct FunctionDeclaration : public Statement {
    std::string id;
    std::vector<FunctionParameter> parameters;
    std::unique_ptr<BlockStatement> body;
    bool isAsync;
    bool isGenerator;
};

// Class Method
struct ClassMethod {
    std::string key;
    std::vector<FunctionParameter> parameters;
    std::unique_ptr<BlockStatement> body;
    enum class Kind { CONSTRUCTOR, METHOD, GET, SET } kind;
    bool isStatic;
    bool isAsync;
};

// Class Property
struct ClassProperty {
    std::string key;
    std::unique_ptr<Expression> value;  // Can be nullptr
    bool isStatic;
    bool isReadonly;
};

// Class Declaration
struct ClassDeclaration : public Statement {
    std::string id;
    std::unique_ptr<Expression> superClass;  // Can be nullptr
    std::vector<ClassProperty> properties;
    std::vector<ClassMethod> methods;
};

// Empty Statement
struct EmptyStatement : public Statement {
};

// Labeled Statement
struct LabeledStatement : public Statement {
    std::string label;
    std::unique_ptr<Statement> statement;
};

// Debugger Statement
struct DebuggerStatement : public Statement {
};

// Program (Root Node)
struct Program : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> body;
};

}  // namespace rplus

#endif  // RPLUS_AST_H
