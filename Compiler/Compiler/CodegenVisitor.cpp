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

CodegenUtils::CodegenUtils()
	: context()
	, builder(context)
	, module("Module", context)
{
}

// Expression codegen visitor
ExpressionCodegen::ExpressionCodegen(CodegenUtils& utils)
	: m_stack()
	, m_utils(utils)
{
}

void ExpressionCodegen::CodegenFuncReturningExpression(const IExpressionAST& node)
{
	node.Accept(*this);

	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	// Создаем тип функции
	llvm::FunctionType* fnType = fnType = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(m_utils.context), llvm::ArrayRef<llvm::Type*>(), false);

	// Создаем функцию main
	llvm::Function* fn = llvm::Function::Create(
		fnType, llvm::Function::ExternalLinkage, "main", &m_utils.module);

	// Создаем базовый блок для функции, куда будет вставлен следующий код
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(m_utils.context, "entry", fn);
	m_utils.builder.SetInsertPoint(bb);

	// Создаем функцию printf
	// 1. Создаем вектор типов аргументов, которая принимает функция printf
	std::vector<llvm::Type*> printfProtoArgsTypes;
	printfProtoArgsTypes.push_back(m_utils.builder.getInt8Ty()->getPointerTo());

	// 2. Создаем тип функции (возвращает int32, принимаем указатель на int8, переменное число аргументов)
	llvm::FunctionType *printfType = llvm::FunctionType::get(
		m_utils.builder.getInt32Ty(), printfProtoArgsTypes, true);

	// 3. Создаем саму функцию
	llvm::Constant* printfFunc = m_utils.module.getOrInsertFunction("printf", printfType);

	// 4. В зависимости от типа вычисленного значения, формируем входные данные для функции
	const std::string fmt = value->getType()->isFloatingPointTy() ? "%f\n" : "%d\n";

	// 5. Генерируем код вызова созданного printf'а
	llvm::ArrayRef<llvm::Value*> printfArgs = { 
		EmitStringLiteral(m_utils.context, m_utils.module, fmt),
		value
	};
	m_utils.builder.CreateCall(printfFunc, printfArgs);

	// 6. Генерируем код для инструкции возврата со значением 0
	llvm::Value* exitCode = llvm::ConstantInt::get(
		m_utils.context, llvm::APInt(32, uint64_t(0), true));
	m_utils.builder.CreateRet(exitCode);

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
		right = m_utils.builder.CreateSIToFP(
			right, llvm::Type::getDoubleTy(m_utils.context), "casttmp");
	}

	// Если правый аргумент не целое число, тогда преобразуем левый аргумент в нецелое число
	if (right->getType()->isFloatingPointTy() &&
		!left->getType()->isFloatingPointTy())
	{
		left = m_utils.builder.CreateSIToFP(
			left, llvm::Type::getDoubleTy(m_utils.context), "casttmp");
	}

	assert(left->getType()->isFloatingPointTy() == right->getType()->isFloatingPointTy());
	const bool isFloat = left->getType()->isFloatingPointTy();

	llvm::Value *value = nullptr;
	switch (node.GetOperator())
	{
	case BinaryExpressionAST::Plus:
		value = !isFloat ?
			m_utils.builder.CreateAdd(left, right, "addtmp") :
			m_utils.builder.CreateFAdd(left, right, "faddtmp");
		break;
	case BinaryExpressionAST::Minus:
		value = !isFloat ?
			m_utils.builder.CreateSub(left, right, "subtmp") :
			m_utils.builder.CreateFSub(left, right, "fsubtmp");
		break;
	case BinaryExpressionAST::Mul:
		value = !isFloat ?
			m_utils.builder.CreateMul(left, right, "multmp") :
			m_utils.builder.CreateFMul(left, right, "fmultmp");
		break;
	case BinaryExpressionAST::Div:
		value = isFloat ?
			m_utils.builder.CreateSDiv(left, right, "divtmp") :
			m_utils.builder.CreateFDiv(left, right, "fdivtmp");
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
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_utils.context), number);
		m_stack.push_back(value);
	}
	else if (literal.type() == typeid(double))
	{
		const double number = boost::get<double>(literal);
		llvm::Value* value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(m_utils.context), number);
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
		m_stack.push_back(m_utils.builder.CreateNeg(value, "negtmp"));
	}
	else if (value->getType()->isFloatingPointTy())
	{
		m_stack.push_back(m_utils.builder.CreateFNeg(value, "fnegtmp"));
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
	: m_utils()
	, m_expressionCodegen(m_utils)
{
}

void StatementCodegen::Generate(const IStatementAST& node, std::ostream& out)
{
	// TODO: implement this
	node.Accept(*this);

	// Выводим результат кодогенерации в указанный поток вывода
	llvm::raw_os_ostream os(out);
	m_utils.module.print(os, nullptr);
}

void StatementCodegen::Visit(const VariableDeclarationAST& node)
{
	// TODO: implement this
	(void)node;
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	// TODO: implement this
	(void)node;
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	(void)node;
	throw std::logic_error("code generation for return statement is not implemented");
}

void StatementCodegen::Visit(const IfStatementAST& node)
{
	(void)node;
	throw std::logic_error("code generation for if statement is not implemented");
}

void StatementCodegen::Visit(const WhileStatementAST& node)
{
	(void)node;
	throw std::logic_error("code generation for while statement is not implemented");
}

void StatementCodegen::Visit(const CompositeStatementAST& node)
{
	// TODO: implement this
	for (size_t i = 0; i < node.GetCount(); ++i)
	{
		node.GetStatement(i).Accept(*this);
	}
}
