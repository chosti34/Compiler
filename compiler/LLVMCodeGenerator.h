#ifndef COMPILER_60MIN_LLVMCODEGENVISITOR_H
#define COMPILER_60MIN_LLVMCODEGENVISITOR_H

#include "ASTVisitor.h"
#include "AST.h"

#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/CodeGen/Analysis.h>

#include <vector>

class LLVMCodeGenerator : public IExpressionVisitor
{
public:
    LLVMCodeGenerator();

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

#endif //COMPILER_60MIN_LLVMCODEGENVISITOR_H
