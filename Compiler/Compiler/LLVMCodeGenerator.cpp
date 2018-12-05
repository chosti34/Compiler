#include "stdafx.h"
#include "LLVMCodeGenerator.h"

namespace
{
llvm::Value* CreateBinaryExpressionCodeInt(llvm::Value* left, llvm::Value* right, llvm::IRBuilder<> & builder, BinaryExpressionAST::Operator operation)
{
	switch (operation)
	{
	case BinaryExpressionAST::Plus:
		return builder.CreateAdd(left, right, "addtmp");
	case BinaryExpressionAST::Minus:
		return builder.CreateSub(left, right, "subtmp");
	case BinaryExpressionAST::Mul:
		return builder.CreateMul(left, right, "multmp");
	case BinaryExpressionAST::Div:
		return builder.CreateSDiv(left, right, "divtmp");
	}
	return nullptr;
}

llvm::Value* CreateBinaryExpressionCodeFloat(llvm::Value* left, llvm::Value* right, llvm::IRBuilder<> & builder, BinaryExpressionAST::Operator operation)
{
	switch (operation)
	{
	case BinaryExpressionAST::Plus:
		return builder.CreateFAdd(left, right, "addtmp");
	case BinaryExpressionAST::Minus:
		return builder.CreateFSub(left, right, "subtmp");
	case BinaryExpressionAST::Mul:
		return builder.CreateFMul(left, right, "multmp");
	case BinaryExpressionAST::Div:
		return builder.CreateFDiv(left, right, "divtmp");
	}
	return nullptr;
}

llvm::Constant* EmitStringLiteral(llvm::LLVMContext& context, llvm::Module& module, const std::string& value)
{
	llvm::Constant* constant = llvm::ConstantDataArray::getString(context, value, true);

	// ???
	llvm::GlobalVariable* global = new llvm::GlobalVariable(
		module, constant->getType(), true, llvm::GlobalVariable::InternalLinkage, constant, "str");

	llvm::Constant* index = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(context));
	std::vector<llvm::Constant*> indices = { index, index };

	return llvm::ConstantExpr::getInBoundsGetElementPtr(constant->getType(), global, indices);
}
}

LLVMCodeGenerator::LLVMCodeGenerator()
	: m_stack()
	, m_context()
	, m_builder(m_context)
	, m_module("module", m_context)
{
}

void LLVMCodeGenerator::CodegenFuncReturningExpression(const IExpressionAST& ast)
{
	ast.Accept(*this);
	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	if (value->getType()->isFloatTy())
	{
		value = m_builder.CreateFPCast(value, llvm::Type::getDoubleTy(m_context), "casttmp");
	}

	llvm::FunctionType* fnType = fnType = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), llvm::ArrayRef<llvm::Type*>(), false);
	llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", &m_module);
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(m_context, "entry", fn);
	m_builder.SetInsertPoint(bb);

	std::vector<llvm::Type*> printfProtoArgs;
	printfProtoArgs.push_back(m_builder.getInt8Ty()->getPointerTo());
	llvm::FunctionType *printfType = llvm::FunctionType::get(m_builder.getInt32Ty(), printfProtoArgs, true);
	llvm::Constant* printfFunc = m_module.getOrInsertFunction("printf", printfType);
	llvm::ArrayRef<llvm::Value*> printfArgs = { EmitStringLiteral(m_context, m_module, value->getType()->isDoubleTy() ? "%f\n" : "%d\n"), value };
	m_builder.CreateCall(printfFunc, printfArgs);

	llvm::Value* exitCode = llvm::ConstantInt::get(m_context, llvm::APInt(32, uint64_t(0), true));
	m_builder.CreateRet(exitCode);

	if (llvm::verifyFunction(*fn))
	{
		fn->eraseFromParent();
		throw std::runtime_error("main function is not verified");
	}
}

llvm::Module& LLVMCodeGenerator::GetLLVMModule()
{
	return m_module;
}

const llvm::Module& LLVMCodeGenerator::GetLLVMModule()const
{
	return m_module;
}

void LLVMCodeGenerator::Visit(const BinaryExpressionAST& node)
{
	node.GetLeft().Accept(*this);
	node.GetRight().Accept(*this);

	llvm::Value *right = m_stack.back();
	m_stack.pop_back();

	llvm::Value *left = m_stack.back();
	m_stack.pop_back();

	if (left->getType()->isFloatTy() && !right->getType()->isFloatTy())
	{
		right = m_builder.CreateSIToFP(right, llvm::Type::getFloatTy(m_context), "casttmp");
	}
	if (right->getType()->isFloatTy() && !left->getType()->isFloatTy())
	{
		left = m_builder.CreateSIToFP(left, llvm::Type::getFloatTy(m_context), "casttmp");
	}

	llvm::Value *value = nullptr;

	if (left->getType()->isFloatTy() || right->getType()->isFloatTy())
	{
		value = CreateBinaryExpressionCodeFloat(left, right, m_builder, node.GetOperator());
	}
	else
	{
		value = CreateBinaryExpressionCodeInt(left, right, m_builder, node.GetOperator());
	}

	assert(value);
	m_stack.push_back(value);
}

void LLVMCodeGenerator::Visit(const LiteralConstantAST& node)
{
	const LiteralConstantAST::Value& literal = node.GetValue();
	if (literal.type() == typeid(int))
	{
		int integer = boost::get<int>(literal);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), integer);
		m_stack.push_back(value);
	}
	else if (literal.type() == typeid(float))
	{
		float floating = boost::get<float>(literal);
		llvm::Value* value = llvm::ConstantFP::get(m_context, llvm::APFloat(floating));
		m_stack.push_back(value);
	}
	else
	{
		assert(false);
		throw std::logic_error("code generation error: can't codegen for undefined literal type");
	}
}

void LLVMCodeGenerator::Visit(const UnaryAST& node)
{
	node.GetExpr().Accept(*this);

	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	if (value->getType()->isIntegerTy())
	{
		m_stack.push_back(m_builder.CreateNeg(value, "negtmp"));
	}
	else if (value->getType()->isFloatTy())
	{
		m_stack.push_back(m_builder.CreateFNeg(value, "fnegtmp"));
	}
	else
	{
		assert(false);
		throw std::logic_error("code generation error: can't codegen for unary operator on undefined literal type");
	}
}

void LLVMCodeGenerator::Visit(const IdentifierAST& node)
{
	(void)node;
	throw std::logic_error("code generation error: variables are not implemented yet!");
}
