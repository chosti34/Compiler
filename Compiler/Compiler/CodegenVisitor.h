#pragma once
#include "../AST/AST.h"
#include "CodegenContext.h"
#include <vector>

struct GeneratedExpression
{
	llvm::Value* value;
	ExpressionTypeAST type;
};

class ExpressionCodegen : public IExpressionVisitor
{
public:
	explicit ExpressionCodegen(CodegenContext& context);
	GeneratedExpression Visit(const IExpressionAST& node);

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;
	void Visit(const FunctionCallExprAST& node) override;

private:
	CodegenContext& m_context;
	std::vector<GeneratedExpression> m_stack;
};

class StatementCodegen : public IStatementVisitor
{
public:
	explicit StatementCodegen(CodegenContext& context);
	void Visit(const IStatementAST& node);

private:
	void Visit(const VariableDeclarationAST& node) override;
	void Visit(const ReturnStatementAST& node) override;
	void Visit(const AssignStatementAST& node) override;
	void Visit(const IfStatementAST& node) override;
	void Visit(const WhileStatementAST& node) override;
	void Visit(const CompositeStatementAST& node) override;
	void Visit(const PrintAST& node) override;
	void Visit(const FunctionCallStatementAST& node) override;

private:
	CodegenContext& m_context;
	ExpressionCodegen m_expressionCodegen;
};

class Codegen
{
public:
	explicit Codegen(CodegenContext& context);
	void Generate(const ProgramAST& program);

private:
	void GenerateFunc(const FunctionAST& func);

private:
	CodegenContext& m_context;
	StatementCodegen m_statementCodegen;
};
