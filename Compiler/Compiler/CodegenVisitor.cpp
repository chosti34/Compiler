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

llvm::Value* EmitDefaultValue(ASTExpressionType type, CodegenUtils& utils)
{
	switch (type)
	{
	case ASTExpressionType::Int:
		return llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(utils.context),
			llvm::APInt(32, uint64_t(0), true)
		);
	case ASTExpressionType::Float:
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(utils.context), 0.0);
	case ASTExpressionType::Bool:
	case ASTExpressionType::String:
		throw std::logic_error("code generation for bool and string is not implemented yet");
	default:
		throw std::logic_error("can't emit code for undefined ast expression type");
	}
}
}

// Expression codegen visitor
ExpressionCodegen::ExpressionCodegen(CodegenContext& context)
	: m_context(context)
	, m_stack()
{
}

void ExpressionCodegen::CodegenFuncReturningExpression(const IExpressionAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	node.Accept(*this);
	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	// Создаем тип функции
	llvm::FunctionType* fnType = fnType = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(utils.context), llvm::ArrayRef<llvm::Type*>(), false);

	// Создаем функцию main
	llvm::Function* fn = llvm::Function::Create(
		fnType, llvm::Function::InternalLinkage, "main", &utils.module);

	// Создаем базовый блок для функции, куда будет вставлен следующий код
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(utils.context, "entry", fn);
	utils.builder.SetInsertPoint(bb);

	// Создаем функцию printf
	// 1. Создаем вектор типов аргументов, которая принимает функция printf
	std::vector<llvm::Type*> printfProtoArgsTypes;
	printfProtoArgsTypes.push_back(utils.builder.getInt8Ty()->getPointerTo());

	// 2. Создаем тип функции (возвращает int32, принимаем указатель на int8, переменное число аргументов)
	llvm::FunctionType *printfType = llvm::FunctionType::get(
		utils.builder.getInt32Ty(), printfProtoArgsTypes, true);

	// 3. Создаем саму функцию
	llvm::Constant* printfFunc = utils.module.getOrInsertFunction("printf", printfType);

	// 4. В зависимости от типа вычисленного значения, формируем входные данные для функции
	const std::string fmt = value->getType()->isFloatingPointTy() ? "%f\n" : "%d\n";

	// 5. Генерируем код вызова созданного printf'а
	llvm::ArrayRef<llvm::Value*> printfArgs = { 
		EmitStringLiteral(utils.context, utils.module, fmt),
		value
	};
	utils.builder.CreateCall(printfFunc, printfArgs);

	// 6. Генерируем код для инструкции возврата со значением 0
	llvm::Value* exitCode = llvm::ConstantInt::get(
		utils.context, llvm::APInt(32, uint64_t(0), true));
	utils.builder.CreateRet(exitCode);

	// 7. Верифицируем функцию
	if (llvm::verifyFunction(*fn))
	{
		fn->eraseFromParent();
		throw std::runtime_error("error while generating code for main function");
	}
}

llvm::Value* ExpressionCodegen::Visit(const IExpressionAST& node)
{
	node.Accept(*this);
	if (!m_stack.empty())
	{
		llvm::Value* value = m_stack.back();
		m_stack.pop_back();
		return value;
	}
	throw std::logic_error("internal error while generating code for expression");
}

void ExpressionCodegen::Visit(const BinaryExpressionAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
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
		right = utils.builder.CreateSIToFP(
			right, llvm::Type::getDoubleTy(utils.context), "casttmp");
	}

	// Если правый аргумент не целое число, тогда преобразуем левый аргумент в нецелое число
	if (right->getType()->isFloatingPointTy() &&
		!left->getType()->isFloatingPointTy())
	{
		left = utils.builder.CreateSIToFP(
			left, llvm::Type::getDoubleTy(utils.context), "casttmp");
	}

	assert(left->getType()->isFloatingPointTy() == right->getType()->isFloatingPointTy());
	const bool isFloat = left->getType()->isFloatingPointTy();

	llvm::Value *value = nullptr;
	switch (node.GetOperator())
	{
	case BinaryExpressionAST::Plus:
		value = !isFloat ?
			utils.builder.CreateAdd(left, right, "addtmp") :
			utils.builder.CreateFAdd(left, right, "faddtmp");
		break;
	case BinaryExpressionAST::Minus:
		value = !isFloat ?
			utils.builder.CreateSub(left, right, "subtmp") :
			utils.builder.CreateFSub(left, right, "fsubtmp");
		break;
	case BinaryExpressionAST::Mul:
		value = !isFloat ?
			utils.builder.CreateMul(left, right, "multmp") :
			utils.builder.CreateFMul(left, right, "fmultmp");
		break;
	case BinaryExpressionAST::Div:
		value = isFloat ?
			utils.builder.CreateSDiv(left, right, "divtmp") :
			utils.builder.CreateFDiv(left, right, "fdivtmp");
		break;
	default:
		throw std::logic_error("can't generate code for undefined binary operator");
	}

	m_stack.push_back(value);
}

void ExpressionCodegen::Visit(const LiteralConstantAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	const LiteralConstantAST::Value& literal = node.GetValue();

	if (literal.type() == typeid(int))
	{
		const int number = boost::get<int>(literal);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(utils.context), number);
		m_stack.push_back(value);
	}
	else if (literal.type() == typeid(double))
	{
		const double number = boost::get<double>(literal);
		llvm::Value* value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(utils.context), number);
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
	CodegenUtils& utils = m_context.GetUtils();
	node.GetExpr().Accept(*this);

	llvm::Value* value = m_stack.back();
	m_stack.pop_back();

	if (value->getType()->isIntegerTy())
	{
		m_stack.push_back(utils.builder.CreateNeg(value, "negtmp"));
	}
	else if (value->getType()->isFloatingPointTy())
	{
		m_stack.push_back(utils.builder.CreateFNeg(value, "fnegtmp"));
	}
	else
	{
		assert(false);
		throw std::logic_error("code generation error: can't codegen for unary operator on undefined literal type");
	}
}

void ExpressionCodegen::Visit(const IdentifierAST& node)
{
	llvm::Value* value = m_context.GetVariable(node.GetName());
	if (!value)
	{
		throw std::runtime_error("variable '" + node.GetName() + "' is not defined");
	}
	m_stack.push_back(value);
}

// Statement codegen visitor
StatementCodegen::StatementCodegen(CodegenContext& context)
	: m_context(context)
	, m_expressionCodegen(context)
{
}

void StatementCodegen::GenerateMainFn(const IStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	// Создаем тип функции main
	llvm::FunctionType* fnType = fnType = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(utils.context), llvm::ArrayRef<llvm::Type*>(), false);

	// Создаем функцию main
	llvm::Function* fn = llvm::Function::Create(
		fnType, llvm::Function::ExternalLinkage, "main", &utils.module);

	// Создаем базовый блок для функции, куда будет вставлен следующий генерируемый код
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(utils.context, "entry", fn);
	utils.builder.SetInsertPoint(bb);

	// Генерируем основной код
	node.Accept(*this);

	// Генерируем код для инструкции возврата со значением 0
	llvm::Value* exitCode = llvm::ConstantInt::get(
		utils.context, llvm::APInt(32, uint64_t(0), true));
	utils.builder.CreateRet(exitCode);

	// 7. Верифицируем функцию
	if (llvm::verifyFunction(*fn))
	{
		fn->eraseFromParent();
		throw std::runtime_error("error while generating code for main function");
	}
}

void StatementCodegen::Visit(const VariableDeclarationAST& node)
{
	m_context.Define(
		node.GetIdentifier().GetName(),
		EmitDefaultValue(node.GetType(), m_context.GetUtils())
	);
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	m_context.Assign(node.GetIdentifier().GetName(), value);
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
	m_context.PushScope();
	for (size_t i = 0; i < node.GetCount(); ++i)
	{
		node.GetStatement(i).Accept(*this);
	}
	m_context.PopScope();
}

void StatementCodegen::Visit(const PrintAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpression());
	llvm::Function* printf = m_context.GetPrintf();

	const std::string& fmt = value->getType()->isDoubleTy() ? "%f\n" : "%d\n";
	std::vector<llvm::Value*> args = { EmitStringLiteral(utils.context, utils.module, fmt), value };

	utils.builder.CreateCall(printf, args);
}
