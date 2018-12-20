#include "stdafx.h"
#include "CodegenVisitor.h"

namespace
{
ExpressionType GetExpressionTypeOfIntegerLLVMType(llvm::Type* type)
{
	assert(type->isIntegerTy());
	switch (type->getIntegerBitWidth())
	{
	case 32:
		return ExpressionType::Int;
	case 1:
		return ExpressionType::Bool;
	default:
		throw std::invalid_argument("unsupported integer bit width");
	}
}

ExpressionType ToExpressionType(llvm::Type* type)
{
	if (type->isIntegerTy())
	{
		return GetExpressionTypeOfIntegerLLVMType(type);
	}
	if (type->isDoubleTy())
	{
		return ExpressionType::Float;
	}
	throw std::invalid_argument("unsupported llvm type");
}

llvm::Type* ToLLVMType(ExpressionType type, llvm::LLVMContext& context)
{
	switch (type)
	{
	case ExpressionType::Int:
		return llvm::Type::getInt32Ty(context);
	case ExpressionType::Float:
		return llvm::Type::getDoubleTy(context);
	case ExpressionType::Bool:
		return llvm::Type::getInt1Ty(context);
	default:
		throw std::logic_error("can't convert string ast literal to llvm type");
	}
}

llvm::Value* ConvertToIntegerValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());

	switch (type)
	{
	case ExpressionType::Int:
		return value;
	case ExpressionType::Float:
		return builder.CreateFPToSI(value, llvm::Type::getInt32Ty(llvmContext), "icasttmp");
	case ExpressionType::Bool:
		return builder.CreateBitCast(value, llvm::Type::getInt32Ty(llvmContext), "icasttmp");
	case ExpressionType::String:
		throw std::runtime_error("can't cast string to integer");
	}

	assert(false);
	throw std::logic_error("ConvertToIntValue() - value type is unknown");
}

llvm::Value* ConvertToFloatValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());

	switch (type)
	{
	case ExpressionType::Int:
	case ExpressionType::Bool:
		return builder.CreateSIToFP(value, llvm::Type::getDoubleTy(llvmContext), "fcasttmp");
	case ExpressionType::Float:
		return value;
	case ExpressionType::String:
		throw std::runtime_error("can't cast string to float");
	}

	assert(false);
	throw std::logic_error("ConvertToIntValue() - value type is unknown");
}

llvm::Value* ConvertToBooleanValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType expressionType = ToExpressionType(value->getType());

	switch (expressionType)
	{
	case ExpressionType::Int:
		return builder.CreateNot(builder.CreateICmpEQ(
			value, llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), uint64_t(0)), "bcasttmp"));
	case ExpressionType::Float:
		return builder.CreateNot(builder.CreateFCmpOEQ(
			value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0), "fcmptmp"), "nottmp");
	case ExpressionType::Bool:
		return value;
	case ExpressionType::String:
		throw std::runtime_error("can't cast string to bool");
	}

	assert(false);
	throw std::logic_error("ConvertToBooleanValue() - undefined ast expression type");
}

// Возвращает nullptr, либо бросает исключение, если нельзя
//  сгенерировать код преобразования значения в другой тип
llvm::Value* CastValue(
	llvm::Value* value,
	ExpressionType type,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<> & builder)
{
	switch (type)
	{
	case ExpressionType::Int:
		return ConvertToIntegerValue(value, llvmContext, builder);
	case ExpressionType::Float:
		return ConvertToFloatValue(value, llvmContext, builder);
	case ExpressionType::Bool:
		return ConvertToBooleanValue(value, llvmContext, builder);
	}
	return nullptr;
}

llvm::Value* CreateIntegerBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator operation,
	llvm::IRBuilder<> & builder)
{
	assert(ToExpressionType(left->getType()) == ToExpressionType(right->getType()));
	assert(ToExpressionType(left->getType()) == ExpressionType::Int);

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
	case BinaryExpressionAST::Mod:
		return builder.CreateSRem(left, right, "modtmp");
	}

	assert(false);
	throw std::logic_error("CreateIntBinaryExpression() - undefined binary expression ast operator");
}

llvm::Value* CreateFloatBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator operation,
	llvm::IRBuilder<> & builder)
{
	assert(ToExpressionType(left->getType()) == ToExpressionType(right->getType()));
	assert(ToExpressionType(left->getType()) == ExpressionType::Float);

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
	case BinaryExpressionAST::Mod:
		return builder.CreateFRem(left, right, "modtmp");
	}

	assert(false);
	throw std::logic_error("CreateFloatBinaryExpression() - undefined binary expression ast operator");
}

llvm::Value* CreateBooleanBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator operation,
	llvm::IRBuilder<> & builder)
{
	assert(ToExpressionType(left->getType()) == ToExpressionType(right->getType()));
	assert(ToExpressionType(left->getType()) == ExpressionType::Bool);

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
	case BinaryExpressionAST::Mod:
		return builder.CreateSRem(left, right, "modtmp");
	}

	assert(false);
	throw std::logic_error("CreateBooleanBinaryExpression() - undefined binary expression ast operator");
}

llvm::Constant* CreateGlobalStringLiteral(
	llvm::LLVMContext& llvmContext,
	llvm::Module& llvmModule,
	const std::string& value)
{
	llvm::Constant* constant = llvm::ConstantDataArray::getString(llvmContext, value, true);

	// ???
	llvm::GlobalVariable* global = new llvm::GlobalVariable(
		llvmModule, constant->getType(), true,
		llvm::GlobalVariable::InternalLinkage, constant, "str");

	llvm::Constant* index = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(llvmContext));
	std::vector<llvm::Constant*> indices = { index, index };

	return llvm::ConstantExpr::getInBoundsGetElementPtr(constant->getType(), global, indices);
}

llvm::AllocaInst* CreateLocalVariable(llvm::Function* func, llvm::Type* type, const std::string& name)
{
	llvm::BasicBlock& block = func->getEntryBlock();
	llvm::IRBuilder<> builder(&block, block.begin());
	return builder.CreateAlloca(type, nullptr, name);
}

llvm::Value* CreateDefaultValue(ExpressionType type, llvm::LLVMContext& llvmContext)
{
	switch (type)
	{
	case ExpressionType::Int:
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), llvm::APInt(32, uint64_t(0), true));
	case ExpressionType::Float:
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0);
	case ExpressionType::Bool:
	case ExpressionType::String:
		throw std::logic_error("code generation for bool and string is not implemented yet");
	default:
		assert(false);
		throw std::logic_error("can't emit code for undefined ast expression type");
	}
}

class ContextScopeHelper
{
public:
	explicit ContextScopeHelper(CodegenContext& context)
		: m_context(context)
	{
		m_context.PushScope();
	}

	~ContextScopeHelper()
	{
		m_context.PopScope();
	}

private:
	CodegenContext& m_context;
};
}

// Expression codegen visitor
ExpressionCodegen::ExpressionCodegen(CodegenContext& context)
	: m_context(context)
	, m_stack()
{
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

	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Value* left = Visit(node.GetLeft());
	llvm::Value* right = Visit(node.GetRight());

	// Есть два варианта действий при посещении узла бинарного оператора:
	//  1. Посмотреть тип левого операнда, и привести правый операнд к этому же типу,
	//     далее смотреть на оператор и генерироровать код в зависимости от оператора и от типа обеих частей выражения
	//  2. Посмотреть на тип правого и левого операнда. По определенному приоритету, выполнить преобразование
	//     обеих частей выражения в один тип. Далее генерировать код, в зависимости от оператора и от типа

	if (ToExpressionType(left->getType()) != ToExpressionType(right->getType()))
	{
		const auto castType = GetPreferredType(
			ToExpressionType(left->getType()),
			ToExpressionType(right->getType()));

		if (!castType)
		{
			const auto fmt = boost::format("can't codegen operator '%1%' on operands with types '%2%' and '%3%'")
				% ToString(node.GetOperator())
				% ToString(ToExpressionType(left->getType()))
				% ToString(ToExpressionType(right->getType()));
			throw std::runtime_error(fmt.str());
		}

		switch (*castType)
		{
		case ExpressionType::Int:
			left = ConvertToIntegerValue(left, llvmContext, builder);
			right = ConvertToIntegerValue(right, llvmContext, builder);
			break;
		case ExpressionType::Float:
			left = ConvertToFloatValue(left, llvmContext, builder);
			right = ConvertToFloatValue(right, llvmContext, builder);
			break;
		case ExpressionType::Bool:
			left = ConvertToBooleanValue(left, llvmContext, builder);
			right = ConvertToBooleanValue(right, llvmContext, builder);
			break;
		case ExpressionType::String:
			throw std::runtime_error("can't codegen binary operator for string");
		default:
			throw std::logic_error("can't codegen binary operator for undefined expression type");
		}

		// TODO: produce warning here
	}

	assert(ToExpressionType(left->getType()) == ToExpressionType(right->getType()));

	switch (ToExpressionType(left->getType()))
	{
	case ExpressionType::Int:
		m_stack.push_back(CreateIntegerBinaryExpression(left, right, node.GetOperator(), builder));
		break;
	case ExpressionType::Float:
		m_stack.push_back(CreateFloatBinaryExpression(left, right, node.GetOperator(), builder));
		break;
	case ExpressionType::Bool:
		m_stack.push_back(CreateBooleanBinaryExpression(left, right, node.GetOperator(), builder));
		break;
	default:
		throw std::runtime_error("can't codegen binary operator '" +
			ToString(node.GetOperator()) + "' for " + ToString(ToExpressionType(left->getType())));
	}
}

void ExpressionCodegen::Visit(const LiteralConstantAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();
	const LiteralConstantAST::Value& constant = node.GetValue();

	if (constant.type() == typeid(int))
	{
		const int number = boost::get<int>(constant);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), number);
		m_stack.push_back(value);
	}
	else if (constant.type() == typeid(double))
	{
		const double number = boost::get<double>(constant);
		llvm::Value* value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), number);
		m_stack.push_back(value);
	}
	else
	{
		assert(false);
		throw std::logic_error("Visit(LiteralConstantAST) - can't codegen for undefined literal constant type");
	}
}

void ExpressionCodegen::Visit(const UnaryAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	llvm::Value* value = Visit(node.GetExpr());
	const ExpressionType type = ToExpressionType(value->getType());

	if (type == ExpressionType::Int)
	{
		m_stack.push_back(builder.CreateNeg(value, "negtmp"));
	}
	else if (type == ExpressionType::Float)
	{
		m_stack.push_back(builder.CreateFNeg(value, "fnegtmp"));
	}
	else
	{
		assert(false);
		throw std::logic_error("Visit(UnaryAST) - can't codegen for unary on undefined literal constant type");
	}
}

void ExpressionCodegen::Visit(const IdentifierAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	const std::string& name = node.GetName();
	llvm::AllocaInst* variable = m_context.GetVariable(name);

	if (!variable)
	{
		throw std::runtime_error("variable '" + name + "' is not defined");
	}

	llvm::Value* value = builder.CreateLoad(variable, name + "Value");
	m_stack.push_back(value);
}

void ExpressionCodegen::Visit(const FunctionCallExprAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = m_context.GetFunction(node.GetName());
	if (!func)
	{
		throw std::runtime_error("calling function '" + node.GetName() + "' that isn't defined");
	}

	if (func->arg_size() != node.GetParamsCount())
	{
		boost::format fmt("function '%1%' expects %2% params, %3% given");
		throw std::runtime_error((fmt % node.GetName() % func->arg_size() % node.GetParamsCount()).str());
	}

	size_t index = 0;
	std::vector<llvm::Value*> params;

	for (llvm::Argument& arg : func->args())
	{
		llvm::Value* value = Visit(node.GetParam(index));

		if (ToExpressionType(value->getType()) != ToExpressionType(arg.getType()))
		{
			llvm::Value* casted = CastValue(value, ToExpressionType(arg.getType()), llvmContext, builder);
			if (!casted)
			{
				auto fmt = boost::format("function '%1%' expects '%2%' as parameter, '%3%' given (can't cast)")
					% func->getName().str()
					% ToString(ToExpressionType(arg.getType()))
					% ToString(ToExpressionType(value->getType()));
				throw std::runtime_error(fmt.str());
			}

			assert(ToExpressionType(casted->getType()) == ToExpressionType(arg.getType()));
			params.push_back(casted);
			continue;
		}

		assert(ToExpressionType(value->getType()) == ToExpressionType(arg.getType()));
		params.push_back(value);
		++index;
	}

	llvm::Value* value = builder.CreateCall(func, params, "calltmp");
	m_stack.push_back(value);
}

// Statement codegen visitor
StatementCodegen::StatementCodegen(CodegenContext& context)
	: m_context(context)
	, m_expressionCodegen(context)
{
}

void StatementCodegen::Visit(const IStatementAST& node)
{
	node.Accept(*this);
}

void StatementCodegen::Visit(const VariableDeclarationAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	const std::string& name = node.GetIdentifier().GetName();
	if (m_context.GetVariable(name))
	{
		throw std::runtime_error("variable '" + name + "' is already defined");
	}

	// Создаем переменную
	llvm::Type* type = ToLLVMType(node.GetType(), llvmContext);
	llvm::Function* func = builder.GetInsertBlock()->getParent();
	llvm::AllocaInst* variable = CreateLocalVariable(func, type, name + "Ptr");

	// Устанавливаем переменной значение по умолчанию
	llvm::Value* defaultValue = CreateDefaultValue(node.GetType(), llvmContext);
	builder.CreateStore(defaultValue, variable);

	// Сохраняем переменную в контекст
	m_context.Define(name, variable);

	// Обработка опционального блока присваивания
	if (const IExpressionAST* expression = node.GetExpression())
	{
		llvm::Value* value = m_expressionCodegen.Visit(*expression);

		if (ToExpressionType(value->getType()) != node.GetType())
		{
			llvm::Value* casted = CastValue(value, node.GetType(), llvmContext, builder);
			if (!casted)
			{
				auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
					% ToString(ToExpressionType(value->getType()))
					% name
					% ToString(node.GetType());
				throw std::runtime_error(fmt.str());
			}

			// TODO: produce warning here
			assert(casted->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
			builder.CreateStore(casted, variable);
			return;
		}

		assert(value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
		builder.CreateStore(value, variable);
	}
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	const std::string& name = node.GetIdentifier().GetName();
	llvm::AllocaInst* variable = m_context.GetVariable(name);
	if (!variable)
	{
		throw std::runtime_error("can't assign because variable '" + name + "' is not defined");
	}

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());

	if (ToExpressionType(value->getType()) != ToExpressionType(variable->getType()->getPointerElementType()))
	{
		llvm::Value* casted = CastValue(value, ToExpressionType(variable->getType()->getPointerElementType()), llvmContext, builder);
		if (!casted)
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
				% ToString(ToExpressionType(value->getType()))
				% name
				% ToString(ToExpressionType(variable->getType()->getPointerElementType()));
			throw std::runtime_error(fmt.str());
		}

		// TODO: produce warning here
		assert(casted->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
		builder.CreateStore(casted, variable);
		return;
	}

	assert(value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
	builder.CreateStore(value, variable);
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();
	const ExpressionType funcReturnType = ToExpressionType(func->getReturnType());
	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());

	if (ToExpressionType(value->getType()) != funcReturnType)
	{
		value = CastValue(value, funcReturnType, llvmContext, builder);
		if (!value)
		{
			throw std::runtime_error("returning expression must be at least convertible to function return type");
		}
	}

	assert(ToExpressionType(value->getType()) == funcReturnType);
	builder.CreateRet(value);
}

void StatementCodegen::Visit(const IfStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::IRBuilder<> & builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(llvmContext, "then", func);
	llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(llvmContext, "else", func);
	llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(llvmContext, "continue", func);

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	value = ConvertToBooleanValue(value, llvmContext, builder);
	builder.CreateCondBr(value, thenBlock, elseBlock);

	auto putBrAfterBranchInsertionIfNecessary = [&](llvm::BasicBlock* branch) {
		// Если блок не имеет выхода, добавляем безусловный переход
		if (!branch->getTerminator())
		{
			builder.CreateBr(continueBlock);
			return;
		}
		// Если блок имеет вложенные if инструкции, и при этом блок continue не имеет выхода,
		//  добавляем безусловный переход
		if (!m_branchContinueStack.empty())
		{
			const bool hasTerminated = m_branchContinueStack.back()->getTerminator();
			m_branchContinueStack.pop_back();
			if (!hasTerminated)
			{
				builder.CreateBr(continueBlock);
			}
		}
	};

	builder.SetInsertPoint(thenBlock);
	Visit(node.GetThenStmt());
	putBrAfterBranchInsertionIfNecessary(thenBlock);

	builder.SetInsertPoint(elseBlock);
	if (node.GetElseStmt())
	{
		Visit(*node.GetElseStmt());
	}
	putBrAfterBranchInsertionIfNecessary(elseBlock);

	builder.SetInsertPoint(continueBlock);
	m_branchContinueStack.push_back(continueBlock);
}

void StatementCodegen::Visit(const WhileStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::IRBuilder<> & builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "loop", func);
	llvm::BasicBlock* afterLoop = llvm::BasicBlock::Create(llvmContext, "afterloop", func);

	llvm::Value* value = ConvertToBooleanValue(
		m_expressionCodegen.Visit(node.GetExpr()), llvmContext, builder);
	builder.CreateCondBr(value, body, afterLoop);

	builder.SetInsertPoint(body);
	Visit(node.GetStatement());
	value = ConvertToBooleanValue(
		m_expressionCodegen.Visit(node.GetExpr()), llvmContext, builder);
	builder.CreateCondBr(value, body, afterLoop);

	builder.SetInsertPoint(afterLoop);
}

void StatementCodegen::Visit(const CompositeStatementAST& node)
{
	ContextScopeHelper scopedContext(m_context);
	llvm::IRBuilder<>& builder = m_context.GetUtils().GetBuilder();

	for (size_t i = 0; i < node.GetCount(); ++i)
	{
		node.GetStatement(i).Accept(*this);
		if (builder.GetInsertBlock()->getTerminator())
		{
			break;
		}
		// TODO: produce warning about unreachable code
	}
}

void StatementCodegen::Visit(const PrintAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();
	llvm::IRBuilder<> & builder = utils.GetBuilder();
	llvm::Module& llvmModule = utils.GetModule();

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpression());
	if (!value->getType()->isIntegerTy() && !value->getType()->isDoubleTy())
	{
		throw std::runtime_error("strings and booleans can't be printed out yet");
	}

	llvm::Function* printf = m_context.GetPrintf();

	const std::string& fmt = value->getType()->isDoubleTy() ? "%f\n" : "%d\n";
	std::vector<llvm::Value*> args = { CreateGlobalStringLiteral(llvmContext, llvmModule, fmt), value };

	builder.CreateCall(printf, args, "printtmp");
}

void StatementCodegen::Visit(const FunctionCallStatementAST& node)
{
	m_expressionCodegen.Visit(node.GetCall());
	// TODO: produce warning about unused function result
}

Codegen::Codegen(CodegenContext& context)
	: m_context(context)
{
}

void Codegen::Generate(const ProgramAST& program)
{
	for (size_t i = 0; i < program.GetFunctionsCount(); ++i)
	{
		GenerateFunc(program.GetFunction(i));
	}
}

void Codegen::GenerateFunc(const FunctionAST& func)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::Module& llvmModule = utils.GetModule();

	const std::string& name = func.GetIdentifier().GetName();

	// Задаем возвращаемый тип
	llvm::Type* returnType = ToLLVMType(func.GetReturnType(), llvmContext);

	// Задаем типы аргументов функции
	std::vector<llvm::Type*> argumentTypes;
	argumentTypes.reserve(func.GetParams().size());
	for (const FunctionAST::Param& param : func.GetParams())
	{
		argumentTypes.push_back(ToLLVMType(param.second, llvmContext));
	}

	// Создаем прототип и саму функцию
	llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argumentTypes, false);
	llvm::Function* llvmFunc = llvm::Function::Create(
		funcType, llvm::Function::ExternalLinkage, name, &llvmModule);

	// Задаем имена аргументов функции, добавляем переменные в контекст
	ContextScopeHelper scopedContext(m_context);

	// Создаем базовый блок для вставки инструкции функции
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvmContext, name + "_entry", llvmFunc);
	builder.SetInsertPoint(bb);

	size_t index = 0;
	for (llvm::Argument& argument : llvmFunc->args())
	{
		assert(index < func.GetParams().size());
		const FunctionAST::Param& param = func.GetParams()[index];
		argument.setName(func.GetParams()[index].first);

		llvm::AllocaInst* variable = CreateLocalVariable(
			llvmFunc, ToLLVMType(param.second, llvmContext), param.first + "Ptr");
		m_context.Define(param.first, variable);
		builder.CreateStore(&argument, variable);

		++index;
	}

	// Генерируем код инструкции функции (может быть композитной)
	StatementCodegen statementCodegen(m_context);
	statementCodegen.Visit(func.GetStatement());

	for (llvm::BasicBlock& basicBlock : llvmFunc->getBasicBlockList())
	{
		if (!basicBlock.getTerminator())
		{
			throw std::runtime_error("every path must have return statement in function '" + name + "'");
		}
	}

	std::string output;
	llvm::raw_string_ostream out(output);

	if (llvm::verifyFunction(*llvmFunc, &out))
	{
		llvmFunc->eraseFromParent();
		throw std::runtime_error(out.str());
	}

	m_context.AddFunction(func.GetIdentifier().GetName(), llvmFunc);
}
