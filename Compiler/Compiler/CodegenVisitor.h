#pragma once
#include "../AST/AST.h"
#include "CodegenContext.h"
#include <vector>

class ExpressionCodegen : public IExpressionVisitor
{
public:
	explicit ExpressionCodegen(CodegenContext& context);

	void CodegenFuncReturningExpression(const IExpressionAST& node);
	llvm::Value* Visit(const IExpressionAST& node);

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;

private:
	CodegenContext& m_context;
	std::vector<llvm::Value*> m_stack;
};

class StatementCodegen : public IStatementVisitor
{
public:
	explicit StatementCodegen(CodegenContext& context);
	void GenerateMainFn(const IStatementAST& node);

private:
	void Visit(const VariableDeclarationAST& node) override;
	void Visit(const ReturnStatementAST& node) override;
	void Visit(const AssignStatementAST& node) override;
	void Visit(const IfStatementAST& node) override;
	void Visit(const WhileStatementAST& node) override;
	void Visit(const CompositeStatementAST& node) override;
	void Visit(const PrintAST& node) override;

private:
	CodegenContext& m_context;
	ExpressionCodegen m_expressionCodegen;
};
