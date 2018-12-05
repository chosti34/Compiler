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

class LLVMCodeGenerator : public IExpressionVisitor
{
public:
	explicit LLVMCodeGenerator();

	void CodegenFuncReturningExpression(const IExpressionAST& ast);

	llvm::Module& GetLLVMModule();
	const llvm::Module& GetLLVMModule()const;

private:
	void Visit(const BinaryExpressionAST& node) override;
	void Visit(const LiteralConstantAST& node) override;
	void Visit(const UnaryAST& node) override;
	void Visit(const IdentifierAST& node) override;

private:
	std::vector<llvm::Value*> m_stack;
	llvm::LLVMContext m_context;
	llvm::IRBuilder<> m_builder;
	llvm::Module m_module;
};
