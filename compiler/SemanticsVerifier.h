#ifndef COMPILER_60MIN_SEMANTICSVERIFIER_H
#define COMPILER_60MIN_SEMANTICSVERIFIER_H

#include "AST.h"
#include "VariablesScopeChain.h"

// Вычисляет тип выражения любой сложности
class TypeEvaluator : public IExpressionVisitor
{
public:
    explicit TypeEvaluator(ScopeChain& scopes);
    ExpressionType Evaluate(const IExpressionAST& expr);

private:
    void Visit(const IdentifierAST& identifier) override;
    void Visit(const LiteralConstantAST& number) override;
    void Visit(const BinaryExpressionAST& binary) override;
    void Visit(const UnaryAST& unary) override;

private:
    std::vector<ExpressionType> m_stack;
    ScopeChain& m_scopes;
};

// Проверяет AST на семантическую корректность
class SemanticsVerifier : public IStatementVisitor
{
public:
    explicit SemanticsVerifier();
    void VerifySemantics(const IStatementAST& statement);

private:
    void Visit(const IStatementAST& stmt);

    void Visit(const VariableDeclarationAST& variableDeclaration) override;
    void Visit(const AssignStatementAST& assignment) override;
    void Visit(const ReturnStatementAST& returnStmt) override;
    void Visit(const IfStatementAST& condition) override;
    void Visit(const WhileStatementAST& whileStmt) override;
    void Visit(const CompositeStatementAST& composite) override;

private:
    std::unique_ptr<ScopeChain> m_scopes;
    std::unique_ptr<TypeEvaluator> m_evaluator;
};

#endif //COMPILER_60MIN_SEMANTICSVERIFIER_H
