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

struct CodegenUtils
{
	explicit CodegenUtils();
	llvm::LLVMContext context;
	llvm::IRBuilder<> builder;
	llvm::Module module;
};

class ExpressionCodegen : public IExpressionVisitor
{
public:
	explicit ExpressionCodegen(CodegenUtils& utils);

	void CodegenFuncReturningExpression(const IExpressionAST& node);

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;

private:
	CodegenUtils& m_utils;
	std::vector<llvm::Value*> m_stack;
};

class StatementCodegen : public IStatementVisitor
{
public:
	explicit StatementCodegen();
	void Generate(const IStatementAST& node, std::ostream& out);

private:
	void Visit(const VariableDeclarationAST& node) override;
	void Visit(const ReturnStatementAST& node) override;
	void Visit(const AssignStatementAST& node) override;
	void Visit(const IfStatementAST& node) override;
	void Visit(const WhileStatementAST& node) override;
	void Visit(const CompositeStatementAST& node) override;

private:
	CodegenUtils m_utils;
	ExpressionCodegen m_expressionCodegen;
};
