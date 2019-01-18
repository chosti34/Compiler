#include "stdafx.h"
#include "CodegenVisitor.h"

namespace
{
// Function forward declaration
ExpressionType ToExpressionType(llvm::Type* type);

// My compiler supports 32 bit integers and booleans
ExpressionType ToExpressionTypeFromInt(llvm::Type* type)
{
	if (type->getIntegerBitWidth() == 32)
	{
		return { ExpressionType::Int, 0 };
	}
	if (type->getIntegerBitWidth() == 1)
	{
		return { ExpressionType::Bool, 0 };
	}
	throw std::logic_error("llvm::Type have unsupported integer bit width");
}

// Expression type can be string or array, depending on passed llvm::Type
ExpressionType ToExpressionTypeFromPtr(llvm::Type* type)
{
	assert(type->isPointerTy());
	llvm::Type* ptrElementType = type->getPointerElementType();

	if (ptrElementType->getTypeID() == llvm::Type::IntegerTyID && ptrElementType->getIntegerBitWidth() == 8)
	{
		return { ExpressionType::String, 0 };
	}
	if (ptrElementType->getTypeID() == llvm::Type::IntegerTyID && ptrElementType->getIntegerBitWidth() == 32)
	{
		return { ExpressionType::Int, 1 };
	}
	if (ptrElementType->getTypeID() == llvm::Type::IntegerTyID && ptrElementType->getIntegerBitWidth() == 1)
	{
		return { ExpressionType::Bool, 1 };
	}
	if (ptrElementType->getTypeID() == llvm::Type::DoubleTyID)
	{
		return { ExpressionType::Float, 1 };
	}

	// Processing as multidimensional array
	if (ptrElementType->isPointerTy())
	{
		const ExpressionType typeOfPtrElement = ToExpressionTypeFromPtr(ptrElementType);
		return { typeOfPtrElement.value, typeOfPtrElement.nesting + 1 };
	}

	assert(false);
	throw std::logic_error("can't find proper array ExpressionType for llvm::Type* pointer type");
}

// Function performs cast from llvm::Type to ExpressionType
ExpressionType ToExpressionType(llvm::Type* type)
{
	if (type->isIntegerTy())
	{
		return ToExpressionTypeFromInt(type);
	}
	if (type->isDoubleTy())
	{
		return { ExpressionType::Float, 0 };
	}
	if (type->isPointerTy())
	{
		return ToExpressionTypeFromPtr(type);
	}
	throw std::logic_error("can't convert llvm::Type to ExpressionType");
}

// Function performs cast from ExpressionType to llvm::Type
llvm::Type* ToLLVMType(ExpressionType type, llvm::LLVMContext& context)
{
	if (type.nesting != 0)
	{
		ExpressionType typeOfStored = { type.value, type.nesting - 1 };
		llvm::Type* typeOfStoredLLVM = ToLLVMType(typeOfStored, context);
		return typeOfStoredLLVM->getPointerTo();
	}

	switch (type.value)
	{
	case ExpressionType::Int:
		return llvm::Type::getInt32Ty(context);
	case ExpressionType::Float:
		return llvm::Type::getDoubleTy(context);
	case ExpressionType::Bool:
		return llvm::Type::getInt1Ty(context);
	case ExpressionType::String:
		return llvm::Type::getInt8PtrTy(context);
	}

	throw std::logic_error("can't convert ExpressionType to llvm::Type");
}

llvm::Value* ConvertToIntegerValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());

	// Strings and arrays can't be casted to integer
	switch (type.value)
	{
	case ExpressionType::Int:
		return value;
	case ExpressionType::Float:
		return builder.CreateFPToUI(value, llvm::Type::getInt32Ty(llvmContext), "icasttmp");
	case ExpressionType::Bool:
		return builder.CreateIntCast(value, llvm::Type::getInt32Ty(llvmContext), false, "icasttmp");
	default:
		throw std::invalid_argument("can't cast array to integer");
	}
}

llvm::Value* ConvertToFloatValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());

	// Strings and arrays can't be casted to float
	switch (type.value)
	{
	case ExpressionType::Int:
	case ExpressionType::Bool:
		return builder.CreateUIToFP(value, llvm::Type::getDoubleTy(llvmContext), "fcasttmp");
	case ExpressionType::Float:
		return value;
	default:
		throw std::invalid_argument("can't cast array to integer");
	}
}

llvm::Value* ConvertToBooleanValue(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());

	// Strings and arrays can't be casted to bool
	switch (type.value)
	{
	case ExpressionType::Int:
		return builder.CreateNot(builder.CreateICmpEQ(
			value, llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), uint64_t(0)), "bcasttmp"));
	case ExpressionType::Float:
		return builder.CreateNot(builder.CreateFCmpOEQ(
			value, llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0), "fcmptmp"), "nottmp");
	case ExpressionType::Bool:
		return value;
	default:
		throw std::invalid_argument("can't cast array to bool");
	}
}

//? TODO: убрать выбросы исключений из вызываемых функций, чтобы
//?  функция возвращала только nullptr при неудаче
// Возвращает nullptr, либо бросает исключение, если нельзя
//  сгенерировать код преобразования значения в другой тип
llvm::Value* CastValue(
	llvm::Value* value,
	ExpressionType type,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	switch (type.value)
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
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
#ifdef _DEBUG
	const ExpressionType ltype = ToExpressionType(left->getType());
	const ExpressionType rtype = ToExpressionType(right->getType());
	assert(ltype.value == ExpressionType::Int && rtype.value == ExpressionType::Int);
#endif

	switch (operation)
	{
	case BinaryExpressionAST::Or:
		return builder.CreateOr(
			ConvertToBooleanValue(left, llvmContext, builder), 
			ConvertToBooleanValue(right, llvmContext, builder), "ortmp");
	case BinaryExpressionAST::And:
		return builder.CreateAnd(
			ConvertToBooleanValue(left, llvmContext, builder),
			ConvertToBooleanValue(right, llvmContext, builder), "andtmp");
	case BinaryExpressionAST::Equals:
		return builder.CreateICmpEQ(left, right, "eqtmp");
	case BinaryExpressionAST::NotEquals:
		return builder.CreateNot(builder.CreateICmpEQ(left, right, "eqtmp"), "nottmp");
	case BinaryExpressionAST::Less:
		return builder.CreateICmpSLT(left, right, "lttmp");
	case BinaryExpressionAST::LessOrEquals:
		return builder.CreateICmpSLE(left, right, "letmp");
	case BinaryExpressionAST::More:
		return builder.CreateICmpSGT(left, right, "gttmp");
	case BinaryExpressionAST::MoreOrEquals:
		return builder.CreateICmpSGE(left, right, "getmp");
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
	throw std::logic_error("undefined BinaryExpressionAST::Operator in CreateIntBinaryExpression");
}

llvm::Value* CreateFloatBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator operation,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
#ifdef _DEBUG
	const ExpressionType ltype = ToExpressionType(left->getType());
	const ExpressionType rtype = ToExpressionType(right->getType());
	assert(ltype.value == ExpressionType::Float && rtype.value == ExpressionType::Float);
#endif

	switch (operation)
	{
	case BinaryExpressionAST::Or:
		return builder.CreateOr(
			ConvertToBooleanValue(left, llvmContext, builder),
			ConvertToBooleanValue(right, llvmContext, builder), "ortmp");
	case BinaryExpressionAST::And:
		return builder.CreateAnd(
			ConvertToBooleanValue(left, llvmContext, builder),
			ConvertToBooleanValue(right, llvmContext, builder), "andtmp");
	case BinaryExpressionAST::Equals:
		return builder.CreateFCmpOEQ(left, right, "eqtmp");
	case BinaryExpressionAST::NotEquals:
		return builder.CreateNot(builder.CreateFCmpOEQ(left, right, "eqtmp"), "nottmp");
	case BinaryExpressionAST::Less:
		return builder.CreateFCmpOLT(left, right, "lttmp");
	case BinaryExpressionAST::LessOrEquals:
		return builder.CreateFCmpOLE(left, right, "letmp");
	case BinaryExpressionAST::More:
		return builder.CreateFCmpOGT(left, right, "gttmp");
	case BinaryExpressionAST::MoreOrEquals:
		return builder.CreateFCmpOGE(left, right, "getmp");
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
	throw std::logic_error("undefined BinaryExpressionAST::Operator in CreateFloatBinaryExpression");
}

llvm::Value* CreateBooleanBinaryExpression(
	llvm::Value* left,
	llvm::Value* right,
	BinaryExpressionAST::Operator operation,
	llvm::LLVMContext&,
	llvm::IRBuilder<> & builder)
{
#ifdef _DEBUG
	const ExpressionType ltype = ToExpressionType(left->getType());
	const ExpressionType rtype = ToExpressionType(right->getType());
	assert(ltype.value == ExpressionType::Bool && rtype.value == ExpressionType::Bool);
#endif

	switch (operation)
	{
	case BinaryExpressionAST::Or:
		return builder.CreateOr(left, right, "bortmp");
	case BinaryExpressionAST::And:
		return builder.CreateAnd(left, right, "bandtmp");
	case BinaryExpressionAST::Equals:
		return builder.CreateICmpEQ(left, right, "beqtmp");
	case BinaryExpressionAST::NotEquals:
		return builder.CreateICmpNE(left, right, "bneqtmp");
	case BinaryExpressionAST::Less:
		return builder.CreateICmpSLT(left, right, "blttmp");
	case BinaryExpressionAST::LessOrEquals:
		return builder.CreateICmpSLE(left, right, "bletmp");
	case BinaryExpressionAST::More:
		return builder.CreateICmpSGT(left, right, "bgttmp");
	case BinaryExpressionAST::MoreOrEquals:
		return builder.CreateICmpSGE(left, right, "bgetmp");
	case BinaryExpressionAST::Plus:
	case BinaryExpressionAST::Minus:
	case BinaryExpressionAST::Mul:
	case BinaryExpressionAST::Div:
	case BinaryExpressionAST::Mod:
		throw std::runtime_error("can't perform codegen for operator '" + ToString(operation) + "' on two booleans");
	}

	assert(false);
	throw std::logic_error("undefined BinaryExpressionAST::Operator in CreateBooleanBinaryExpression");
}

// Codegen arithmetic value negation
llvm::Value* CodegenNegativeValue(llvm::Value* value, llvm::IRBuilder<>& builder)
{
	const ExpressionType type = ToExpressionType(value->getType());
	switch (type.value)
	{
	case ExpressionType::Int:
		return builder.CreateNeg(value, "negtmp");
	case ExpressionType::Float:
		return builder.CreateFNeg(value, "fnegtmp");
	case ExpressionType::Bool:
		return builder.CreateNeg(value, "bnegtmp");
	default:
		throw std::runtime_error("can't codegen negative value of " + ToString(type) + " type");
	}
}

// Codegen boolean value negation
llvm::Value* CreateValueNegation(
	llvm::Value* value,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	return builder.CreateNot(ConvertToBooleanValue(value, llvmContext, builder));
}

llvm::Value* CreateDefaultValue(
	ExpressionType type,
	llvm::LLVMContext& llvmContext,
	llvm::IRBuilder<>& builder)
{
	if (type.nesting != 0)
	{
		const ExpressionType subtype = { type.value, type.nesting - 1 };
		llvm::Value* subvalue = CreateDefaultValue(subtype, llvmContext, builder);
		return llvm::Constant::getNullValue(subvalue->getType()->getPointerTo());
	}

	switch (type.value)
	{
	case ExpressionType::Int:
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), llvm::APInt(32, uint64_t(0), true));
	case ExpressionType::Float:
		return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), 0.0);
	case ExpressionType::Bool:
		return llvm::ConstantInt::get(llvm::Type::getInt1Ty(llvmContext), uint64_t(0));
	case ExpressionType::String:
	{
		llvm::Type* i8 = llvm::Type::getInt8Ty(llvmContext);
		llvm::ArrayType* arrayType = llvm::ArrayType::get(i8, 1);
		llvm::AllocaInst* allocaInst = builder.CreateAlloca(arrayType, nullptr, "str_alloc");

		// Создаем константый массив на стеке
		std::vector<llvm::Constant*> constants = { llvm::ConstantInt::get(llvm::Type::getInt8Ty(llvmContext), uint64_t(0)) };
		llvm::Constant* arr = llvm::ConstantArray::get(arrayType, constants);

		llvm::StoreInst* storeInst = builder.CreateStore(arr, allocaInst);
		(void)storeInst;

		return builder.CreateBitCast(allocaInst, llvm::Type::getInt8PtrTy(llvmContext), "str_to_i8_ptr");
	}
	default:
		assert(false);
		throw std::logic_error("can't codegen default value for undefined ExpressionType");
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

llvm::Value* ExpressionCodegen::GenerateFunctionCall(const FunctionCallExpressionAST& node)
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
			++index;
			continue;
		}

		assert(ToExpressionType(value->getType()) == ToExpressionType(arg.getType()));
		params.push_back(value);
		++index;
	}

	if (func->getReturnType()->getTypeID() == llvm::Type::VoidTyID)
	{
		builder.CreateCall(func, params);
		return nullptr;
	}

	return builder.CreateCall(func, params, "calltmp");
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
	//     обеих частей выражения в один тип. Далее генерировать код, в зависимости от оператора и от типа (используется этот вариант)

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

		switch (castType->value)
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

	switch (ToExpressionType(left->getType()).value)
	{
	case ExpressionType::Int:
		m_stack.push_back(CreateIntegerBinaryExpression(left, right, node.GetOperator(), llvmContext, builder));
		break;
	case ExpressionType::Float:
		m_stack.push_back(CreateFloatBinaryExpression(left, right, node.GetOperator(), llvmContext, builder));
		break;
	case ExpressionType::Bool:
		m_stack.push_back(CreateBooleanBinaryExpression(left, right, node.GetOperator(), llvmContext, builder));
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
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	const LiteralConstantAST::Value& constant = node.GetValue();

	// TODO: use boost::static_visitor
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
	else if (constant.type() == typeid(bool))
	{
		const bool boolean = boost::get<bool>(constant);
		llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt1Ty(llvmContext), uint64_t(boolean));
		m_stack.push_back(value);
	}
	else if (constant.type() == typeid(std::string))
	{
		const std::string& str = boost::get<std::string>(constant);
		llvm::Type* i8 = llvm::Type::getInt8Ty(llvmContext);
		llvm::Constant* constantString = llvm::ConstantDataArray::getString(llvmContext, str, true);
		llvm::ArrayType* arrayType = llvm::ArrayType::get(i8, str.length() + 1);

		llvm::AllocaInst* allocaInst = builder.CreateAlloca(arrayType,
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), uint64_t(str.length() + 1)), "str_alloc");
		llvm::StoreInst* storeInst = builder.CreateStore(constantString, allocaInst);
		(void)storeInst;

		m_stack.push_back(builder.CreateBitCast(allocaInst, llvm::Type::getInt8PtrTy(llvmContext), "str_to_i8_ptr"));
	}
	else if (constant.type() == typeid(std::vector<std::shared_ptr<IExpressionAST>>))
	{
		const auto& expressions = boost::get<std::vector<std::shared_ptr<IExpressionAST>>>(constant);
		if (expressions.empty())
		{
			throw std::runtime_error("can't create empty array");
		}

		std::vector<llvm::Value*> values(expressions.size());
		for (size_t i = 0; i < values.size(); ++i)
		{
			values[i] = Visit(*expressions[i]);
		}

		ExpressionType type = ToExpressionType(values[0]->getType());
		const bool hasSameType = std::all_of(values.begin() + 1, values.end(), [&type](llvm::Value* value) {
			return ToExpressionType(value->getType()) == type;
		});
		if (!hasSameType)
		{
			throw std::runtime_error("all array literal element must have same type");
		}

		llvm::ArrayType* arrayType = llvm::ArrayType::get(values[0]->getType(), expressions.size());
		llvm::AllocaInst* arrayValue = builder.CreateAlloca(
			arrayType,
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), uint64_t(expressions.size())),
			"arr_alloc");
		llvm::Value* arrayPtr = builder.CreateBitCast(arrayValue, values[0]->getType()->getPointerTo(), "bitcast");

		for (size_t i = 0; i < values.size(); ++i)
		{
			llvm::Value* ptr = builder.CreateGEP(
				arrayPtr,
				llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), uint64_t(i)),
				"element_ptr[" + std::to_string(i) + "]");
			builder.CreateStore(values[i], ptr);
		}

		m_stack.push_back(builder.CreateBitCast(arrayPtr, values[0]->getType()->getPointerTo(), "array_to_ptr"));
	}
	else
	{
		assert(false);
		throw std::logic_error("Visiting LiteralConstantAST - can't codegen for undefined literal constant type");
	}
}

void ExpressionCodegen::Visit(const UnaryAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Value* value = Visit(node.GetExpr());

	switch (node.GetOperator())
	{
	case UnaryAST::Plus:
		m_stack.push_back(value);
		break;
	case UnaryAST::Minus:
		m_stack.push_back(CodegenNegativeValue(value, builder));
		break;
	case UnaryAST::Negation:
		m_stack.push_back(CreateValueNegation(value, llvmContext, builder));
		break;
	default:
		assert(false);
		throw std::logic_error("Visit(UnaryAST): undefined unary operator");
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

void ExpressionCodegen::Visit(const FunctionCallExpressionAST& node)
{
	llvm::Value* returnValue = GenerateFunctionCall(node);
	if (!returnValue)
	{
		throw std::runtime_error("function '" + node.GetName() + "' returns void - you can't use it in expressions");
	}
	m_stack.push_back(returnValue);
}

void ExpressionCodegen::Visit(const ArrayElementAccessAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::AllocaInst* arrayPtr = m_context.GetVariable(node.GetName());
	if (!arrayPtr)
	{
		throw std::runtime_error("array '" + node.GetName() + "' is not defined");
	}

	if (!arrayPtr->getType()->getPointerElementType()->isPointerTy())
	{
		throw std::runtime_error("variable '" + node.GetName() + "' can't be accessed via index");
	}

	// Will contain pointer to element that user trying to access
	llvm::Value* elementPtr = builder.CreateLoad(arrayPtr, "load_array_ptr_from_variable");
	ExpressionType typeOfArray = ToExpressionType(elementPtr->getType());

	if (node.GetIndexCount() > typeOfArray.nesting)
	{
		auto fmt = boost::format("array %1% have only %2% dimension(s), but trying to access element by index #%3%")
			% node.GetName()
			% typeOfArray.nesting
			% node.GetIndexCount();
		throw std::runtime_error(fmt.str());
	}

	for (size_t i = 0; i < node.GetIndexCount(); ++i)
	{
		llvm::Value* index = ConvertToIntegerValue(
			Visit(node.GetIndex(i)), llvmContext, builder
		);

		elementPtr = builder.CreateLoad(builder.CreateGEP(elementPtr, index, "get_element_ptr"), "load_element");
	}

	llvm::Value* value = elementPtr;

	// Since our language doesn't support char data type (int8_t), we need to cast character to integer
	if (value->getType()->getTypeID() == llvm::Type::IntegerTyID && value->getType()->getIntegerBitWidth() == 8)
	{
		value = builder.CreateIntCast(value, llvm::Type::getInt32Ty(llvmContext), false, "icasttmp");
	}

	assert(value);
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

llvm::BasicBlock* StatementCodegen::GetLastBasicBlockBranch()
{
	return m_branchContinueStack.empty() ? nullptr : m_branchContinueStack.back();
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
	llvm::AllocaInst* variable = builder.CreateAlloca(type, nullptr, name + "Ptr");

	// Устанавливаем переменной значение по умолчанию
	llvm::Value* defaultValue = CreateDefaultValue(node.GetType(), llvmContext, builder);
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

void StatementCodegen::Visit(const ArrayElementAssignAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::AllocaInst* arrayPtr = m_context.GetVariable(node.GetName());
	if (!arrayPtr)
	{
		throw std::runtime_error("variable '" + node.GetName() + "' is not defined");
	}

	if (!arrayPtr->getType()->getPointerElementType()->isPointerTy())
	{
		throw std::runtime_error("variable '" + node.GetName() + "' is not array and can't be accessed via index");
	}

	llvm::Value* elementPtr = builder.CreateLoad(arrayPtr, "load_array");
	const ExpressionType typeOfArray = ToExpressionType(elementPtr->getType());

	if (node.GetIndexCount() > typeOfArray.nesting)
	{
		const auto fmt = boost::format("array %1% have only %2% dimension(s), but trying to assign element with index #%3%")
			% node.GetName()
			% typeOfArray.nesting
			% node.GetIndexCount();
		throw std::runtime_error(fmt.str());
	}

	for (size_t i = 0; i < node.GetIndexCount(); ++i)
	{
		llvm::Value* index = ConvertToIntegerValue(m_expressionCodegen.Visit(node.GetIndex(i)), llvmContext, builder);
		elementPtr = builder.CreateGEP(elementPtr, index, "get_element_ptr");

		// "Разыменовываем" указатель тогда, когда полученный тип указывает на другой указатель
		if (elementPtr->getType()->getPointerElementType()->isPointerTy())
		{
			elementPtr = builder.CreateLoad(elementPtr, "load_element_ptr");
		}
	}

	llvm::Value* expression = m_expressionCodegen.Visit(node.GetExpression());

	// Since we have no support for int8_t, in case of string element assign, we need to cast integer
	assert(elementPtr->getType()->isPointerTy());
	if (elementPtr->getType()->getPointerElementType()->getTypeID() == llvm::Type::IntegerTyID && elementPtr->getType()->getPointerElementType()->getIntegerBitWidth() == 8 &&
		expression->getType()->getTypeID() == llvm::Type::IntegerTyID && expression->getType()->getIntegerBitWidth() == 32)
	{
		expression = builder.CreateIntCast(expression, llvm::Type::getInt8Ty(llvmContext), false, "int32_to_int8");
	}

	// TODO: заменить этот assert на if
	// assert(expression->getType()->getTypeID() == elementPtr->getType()->getPointerElementType()->getTypeID());
	llvm::StoreInst* storeInst = builder.CreateStore(expression, elementPtr);
	(void)storeInst;
}

void StatementCodegen::Visit(const ReturnStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();
	if (func->getReturnType()->getTypeID() == llvm::Type::VoidTyID)
	{
		if (node.GetExpression())
		{
			llvm::Value* value = m_expressionCodegen.Visit(*node.GetExpression());
			const ExpressionType type = ToExpressionType(value->getType());
			throw std::runtime_error("function '" + func->getName().str() + "' can't return value of type " + ToString(type));
		}
		builder.CreateRet(nullptr);
		return;
	}

	const ExpressionType funcReturnType = ToExpressionType(func->getReturnType());
	if (!node.GetExpression())
	{
		throw std::runtime_error("return statement must have expression of type" + ToString(funcReturnType));
	}

	llvm::Value* value = m_expressionCodegen.Visit(*node.GetExpression());
	if (ToExpressionType(value->getType()) != funcReturnType)
	{
		value = CastValue(value, funcReturnType, llvmContext, builder);
		if (!value)
		{
			auto fmt = boost::format("returning expression of type %1% must be at least convertible to function return type (%2%)")
				% ToString(ToExpressionType(value->getType()))
				% ToString(funcReturnType);
			throw std::runtime_error(fmt.str());
		}
	}

	assert(ToExpressionType(value->getType()) == funcReturnType);
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
	llvm::IRBuilder<>& builder = utils.GetBuilder();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();

	llvm::Function* func = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* body = llvm::BasicBlock::Create(llvmContext, "loop", func);
	llvm::BasicBlock* afterLoop = llvm::BasicBlock::Create(llvmContext, "afterloop", func);

	llvm::Value* value = ConvertToBooleanValue(m_expressionCodegen.Visit(node.GetExpr()), llvmContext, builder);
	builder.CreateCondBr(value, body, afterLoop);

	auto putBrAfterBranchInsertionIfNecessary = [&](llvm::BasicBlock* branch) {
		if (!branch->getTerminator())
		{
			value = ConvertToBooleanValue(m_expressionCodegen.Visit(node.GetExpr()), llvmContext, builder);
			builder.CreateCondBr(value, body, afterLoop);
		}
		if (!m_branchContinueStack.empty() && !m_branchContinueStack.back()->getTerminator())
		{
			builder.SetInsertPoint(m_branchContinueStack.back());
			value = ConvertToBooleanValue(m_expressionCodegen.Visit(node.GetExpr()), llvmContext, builder);
			builder.CreateCondBr(value, body, afterLoop);
		}
		if (!m_branchContinueStack.empty())
		{
			m_branchContinueStack.pop_back();
		}
	};

	builder.SetInsertPoint(body);
	Visit(node.GetStatement());
	putBrAfterBranchInsertionIfNecessary(body);

	builder.SetInsertPoint(afterLoop);
	m_branchContinueStack.push_back(afterLoop);
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

void StatementCodegen::Visit(const BuiltinCallStatementAST& node)
{
	switch (node.GetBuiltin())
	{
	case BuiltinCallStatementAST::Print:
		CodegenAsPrint(node);
		break;
	case BuiltinCallStatementAST::Scan:
		CodegenAsScan(node);
		break;
	default:
		assert(false);
		throw std::logic_error("can't codegen undefined builtin call");
	}
}

void StatementCodegen::CodegenAsPrint(const BuiltinCallStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::LLVMContext& llvmContext = utils.GetLLVMContext();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	std::vector<llvm::Value*> expressions(node.GetParamsCount());
	for (size_t i = 0; i < expressions.size(); ++i)
	{
		expressions[i] = m_expressionCodegen.Visit(node.GetExpression(i));
		if (ToExpressionType(expressions[i]->getType()) == ExpressionType{ ExpressionType::Bool, 0 })
		{
			expressions[i] = ConvertToIntegerValue(expressions[i], llvmContext, builder);
		}
	}

	if (expressions.empty() || ToExpressionType(expressions.front()->getType()) != ExpressionType{ ExpressionType::String, 0 })
	{
		throw std::runtime_error("print statement requires string as first argument");
	}

	builder.CreateCall(m_context.GetPrintf(), expressions, "printtmp");
}

void StatementCodegen::CodegenAsScan(const BuiltinCallStatementAST& node)
{
	CodegenUtils& utils = m_context.GetUtils();
	llvm::IRBuilder<>& builder = utils.GetBuilder();

	if (node.GetParamsCount() < 2)
	{
		throw std::runtime_error("scan statement expects at least 2 parameters, " + std::to_string(node.GetParamsCount()) + " given");
	}

	std::vector<llvm::Value*> expressions(node.GetParamsCount());
	expressions[0] = m_expressionCodegen.Visit(node.GetExpression(0));
	if (ToExpressionType(expressions[0]->getType()) != ExpressionType{ ExpressionType::String, 0 })
	{
		throw std::runtime_error("scan statement requires string as first argument");
	}

	for (size_t i = 1; i < expressions.size(); ++i)
	{
		const IdentifierAST* identifier = dynamic_cast<const IdentifierAST*>(&node.GetExpression(i));
		if (!identifier)
		{
			throw std::runtime_error("you can only pass identifiers to scan");
		}

		if (auto variable = m_context.GetVariable(identifier->GetName()))
		{
			expressions[i] = variable;
		}
		else
		{
			throw std::runtime_error("variable '" + identifier->GetName() + "' is not defined");
		}
	}

	builder.CreateCall(m_context.GetScanf(), expressions, "scantmp");
}

void StatementCodegen::Visit(const FunctionCallStatementAST& node)
{
	llvm::Value* returnValue = m_expressionCodegen.GenerateFunctionCall(node.GetCallAsDerived());
	if (returnValue)
	{
		// TODO: produce warning about unused function return value
	}
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
	llvm::Type* returnType = func.GetReturnType() ?
		ToLLVMType(*func.GetReturnType(), llvmContext) :
		llvm::Type::getVoidTy(llvmContext);

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
	m_context.AddFunction(func.GetIdentifier().GetName(), llvmFunc);

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

		llvm::AllocaInst* variable = builder.CreateAlloca(ToLLVMType(param.second, llvmContext), nullptr, param.first + "Ptr");
		m_context.Define(param.first, variable);
		builder.CreateStore(&argument, variable);

		++index;
	}

	// Генерируем код инструкции функции (может быть композитной)
	StatementCodegen statementCodegen(m_context);
	statementCodegen.Visit(func.GetStatement());

	// Добавляем return void
	if (llvm::BasicBlock* lastContinueBranch = statementCodegen.GetLastBasicBlockBranch())
	{
		if (llvmFunc->getReturnType()->getTypeID() == llvm::Type::VoidTyID && !lastContinueBranch->getTerminator())
		{
			builder.SetInsertPoint(lastContinueBranch);
			builder.CreateRet(nullptr);
		}
	}
	else
	{
		if (llvmFunc->getReturnType()->getTypeID() == llvm::Type::VoidTyID && !llvmFunc->getBasicBlockList().back().getTerminator())
		{
			builder.SetInsertPoint(&llvmFunc->getBasicBlockList().back());
			builder.CreateRet(nullptr);
		}
	}

	for (llvm::BasicBlock& basicBlock : llvmFunc->getBasicBlockList())
	{
		if (!basicBlock.getTerminator())
		{
			llvmFunc->dump();
			throw std::runtime_error("every path must have return statement");
		}
	}

	std::string output;
	llvm::raw_string_ostream out(output);

	if (llvm::verifyFunction(*llvmFunc, &out))
	{
		utils.GetModule().dump();
		llvmFunc->eraseFromParent();
		throw std::runtime_error(out.str());
	}
}
