#ifndef COMPILER_60MIN_ASTVISITOR_H
#define COMPILER_60MIN_ASTVISITOR_H

class NumberConstantAST;
class IdentifierAST;
class BinaryExpressionAST;
class UnaryAST;

class IExpressionVisitor
{
public:
    virtual ~IExpressionVisitor() = default;
    virtual void Visit(const BinaryExpressionAST& node) = 0;
    virtual void Visit(const NumberConstantAST& node) = 0;
    virtual void Visit(const UnaryAST& node) = 0;
    virtual void Visit(const IdentifierAST& node) = 0;
};

class VariableDeclarationAST;
class AssignStatementAST;
class ReturnStatementAST;
class IfStatementAST;
class WhileStatementAST;
class CompositeStatementAST;

class IStatementVisitor
{
public:
    virtual ~IStatementVisitor() = default;
    virtual void Visit(const VariableDeclarationAST& vardecl) = 0;
    virtual void Visit(const AssignStatementAST& assign) = 0;
    virtual void Visit(const ReturnStatementAST& ret) = 0;
    virtual void Visit(const IfStatementAST& ifstmt) = 0;
    virtual void Visit(const WhileStatementAST& loop) = 0;
    virtual void Visit(const CompositeStatementAST& composite) = 0;
};

#endif //COMPILER_60MIN_ASTVISITOR_H
