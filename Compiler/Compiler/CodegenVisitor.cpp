#include "stdafx.h"
#include "CodegenVisitor.h"

namespace
{
llvm::Type* CreateLLVMType(ASTExpressionType type, CodegenUtils& utils)
{
	switch (type)
	{
	case ASTExpressionType::Int:
		return llvm::Type::getInt32Ty(utils.context);
	case ASTExpressionType::Float:
		return llvm::Type::getDoubleTy(utils.context);
	default:
		throw std::logic_error("booleans and strings are not supported yet");
	}
}

std::vector<llvm::Type*> CreateFunctionArgumentTypes(const std::vector<FunctionAST::Parameter>& params, CodegenUtils& utils)
{
	std::vector<llvm::Type*> types;
	for (const auto& param : params)
	{
		types.push_back(CreateLLVMType(param.second, utils));
	}
	return types;
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

	std::vector<llvm::Value*> args;
	for (size_t i = 0; i < node.GetParamsCount(); ++i)
	{
		args.push_back(Visit(node.GetParam(i)));
	}

	llvm::Value* value = utils.builder.CreateCall(func, args, "calltmp");
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
	m_context.Define(
		node.GetIdentifier().GetName(),
		EmitDefaultValue(node.GetType(), m_context.GetUtils())
	);

	if (const IExpressionAST* expression = node.GetExpression())
	{
		llvm::Value* value = m_expressionCodegen.Visit(*expression);
		m_context.Assign(node.GetIdentifier().GetName(), value);
	}
}

void StatementCodegen::Visit(const AssignStatementAST& node)
{
	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	m_context.Assign(node.GetIdentifier().GetName(), value);
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();

	llvm::Value* value = m_expressionCodegen.Visit(node.GetExpr());
	utils.builder.CreateRet(value);
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
	ContextScopeHelper scopedContext(m_context);
	for (size_t i = 0; i < node.GetCount(); ++i)
	{
		node.GetStatement(i).Accept(*this);
		if (m_context.GetUtils().builder.GetInsertBlock()->getTerminator())
		{
			break;
		}
	}
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

void StatementCodegen::Visit(const FunctionCallStatementAST& node)
{
	m_expressionCodegen.Visit(node.GetCall());
}

Codegen::Codegen(CodegenContext& context)
	: m_context(context)
	, m_statementCodegen(context)
{
}

void Codegen::Generate(const ProgramAST& program)
{
	for (size_t i = 0; i < program.GetFunctionsCount(); ++i)
	{
		Generate(program.GetFunction(i));
	}
}

void Codegen::Generate(const FunctionAST& func)
{
	CodegenUtils& utils = m_context.GetUtils();

	// Задаем возвращаемый тип и типы аргументов функции
	llvm::Type* returnType = CreateLLVMType(func.GetReturnType(), utils);
	std::vector<llvm::Type*> argumentTypes = CreateFunctionArgumentTypes(func.GetParams(), utils);

	// Создаем прототип и саму функцию
	llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argumentTypes, false);
	llvm::Function* llvmFunc = llvm::Function::Create(
		funcType, llvm::Function::ExternalLinkage, func.GetIdentifier().GetName(), &utils.module);

	// Задаем имена аргументов функции, добавляем переменные в контекст
	ContextScopeHelper scopedContext(m_context);

	size_t index = 0;
	const std::vector<FunctionAST::Parameter> &params = func.GetParams();

	for (llvm::Argument& argument : llvmFunc->args())
	{
		assert(index < params.size());
		argument.setName(params[index].first);
		m_context.Define(params[index].first, &argument);
		++index;
	}

	// Создаем базовый блок для вставки инструкции функции
	llvm::BasicBlock* bb = llvm::BasicBlock::Create(utils.context, func.GetIdentifier().GetName() + "_entry", llvmFunc);
	utils.builder.SetInsertPoint(bb);

	// Генерируем код инструкции функции (может быть композитной)
	m_statementCodegen.Visit(func.GetStatement());

	std::string output;
	llvm::raw_string_ostream out(output);

	if (llvm::verifyFunction(*llvmFunc, &out))
	{
		llvmFunc->eraseFromParent();
		throw std::runtime_error(out.str());
	}

	m_context.AddFunction(func.GetIdentifier().GetName(), llvmFunc);
}
