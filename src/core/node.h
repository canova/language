#ifndef node_h
#define node_h

#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NIdentifier;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NIdentifier*> IdentifierList;
typedef std::vector<NVariableDeclaration*> VariableList;

enum VariableType {
    Bool,
    Integer,
    Double,
    String,
    Object,
    Void
};

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NString : public NExpression {
public:
    std::string value;
    NString(const std::string& value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReference : public NExpression {
public:
    IdentifierList refs;
    NReference() { }
    NReference(NIdentifier& id) { refs.push_back(&id); }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
    const NReference& ref;
    ExpressionList arguments;
    NMethodCall(const NReference& ref, ExpressionList& arguments) :
        ref(ref), arguments(arguments) { }
    NMethodCall(const NReference& ref) : ref(ref) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
public:
    NReference& lhs;
    NExpression& rhs;
    NAssignment(NReference& lhs, NExpression& rhs) :
        lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReturnStatement : public NStatement {
public:
    NExpression& expression;
    NReturnStatement(NExpression& expression) :
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
    const VariableType type;
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const VariableType type, NIdentifier& id) :
        type(type), id(id) { assignmentExpr = NULL; }
    NVariableDeclaration(const VariableType type, NIdentifier& id, NExpression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExternDeclaration : public NStatement {
public:
    const VariableType& type;
    const NIdentifier& id;
    VariableList arguments;
    NExternDeclaration(const VariableType& type, const NIdentifier& id, const VariableList& arguments) :
        type(type), id(id), arguments(arguments) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
    const VariableType type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const VariableType type, const NIdentifier& id,
                         const VariableList& arguments, NBlock& block) :
                         type(type), id(id), arguments(arguments), block(block)
    { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

#endif // node_h
