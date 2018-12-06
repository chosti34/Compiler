#include "stdafx.h"
#include "CodegenVisitor.h"

namespace
{
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

LLVMCodegen::LLVMCodegen()
	: context()
	, builder(context)
	, module("Module", context)
{
}

// Expression codegen visitor
ExpressionCodegen::ExpressionCodegen(LLVMCodegen& llvmCodegen)
	: m_stack()
	, m_llvmCodegen(llvmCodegen)
{
}

void ExpressionCodegen::CodegenFuncReturningExpression(const IExpressionAST& node)
{
	node.Accept(*this);

	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	// Создаем тип функции
	llvm::FunctionType* fnType = fnType = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(m_llvmCodegen.context), llvm::ArrayRef<llvm::Type*>(), false);

	// Создаем функцию main
	llvm::Function* fn = llvm::Function::Create(
		fnType, llvm::Function::ExternalLinkage, "main", &m_llvmCodegen.module);

	// Создаем базовый блок для функции, куда будет вставлен следующий код
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(m_llvmCodegen.context, "entry", fn);
	m_llvmCodegen.builder.SetInsertPoint(bb);

	// Создаем функцию printf
	// 1. Создаем вектор типов аргументов, которая принимает функция printf
	std::vector<llvm::Type*> printfProtoArgsTypes;
	printfProtoArgsTypes.push_back(m_llvmCodegen.builder.getInt8Ty()->getPointerTo());

	// 2. Создаем тип функции (возвращает int32, принимаем указатель на int8, переменное число аргументов)
	llvm::FunctionType *printfType = llvm::FunctionType::get(
		m_llvmCodegen.builder.getInt32Ty(), printfProtoArgsTypes, true);

	// 3. Создаем саму функцию
	llvm::Constant* printfFunc = m_llvmCodegen.module.getOrInsertFunction("printf", printfType);

	// 4. В зависимости от типа вычисленного значения, формируем входные данные для функции
	const std::string fmt = value->getType()->isFloatingPointTy() ? "%f\n" : "%d\n";

	// 5. Генерируем код вызова созданного printf'а
	llvm::ArrayRef<llvm::Value*> printfArgs = { 
		EmitStringLiteral(m_llvmCodegen.context, m_llvmCodegen.module, fmt),
		value
	};
	m_llvmCodegen.builder.CreateCall(printfFunc, printfArgs);

	// 6. Генерируем код для инструкции возврата со значением 0
	llvm::Value* exitCode = llvm::ConstantInt::get(
		m_llvmCodegen.context, llvm::APInt(32, uint64_t(0), true));
	m_llvmCodegen.builder.CreateRet(exitCode);

	// 7. Верифицируем функцию
	if (llvm::verifyFunction(*fn))
	{
		fn->eraseFromParent();
		throw std::runtime_error("error while generating code for main function");
	}
}

void ExpressionCodegen::Visit(const BinaryExpressionAST& node)
{
	node.GetLeft().Accept(*this);
	node.GetRight().Accept(*this);

	llvm::Value* right = m_stack.back();
	m_stack.pop_back();

	llvm::Value* left = m_stack.back();
	m_stack.pop_back();

	// Если левый аргумент нецелое число, тогда преобразуем правый аргумент в нецелое число
	if (left->getType()->isFloatingPointTy() &&
		!right->getType()->isFloatingPointTy())
	{
		right = m_llvmCodegen.builder.CreateSIToFP(
			right, llvm::Type::getDoubleTy(m_llvmCodegen.context), "casttmp");
	}

	// Если правый аргумент не целое число, тогда преобразуем левый аргумент в нецелое число
	if (right->getType()->isFloatingPointTy() &&
		!left->getType()->isFloatingPointTy())
	{
		left = m_llvmCodegen.builder.CreateSIToFP(
			left, llvm::Type::getDoubleTy(m_llvmCodegen.context), "casttmp");
	}

	assert(left->getType()->isFloatingPointTy() == right->getType()->isFloatingPointTy());
	const bool isFloat = left->getType()->isFloatingPointTy();

	llvm::Value *value = nullptr;
	switch (node.GetOperator())
	{
	case BinaryExpressionAST::Plus:
		value = !isFloat ?
			m_llvmCodegen.builder.CreateAdd(left, right, "addtmp") :
			m_llvmCodegen.builder.CreateFAdd(left, right, "faddtmp");
		break;
	case BinaryExpressionAST::Minus:
		value = !isFloat ?
			m_llvmCodegen.builder.CreateSub(left, right, "subtmp") :
			m_llvmCodegen.builder.CreateFSub(left, right, "fsubtmp");
		break;
	case BinaryExpressionAST::Mul:
		value = !isFloat ?
			m_llvmCodegen.builder.CreateMul(left, right, "multmp") :
			m_llvmCodegen.builder.CreateFMul(left, right, "fmultmp");
		break;
	case BinaryExpressionAST::Div:
		value = isFloat ?
			m_llvmCodegen.builder.CreateSDiv(left, right, "divtmp") :
			m_llvmCodegen.builder.CreateFDiv(left, right, "fdivtmp");
		break;
	default:
		throw std::logic_error("can't generate code for undefined binary operator");
	}

	m_stack.push_back(value);
}

void ExpressionCodegen::Visit(const LiteralConstantAST& node)
{
	const LiteralConstantAST::Value& literal = node.GetValue();
	if (literal.type() == typeid(int))
	{
		const int number = boost::get<int>(literal);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_llvmCodegen.context), number);
		m_stack.push_back(value);
	}
	else if (literal.type() == typeid(double))
	{
		const double number = boost::get<double>(literal);
		llvm::Value* value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(m_llvmCodegen.context), number);
		m_stack.push_back(value);
	}
	else
	{
		assert(false);
		throw std::logic_error("code generation error: can't codegen for undefined literal type");
	}
}

void ExpressionCodegen::Visit(const UnaryAST& node)
{
	node.GetExpr().Accept(*this);

	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	if (value->getType()->isIntegerTy())
	{
		m_stack.push_back(m_llvmCodegen.builder.CreateNeg(value, "negtmp"));
	}
	else if (value->getType()->isFloatingPointTy())
	{
		m_stack.push_back(m_llvmCodegen.builder.CreateFNeg(value, "fnegtmp"));
	}
	else
	{
		assert(false);
		throw std::logic_error("code generation error: can't codegen for unary operator on undefined literal type");
	}
}

void ExpressionCodegen::Visit(const IdentifierAST& node)
{
	(void)node;
	throw std::logic_error("code generation error: variables are not implemented yet!");
}

// Statement codegen visitor
StatementCodegen::StatementCodegen()
	: m_llvmCodegen()
	, m_expressionCodegen(m_llvmCodegen)
{
}

void StatementCodegen::Codegen(const IStatementAST& node)
{
	node.Accept(*this);
}

void StatementCodegen::Run(const IExpressionAST & node)
{
	m_expressionCodegen.CodegenFuncReturningExpression(node);

	llvm::raw_os_ostream output(std::cout);
	m_llvmCodegen.module.print(output, nullptr);
}

void StatementCodegen::Visit(const VariableDeclarationAST& node)
{
	(void)node;
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	(void)node;
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	(void)node;
}

void StatementCodegen::Visit(const IfStatementAST& node)
{
	(void)node;
}

void StatementCodegen::Visit(const WhileStatementAST& node)
{
	(void)node;
}

void StatementCodegen::Visit(const CompositeStatementAST& node)
{
	(void)node;
}
