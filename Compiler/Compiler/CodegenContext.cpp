#include "stdafx.h"
#include "CodegenContext.h"

namespace
{
llvm::Function* CreatePrintfBuiltinFunction(CodegenUtils& utils)
{
	llvm::LLVMContext& context = utils.context;

	// ������������ ������������ �������� ������� printf - char*
	std::vector<llvm::Type*> argTypes = { llvm::Type::getInt8PtrTy(context) };

	// �������, ��� ������� ������ int32, � ����� ��� ��� ����� ���������� ����� ����������
	llvm::FunctionType* fnType = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(context), argTypes, true);

	// ������� ������� printf, ������� ����� "����������" � ����������� ������ �����
	return llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "printf", &utils.module);
}
}

CodegenUtils::CodegenUtils()
	: context()
	, builder(context)
	, module("Module", context)
{
}

CodegenContext::CodegenContext()
	: m_utils()
	, m_scopes()
	, m_printf(CreatePrintfBuiltinFunction(m_utils))
{
}

void CodegenContext::PushScope()
{
	m_scopes.PushScope();
}

void CodegenContext::PopScope()
{
	m_scopes.PopScope();
}

void CodegenContext::Define(const std::string& name, llvm::Value* value)
{
	m_scopes.Define(name, value);
}

void CodegenContext::Assign(const std::string& name, llvm::Value* value)
{
	m_scopes.Assign(name, value);
}

llvm::Value* CodegenContext::GetVariable(const std::string& name)
{
	auto variable = m_scopes.GetValue(name);
	return variable.value_or(nullptr);
}

llvm::Function* CodegenContext::GetPrintf()
{
	return m_printf;
}

CodegenUtils& CodegenContext::GetUtils()
{
	return m_utils;
}

void CodegenContext::Dump(std::ostream& out)
{
	llvm::raw_os_ostream os(out);
	m_utils.module.print(os, nullptr);
}
