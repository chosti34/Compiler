#pragma once
#include "../AST/AST.h"
#include "ScopeChain.h"

using TypeScopeChain = ScopeChain<ASTExpressionType>;

// Вычисляет тип AST выражения
class TypeEvaluator : public IExpressionVisitor
{
public:
	explicit TypeEvaluator(TypeScopeChain& scopes);
	ASTExpressionType Evaluate(const IExpressionAST& expr);

private:
	void Visit(const IdentifierAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const UnaryAST& node) override;

private:
	std::vector<ASTExpressionType> m_stack;
	TypeScopeChain& m_scopes;
};

// Проверяет AST на семантическую корректность
class SemanticsVerifier : public IStatementVisitor
{
public:
	explicit SemanticsVerifier();
	void VerifySemantics(const IStatementAST& node);

private:
	void Visit(const VariableDeclarationAST& node) override;
	void Visit(const AssignStatementAST& node) override;
	void Visit(const ReturnStatementAST& node) override;
	void Visit(const IfStatementAST& node) override;
	void Visit(const WhileStatementAST& node) override;
	void Visit(const CompositeStatementAST& node) override;
	void Visit(const PrintAST& node) override;

private:
	std::unique_ptr<TypeScopeChain> m_scopes;
	std::unique_ptr<TypeEvaluator> m_evaluator;
};
