#pragma once
#include "../AST/AST.h"
#include "CodegenContext.h"
#include <vector>

class ExpressionCodegen : public IExpressionVisitor
{
public:
	explicit ExpressionCodegen(CodegenContext& context);
	llvm::Value* Visit(const IExpressionAST& node);

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;
	void Visit(const FunctionCallExprAST& node) override;

private:
	CodegenContext& m_context;
	std::vector<llvm::Value*> m_stack;
};

class StatementCodegen : public IStatementVisitor
{
public:
	explicit StatementCodegen(CodegenContext& context,
		std::vector<llvm::BasicBlock*> & continueBlocks);
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
	std::vector<llvm::BasicBlock*> & m_continueBlocks;
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
};
