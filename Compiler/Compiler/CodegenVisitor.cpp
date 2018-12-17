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
	if (type->isPointerTy())
	{
		if (type->getPointerElementType()->isIntegerTy())
		{
			if (type->getPointerElementType()->getIntegerBitWidth() == 32)
			{
				return ExpressionTypeAST::Int;
			}
			else if (type->getPointerElementType()->getIntegerBitWidth() == 1)
			{
				return ExpressionTypeAST::Bool;
			}
		}
		if (type->getPointerElementType()->isDoubleTy())
		{
			return ExpressionTypeAST::Float;
		}
	}
	else
	{
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

llvm::Value* ConvertToBooleanValue(llvm::Value* value, CodegenUtils& utils)
{
	ExpressionTypeAST typeAST = ConvertToExpressionTypeAST(value->getType());
	switch (typeAST)
	{
	case ExpressionTypeAST::Int:
		return utils.builder.CreateICmpEQ(
			value, llvm::ConstantInt::get(llvm::Type::getInt32Ty(utils.context), uint64_t(1)), "iboolcast");
	case ExpressionTypeAST::Float:
		return utils.builder.CreateNot(
			utils.builder.CreateFCmpOEQ(
				value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(utils.context), 0.0), "fboolcast"),
			"nottmp");
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
	const std::vector<FunctionAST::Param> &params,
	llvm::LLVMContext& context)
{
	std::vector<llvm::Type*> typesLLVM;
	typesLLVM.reserve(params.size());

	for (const FunctionAST::Param& param : params)
	{
		const ExpressionTypeAST& typeAST = param.second;
		typesLLVM.push_back(ConvertToTypeLLVM(typeAST, context));
	}

	return typesLLVM;
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

llvm::Value* EmitDefaultValue(ExpressionTypeAST type, CodegenUtils& utils)
{
	switch (type)
	{
	case ExpressionTypeAST::Int:
		return llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(utils.context),
			llvm::APInt(32, uint64_t(0), true));
	case ExpressionTypeAST::Float:
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(utils.context), 0.0);
	case ExpressionTypeAST::Bool:
	case ExpressionTypeAST::String:
		throw std::logic_error("code generation for bool and string is not implemented yet");
	default:
		throw std::logic_error("can't emit code for undefined ast expression type");
	}
}

bool Cast(GeneratedExpression& generated, ExpressionTypeAST to, CodegenUtils& utils)
{
	assert(generated.type != to);
	if (!Convertible(generated.type, to))
	{
		return false;
	}

	if (generated.type == ExpressionTypeAST::Int)
	{
		if (to == ExpressionTypeAST::Float)
		{
			generated.value = utils.builder.CreateSIToFP(
				generated.value, llvm::Type::getDoubleTy(utils.context), "casttmp");
			generated.type = ExpressionTypeAST::Float;
			return true;
		}
	}
	if (generated.type == ExpressionTypeAST::Float)
	{
		if (to == ExpressionTypeAST::Int)
		{
			generated.value = utils.builder.CreateFPToSI(
				generated.value, llvm::Type::getInt32Ty(utils.context), "casttmp");
			generated.type = ExpressionTypeAST::Int;
			return true;
		}
	}
	return false;
}

bool CastToMatchBinaryExpression(
	GeneratedExpression& left,
	GeneratedExpression& right,
	BinaryExpressionAST::Operator op,
	CodegenUtils& utils)
{
	assert(left.type != right.type);
	(void)op;

	if (left.type == ExpressionTypeAST::Float && right.type == ExpressionTypeAST::Int)
	{
		return Cast(right, ExpressionTypeAST::Float, utils);
	}
	if (left.type == ExpressionTypeAST::Int && right.type == ExpressionTypeAST::Float)
	{
		return Cast(left, ExpressionTypeAST::Float, utils);
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

GeneratedExpression ExpressionCodegen::Visit(const IExpressionAST& node)
{
	node.Accept(*this);
	if (!m_stack.empty())
	{
		GeneratedExpression value = m_stack.back();
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

	GeneratedExpression right = m_stack.back();
	m_stack.pop_back();

	GeneratedExpression left = m_stack.back();
	m_stack.pop_back();

	if (node.GetOperator() == BinaryExpressionAST::Mod &&
		(left.type != ExpressionTypeAST::Int || right.type != ExpressionTypeAST::Int))
	{
		throw std::runtime_error("modulo operator expects integers on both sides of an expression");
	}

	if (right.type != left.type)
	{
		if (!CastToMatchBinaryExpression(left, right, node.GetOperator(), utils))
		{
			auto fmt = boost::format("can't perform operator '%1%' on operands with types '%2%' and '%3%'")
				% ToString(node.GetOperator())
				% ToString(left.type)
				% ToString(right.type);
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(left.type == right.type);
	const bool isFloat = left.type == ExpressionTypeAST::Float;

	llvm::Value *value = nullptr;

	switch (node.GetOperator())
	{
	case BinaryExpressionAST::Plus:
		value = !isFloat ?
			utils.builder.CreateAdd(left.value, right.value, "addtmp") :
			utils.builder.CreateFAdd(left.value, right.value, "faddtmp");
		break;
	case BinaryExpressionAST::Minus:
		value = !isFloat ?
			utils.builder.CreateSub(left.value, right.value, "subtmp") :
			utils.builder.CreateFSub(left.value, right.value, "fsubtmp");
		break;
	case BinaryExpressionAST::Mul:
		value = !isFloat ?
			utils.builder.CreateMul(left.value, right.value, "multmp") :
			utils.builder.CreateFMul(left.value, right.value, "fmultmp");
		break;
	case BinaryExpressionAST::Div:
		value = !isFloat ?
			utils.builder.CreateSDiv(left.value, right.value, "fdivtmp") :
			utils.builder.CreateFDiv(left.value, right.value, "divtmp");
		break;
	case BinaryExpressionAST::Mod:
		assert(!isFloat);
		value = utils.builder.CreateSRem(left.value, right.value, "modtmp");
		break;
	default:
		throw std::logic_error("can't generate code for undefined binary operator");
	}

	m_stack.push_back({ value, (isFloat ? ExpressionTypeAST::Float : ExpressionTypeAST::Int) });
}

void ExpressionCodegen::Visit(const LiteralConstantAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	const LiteralConstantAST::Value& literal = node.GetValue();

	if (literal.type() == typeid(int))
	{
		const int number = boost::get<int>(literal);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(utils.context), number);
		m_stack.push_back({ value, ExpressionTypeAST::Int });
	}
	else if (literal.type() == typeid(double))
	{
		const double number = boost::get<double>(literal);
		llvm::Value* value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(utils.context), number);
		m_stack.push_back({ value, ExpressionTypeAST::Float });
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

	GeneratedExpression generated = m_stack.back();
	m_stack.pop_back();

	if (generated.type == ExpressionTypeAST::Int)
	{
		m_stack.push_back({ utils.builder.CreateNeg(generated.value, "negtmp"), ExpressionTypeAST::Int });
	}
	else if (generated.type == ExpressionTypeAST::Float)
	{
		m_stack.push_back({ utils.builder.CreateFNeg(generated.value, "fnegtmp"), ExpressionTypeAST::Float });
	}
	else
	{
		assert(false);
		throw std::logic_error("can't codegen for unary operator on undefined literal type");
	}
}

void ExpressionCodegen::Visit(const IdentifierAST& node)
{
	const std::string& name = node.GetName();
	llvm::AllocaInst* variable = m_context.GetVariable(name);

	if (!variable)
	{
		throw std::runtime_error("variable '" + name + "' is not defined");
	}

	llvm::Value* value = m_context.GetUtils().builder.CreateLoad(variable, name + "Value");
	m_stack.push_back({ value, ConvertToExpressionTypeAST(value->getType()) });
}

void ExpressionCodegen::Visit(const FunctionCallExprAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::Function* func = m_context.GetFunction(node.GetName());
	if (!func)
	{
		throw std::runtime_error("function '" + node.GetName() + "' is undefined");
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
		GeneratedExpression generatedParam = Visit(node.GetParam(index));
		ExpressionTypeAST formalParamType = ConvertToExpressionTypeAST(arg.getType());

		if (generatedParam.type != formalParamType)
		{
			bool casted = Cast(generatedParam, formalParamType, utils);
			if (!casted)
			{
				auto fmt = boost::format("function '%1%' expects '%2%' as parameter, '%3%' given (can't cast)")
					% func->getName().str()
					% ToString(formalParamType)
					% ToString(generatedParam.type);
				throw std::runtime_error(fmt.str());
			}
		}

		assert(generatedParam.type == formalParamType);
		args.push_back(generatedParam.value);
		++index;
	}

	llvm::Value* value = utils.builder.CreateCall(func, args, "calltmp");
	m_stack.push_back({ value, ConvertToExpressionTypeAST(func->getReturnType()) });
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
	const std::string& name = node.GetIdentifier().GetName();

	if (m_context.GetVariable(name))
	{
		throw std::runtime_error("variable '" + name + "' is already defined");
	}

	// Создаем переменную
	llvm::Type* type = ConvertToTypeLLVM(node.GetType(), utils.context);
	llvm::AllocaInst* variable = CreateLocalVariable(utils.builder.GetInsertBlock()->getParent(), type, name + "Ptr");

	// Устанавливаем переменной значение по умолчанию
	llvm::Value* defaultValue = EmitDefaultValue(node.GetType(), utils);
	utils.builder.CreateStore(defaultValue, variable);

	// Сохраняем переменную в контекст
	m_context.Define(name, variable);

	const IExpressionAST* expression = node.GetExpression();
	if (!expression)
	{
		return;
	}

	GeneratedExpression generated = m_expressionCodegen.Visit(*expression);
	if (generated.type != node.GetType())
	{
		if (!Cast(generated, node.GetType(), m_context.GetUtils()))
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
				% ToString(generated.type)
				% name
				% ToString(node.GetType());
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(generated.value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
	utils.builder.CreateStore(generated.value, variable);
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	const std::string& name = node.GetIdentifier().GetName();
	llvm::AllocaInst* variable = m_context.GetVariable(name);
	ExpressionTypeAST variableTypeAST = ConvertToExpressionTypeAST(variable->getType());

	if (!variable)
	{
		throw std::runtime_error("can't assign because variable '" + name + "' is not defined");
	}

	GeneratedExpression generated = m_expressionCodegen.Visit(node.GetExpr());
	if (generated.type != variableTypeAST)
	{
		if (!Cast(generated, variableTypeAST, m_context.GetUtils()))
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
				% ToString(generated.type)
				% name
				% ToString(variableTypeAST);
			throw std::runtime_error(fmt.str());
		}
		// TODO: produce warning here
	}

	assert(generated.value->getType()->getTypeID() == variable->getType()->getPointerElementType()->getTypeID());
	utils.builder.CreateStore(generated.value, variable);
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::Function* func = utils.builder.GetInsertBlock()->getParent();
	ExpressionTypeAST funcReturnType = ConvertToExpressionTypeAST(func->getReturnType());

	GeneratedExpression generated = m_expressionCodegen.Visit(node.GetExpr());
	if (generated.type != funcReturnType)
	{
		bool casted = Cast(generated, funcReturnType, utils);
		if (!casted)
		{
			throw std::runtime_error("returning expression must be at least convertible to function return type");
		}
	}
	assert(generated.type == funcReturnType);
	utils.builder.CreateRet(generated.value);
}

void StatementCodegen::Visit(const IfStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.builder;
	llvm::LLVMContext& context = utils.context;

	llvm::Function* func = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(context, "then", func);
	llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(context, "else", func);
	llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(context, "continue", func);

	GeneratedExpression generated = m_expressionCodegen.Visit(node.GetExpr());
	generated.value = ConvertToBooleanValue(generated.value, utils);
	builder.CreateCondBr(generated.value, thenBlock, elseBlock);

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
	for (size_t i = 0; i < node.GetCount(); ++i)
	{
		node.GetStatement(i).Accept(*this);
		if (m_context.GetUtils().builder.GetInsertBlock()->getTerminator())
		{
			break;
		}
		// TODO: produce warning about unreachable code
	}
}

void StatementCodegen::Visit(const PrintAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	GeneratedExpression generated = m_expressionCodegen.Visit(node.GetExpression());
	if (generated.type != ExpressionTypeAST::Int &&
		generated.type != ExpressionTypeAST::Float)
	{
		throw std::runtime_error("strings and booleans can't be printed out yet");
	}

	llvm::Function* printf = m_context.GetPrintf();

	const std::string& fmt = generated.value->getType()->isDoubleTy() ? "%f\n" : "%d\n";
	std::vector<llvm::Value*> args = { EmitStringLiteral(utils.context, utils.module, fmt), generated.value };

	utils.builder.CreateCall(printf, args);
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
	const std::string& name = func.GetIdentifier().GetName();

	// Задаем возвращаемый тип и типы аргументов функции
	llvm::Type* returnType = ConvertToTypeLLVM(func.GetReturnType(), utils.context);
	std::vector<llvm::Type*> argumentTypes = CreateFunctionArgumentTypes(func.GetParams(), utils.context);

	// Создаем прототип и саму функцию
	llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argumentTypes, false);
	llvm::Function* llvmFunc = llvm::Function::Create(
		funcType, llvm::Function::ExternalLinkage, func.GetIdentifier().GetName(), &utils.module);

	// Задаем имена аргументов функции, добавляем переменные в контекст
	ContextScopeHelper scopedContext(m_context);

	// Создаем базовый блок для вставки инструкции функции
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(utils.context, name + "_entry", llvmFunc);
	utils.builder.SetInsertPoint(bb);

	size_t index = 0;
	for (llvm::Argument& argument : llvmFunc->args())
	{
		assert(index < func.GetParams().size());
		const FunctionAST::Param& param = func.GetParams()[index];
		argument.setName(func.GetParams()[index].first);

		llvm::AllocaInst* variable = CreateLocalVariable(llvmFunc, ConvertToTypeLLVM(param.second, utils.context), param.first + "Ptr");
		m_context.Define(param.first, variable);
		utils.builder.CreateStore(&argument, variable);

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
			utils.builder.SetInsertPoint(block);
			utils.builder.CreateBr(next);
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
