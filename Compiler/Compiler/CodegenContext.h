#pragma once
#include "ScopeChain.h"
#include <ostream>

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

class CodegenUtils
{
public:
	CodegenUtils();

	llvm::LLVMContext& GetLLVMContext();
	llvm::IRBuilder<>& GetBuilder();
	llvm::Module& GetModule();

private:
	llvm::LLVMContext m_context;
	llvm::IRBuilder<> m_builder;
	llvm::Module m_module;
};

class CodegenContext
{
public:
	CodegenContext();

	void PushScope();
	void PopScope();

	void Define(const std::string& name, llvm::AllocaInst* value);
	void Assign(const std::string& name, llvm::AllocaInst* value);

	// ���������� nullptr, ���� ���������� �� ������� � ������� ��������� ���������
	llvm::AllocaInst* GetVariable(const std::string& name);

	llvm::Function* GetPrintf();
	CodegenUtils& GetUtils();

	void AddFunction(const std::string& name, llvm::Function* func);
	// ���������� nullptr, ���� ������� �� ������� � ������� ��������� ���������
	llvm::Function* GetFunction(const std::string& name);

	void Dump(std::ostream& out);

private:
	CodegenUtils m_utils;
	ScopeChain<llvm::AllocaInst*> m_scopes;

	llvm::Function* m_printf; // builtin function
	std::unordered_map<std::string, llvm::Function*> m_functions; // user defined
};
