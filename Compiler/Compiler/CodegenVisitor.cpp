#include "stdafx.h"
#include "CodegenVisitor.h"

namespace
{
std::string ToString(BinaryExpressionAST::Operator operation)
{
	switch (operation)
	{
	case BinaryExpressionAST::Plus:
		return "+";
	case BinaryExpressionAST::Minus:
		return "-";
	case BinaryExpressionAST::Mul:
		return "*";
	case BinaryExpressionAST::Div:
		return "/";
	case BinaryExpressionAST::Mod:
		return "%";
	}
	throw std::logic_error("can't cast undefined binary operator to string");
}

ExpressionTypeAST ConvertToExpressionTypeAST(llvm::Type* type)
{
	assert(!type->isPointerTy());
	if (type->isIntegerTy())
	{
		if (type->getIntegerBitWidth() == 32)
		{
			return ExpressionTypeAST::Int;
		}
		else if (type->getIntegerBitWidth() == 1)
		{
			return ExpressionTypeAST::Bool;
		}
	}
	if (type->isDoubleTy())
	{
		return ExpressionTypeAST::Float;
	}
	throw std::invalid_argument("can't convert passed llvm type to expression type ast");
}

llvm::Type* ConvertToTypeLLVM(ExpressionTypeAST type, llvm::LLVMContext& context)
{
	switch (type)
	{
	case ExpressionTypeAST::Int:
		return llvm::Type::getInt32Ty(context);
	case ExpressionTypeAST::Float:
		return llvm::Type::getDoubleTy(context);
	case ExpressionTypeAST::Bool:
		return llvm::Type::getInt1Ty(context);
	default:
		throw std::logic_error("can't convert string ast literal to llvm type");
	}
}

llvm::Value* ConvertToBooleanValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	ExpressionTypeAST typeAST = ConvertToExpressionTypeAST(value->getType());
	switch (typeAST)
	{
	case ExpressionTypeAST::Int:
		return builder.CreateICmpEQ(value, llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(llvmContext), uint64_t(1)), "iboolcast");
	case ExpressionTypeAST::Float:
		return builder.CreateNot(builder.CreateFCmpOEQ(
				value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0), "fboolcast"), "nottmp");
	case ExpressionTypeAST::Bool:
		return value;
	case ExpressionTypeAST::String:
		throw std::runtime_error("can't cast string to bool");
	default:
		assert(false);
		throw std::logic_error("ConvertToBooleanValue() - undefined ast expression type");
	}
}

std::vector<llvm::Type*> CreateFunctionArgumentTypes(
	const std::vector<FunctionAST::Param>& params,
	llvm::LLVMContext& llvmContext)
{
	std::vector<llvm::Type*> typesLLVM;
	typesLLVM.reserve(params.size());

	for (const FunctionAST::Param& param : params)
	{
		const ExpressionTypeAST& typeAST = param.second;
		typesLLVM.push_back(ConvertToTypeLLVM(typeAST, llvmContext));
	}

	return typesLLVM;
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

llvm::Value* EmitDefaultValue(ExpressionTypeAST type, llvm::LLVMContext& llvmContext)
{
	switch (type)
	{
	case ExpressionTypeAST::Int:
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), llvm::APInt(32, uint64_t(0), true));
	case ExpressionTypeAST::Float:
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0);
	case ExpressionTypeAST::Bool:
	case ExpressionTypeAST::String:
		throw std::logic_error("code generation for bool and string is not implemented yet");
	default:
		assert(false);
		throw std::logic_error("can't emit code for undefined ast expression type");
	}
}

bool CastValue(llvm::Value*& value, ExpressionTypeAST castToThatType, CodegenUtils& utils)
{
	const ExpressionTypeAST currentType = ConvertToExpressionTypeAST(value->getType());
	if (!Convertible(currentType, castToThatType))
	{
		return false;
	}

	if (currentType == ExpressionTypeAST::Int)
	{
		if (castToThatType == ExpressionTypeAST::Float)
		{
			value = utils.GetBuilder().CreateSIToFP(value, llvm::Type::getDoubleTy(utils.GetLLVMContext()), "casttmp");
			return true;
		}
	}
	if (currentType == ExpressionTypeAST::Float)
	{
		if (castToThatType == ExpressionTypeAST::Int)
		{
			value = utils.GetBuilder().CreateFPToSI(value, llvm::Type::getInt32Ty(utils.GetLLVMContext()), "casttmp");
			return true;
		}
	}
	return false;
}

bool CastToMatchBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator op,
	CodegenUtils& utils)
{
	const ExpressionTypeAST leftType = ConvertToExpressionTypeAST(left->getType());
	const ExpressionTypeAST rightType = ConvertToExpressionTypeAST(right->getType());

	if (leftType == ExpressionTypeAST::Float && rightType == ExpressionTypeAST::Int)
	{
		return CastValue(right, ExpressionTypeAST::Float, utils);
	}
	if (leftType == ExpressionTypeAST::Int && rightType == ExpressionTypeAST::Float)
	{
		return CastValue(left, ExpressionTypeAST::Float, utils);
	}
	return false;
}

llvm::AllocaInst* CreateLocalVariable(llvm::Function* func, llvm::Type* type, const std::string& name)
{
	llvm::BasicBlock& block = func->getEntryBlock();
	llvm::IRBuilder<> builder(&block, block.begin());
	return builder.CreateAlloca(type, nullptr, name);
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

	llvm::Value* left = Visit(node.GetLeft());
	llvm::Value* right = Visit(node.GetRight());

	const ExpressionTypeAST leftType = ConvertToExpressionTypeAST(left->getType());
	const ExpressionTypeAST rightType = ConvertToExpressionTypeAST(right->getType());

	if (leftType != rightType)
	{
		if (!CastToMatchBinaryExpression(left, right, node.GetOperator(), utils))
		{
			auto fmt = boost::format("can't perform operator '%1%' on operands with types '%2%' and '%3%'")
				% ToString(node.GetOperator())
				% ToString(leftType)
				% ToString(rightType);
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(ConvertToExpressionTypeAST(left->getType()) == ConvertToExpressionTypeAST(right->getType()));
	const bool isFloat = leftType == ExpressionTypeAST::Float;

	llvm::Value *value = nullptr;

	switch (node.GetOperator())
	{
	case BinaryExpressionAST::Plus:
		value = !isFloat ?
			builder.CreateAdd(left, right, "addtmp") :
			builder.CreateFAdd(left, right, "faddtmp");
		break;
	case BinaryExpressionAST::Minus:
		value = !isFloat ?
			builder.CreateSub(left, right, "subtmp") :
			builder.CreateFSub(left, right, "fsubtmp");
		break;
	case BinaryExpressionAST::Mul:
		value = !isFloat ?
			builder.CreateMul(left, right, "multmp") :
			builder.CreateFMul(left, right, "fmultmp");
		break;
	case BinaryExpressionAST::Div:
		value = !isFloat ?
			builder.CreateSDiv(left, right, "fdivtmp") :
			builder.CreateFDiv(left, right, "divtmp");
		break;
	case BinaryExpressionAST::Mod:
		assert(!isFloat);
		value = builder.CreateSRem(left, right, "modtmp");
		break;
	default:
		throw std::logic_error("can't generate code for undefined binary operator");
	}

	m_stack.push_back(value);
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
	const ExpressionTypeAST type = ConvertToExpressionTypeAST(value->getType());

	if (type == ExpressionTypeAST::Int)
	{
		m_stack.push_back(builder.CreateNeg(value, "negtmp"));
	}
	else if (type == ExpressionTypeAST::Float)
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
	std::vector<llvm::Value*> args;

	for (llvm::Argument& arg : func->args())
	{
		llvm::Value* value = Visit(node.GetParam(index));
		const ExpressionTypeAST valueType = ConvertToExpressionTypeAST(value->getType());
		const ExpressionTypeAST argumentType = ConvertToExpressionTypeAST(arg.getType());

		if (valueType != argumentType)
		{
			bool casted = CastValue(value, argumentType, utils);
			if (!casted)
			{
				auto fmt = boost::format("function '%1%' expects '%2%' as parameter, '%3%' given (can't cast)")
					% func->getName().str()
					% ToString(argumentType)
					% ToString(valueType);
				throw std::runtime_error(fmt.str());
			}
		}

		assert(ConvertToExpressionTypeAST(value->getType()) == ConvertToExpressionTypeAST(arg.getType()));
		args.push_back(value);
		++index;
	}

	llvm::Value* value = builder.CreateCall(func, args, "calltmp");
	m_stack.push_back(value);
}

// Statement codegen visitor
StatementCodegen::StatementCodegen(CodegenContext& context, std::vector<llvm::BasicBlock*> & continueBlocks)
	: m_context(context)
	, m_expressionCodegen(context)
	, m_continueBlocks(continueBlocks)
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
	llvm::Type* type = ConvertToTypeLLVM(node.GetType(), llvmContext);
	llvm::Function* func = builder.GetInsertBlock()->getParent();
	llvm::AllocaInst* variable = CreateLocalVariable(func, type, name + "Ptr");

	// Устанавливаем переменной значение по умолчанию
	llvm::Value* defaultValue = EmitDefaultValue(node.GetType(), llvmContext);
	builder.CreateStore(defaultValue, variable);

	// Сохраняем переменную в контекст
	m_context.Define(name, variable);

	// Обработка опционального блока присваивания
	const IExpressionAST* expression = node.GetExpression();
	if (!expression)
	{
		return;
	}

	llvm::Value* value = m_expressionCodegen.Visit(*expression);
	if (ConvertToExpressionTypeAST(value->getType()) != node.GetType())
	{
		if (!CastValue(value, node.GetType(), m_context.GetUtils()))
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
				% ToString(ConvertToExpressionTypeAST(value->getType()))
				% name
				% ToString(node.GetType());
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
	builder.CreateStore(value, variable);
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	const std::string& name = node.GetIdentifier().GetName();
	llvm::AllocaInst* variable = m_context.GetVariable(name);
	if (!variable)
	{
		throw std::runtime_error("can't assign because variable '" + name + "' is not defined");
	}

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	if (ConvertToExpressionTypeAST(value->getType()) != ConvertToExpressionTypeAST(variable->getType()->getPointerElementType()))
	{
		if (!CastValue(value, ConvertToExpressionTypeAST(variable->getType()->getPointerElementType()), m_context.GetUtils()))
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
				% ToString(ConvertToExpressionTypeAST(value->getType()))
				% name
				% ToString(ConvertToExpressionTypeAST(variable->getType()->getPointerElementType()));
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
	builder.CreateStore(value, variable);
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	llvm::Function* func = builder.GetInsertBlock()->getParent();
	ExpressionTypeAST funcReturnType = ConvertToExpressionTypeAST(func->getReturnType());

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	if (ConvertToExpressionTypeAST(value->getType()) != funcReturnType)
	{
		bool casted = CastValue(value, funcReturnType, utils);
		if (!casted)
		{
			throw std::runtime_error("returning expression must be at least convertible to function return type");
		}
	}

	assert(ConvertToExpressionTypeAST(value->getType()) == funcReturnType);
	builder.CreateRet(value);
}

void StatementCodegen::Visit(const IfStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(llvmContext, "then", func);
	llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(llvmContext, "else", func);
	llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(llvmContext, "continue", func);

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	value = ConvertToBooleanValue(value, llvmContext, builder);
	builder.CreateCondBr(value, thenBlock, elseBlock);

	builder.SetInsertPoint(thenBlock);
	Visit(node.GetThenStmt());
	if (!thenBlock->getTerminator())
	{
		builder.CreateBr(continueBlock);
	}

	builder.SetInsertPoint(elseBlock);
	if (node.GetElseStmt())
	{
		Visit(*node.GetElseStmt());
	}
	if (!elseBlock->getTerminator())
	{
		builder.CreateBr(continueBlock);
	}

	builder.SetInsertPoint(continueBlock);
	m_continueBlocks.push_back(continueBlock);
}

void StatementCodegen::Visit(const WhileStatementAST& node)
{
	(void)node;
	throw std::logic_error("code generation for while statement is not implemented");
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
	llvm::Module& llvmModule = utils.GetModule();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpression());
	if (!value->getType()->isIntegerTy() && !value->getType()->isDoubleTy())
	{
		throw std::runtime_error("strings and booleans can't be printed out yet");
	}

	llvm::Function* printf = m_context.GetPrintf();

	const std::string& fmt = value->getType()->isDoubleTy() ? "%f\n" : "%d\n";
	std::vector<llvm::Value*> args = { CreateGlobalStringLiteral(llvmContext, llvmModule, fmt), value };

	builder.CreateCall(printf, args);
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
	llvm::Module& llvmModule = utils.GetModule();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	const std::string& name = func.GetIdentifier().GetName();

	// Задаем возвращаемый тип и типы аргументов функции
	llvm::Type* returnType = ConvertToTypeLLVM(func.GetReturnType(), llvmContext);
	std::vector<llvm::Type*> argumentTypes = CreateFunctionArgumentTypes(func.GetParams(), llvmContext);

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
			llvmFunc, ConvertToTypeLLVM(param.second, llvmContext), param.first + "Ptr");
		m_context.Define(param.first, variable);
		builder.CreateStore(&argument, variable);

		++index;
	}

	// Генерируем код инструкции функции (может быть композитной)
	std::vector<llvm::BasicBlock*> continueBlocks;
	StatementCodegen statementCodegen(m_context, continueBlocks);
	statementCodegen.Visit(func.GetStatement());

	// Связываем блоки continue условных инструкции
	for (auto it = continueBlocks.begin(); it != continueBlocks.end(); ++it)
	{
		llvm::BasicBlock* block = *it;
		if (!block->getTerminator() && std::next(it) != continueBlocks.end())
		{
			llvm::BasicBlock* next = *std::next(it);
			builder.SetInsertPoint(block);
			builder.CreateBr(next);
		};
	}

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
