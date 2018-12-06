#pragma once
#include "../AST/AST.h"

#pragma warning(push, 0)
#pragma warning(disable: 4146)
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/CodeGen/Analysis.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)

#include <vector>

struct LLVMCodegen
{
	explicit LLVMCodegen();
	llvm::LLVMContext context;
	llvm::IRBuilder<> builder;
	llvm::Module module;
};

class ExpressionCodegen : public IExpressionVisitor
{
public:
	explicit ExpressionCodegen(LLVMCodegen& llvmCodegen);

	void CodegenFuncReturningExpression(const IExpressionAST& node);

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;

private:
	LLVMCodegen & m_llvmCodegen;
	std::vector<llvm::Value*> m_stack;
};

class StatementCodegen : public IStatementVisitor
{
public:
	explicit StatementCodegen();
	void Codegen(const IStatementAST& node);
	void Run(const IExpressionAST& node);

private:
	void Visit(const VariableDeclarationAST& node) override;
	void Visit(const ReturnStatementAST& node) override;
	void Visit(const AssignStatementAST& node) override;
	void Visit(const IfStatementAST& node) override;
	void Visit(const WhileStatementAST& node) override;
	void Visit(const CompositeStatementAST& node) override;

private:
	LLVMCodegen m_llvmCodegen;
	ExpressionCodegen m_expressionCodegen;
};
