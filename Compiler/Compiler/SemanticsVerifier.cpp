#include "stdafx.h"
#include "SemanticsVerifier.h"

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
	}
	throw std::logic_error("can't cast undefined binary operator to string");
}
}

// Type evaluator
TypeEvaluator::TypeEvaluator(TypeScopeChain& scopes)
	: m_scopes(scopes)
{
}

ASTExpressionType TypeEvaluator::Evaluate(const IExpressionAST& expr)
{
	expr.Accept(*this);
	if (!m_stack.empty())
	{
		const ASTExpressionType evaluated = m_stack.back();
		m_stack.pop_back();
		return evaluated;
	}
	else
	{
		throw std::logic_error("type evaluating error: stack is empty");
	}
}

void TypeEvaluator::Visit(const IdentifierAST& node)
{
	const std::string& name = node.GetName();
	const auto type = m_scopes.GetValue(name);

	if (!bool(type))
	{
		throw std::runtime_error("identifier '" + name + "' is undefined");
	}

	m_stack.push_back(*type);
}

void TypeEvaluator::Visit(const LiteralConstantAST& node)
{
	const LiteralConstantAST::Value& value = node.GetValue();
	if (value.type() == typeid(int))
	{
		m_stack.push_back(ASTExpressionType::Int);
	}
	else if (value.type() == typeid(double))
	{
		m_stack.push_back(ASTExpressionType::Float);
	}
	else
	{
		assert(false);
		throw std::logic_error("type evaluation error: undefined literal constant type");
	}
}

void TypeEvaluator::Visit(const BinaryExpressionAST& node)
{
	const ASTExpressionType left = Evaluate(node.GetLeft());
	const ASTExpressionType right = Evaluate(node.GetRight());

	// '+', '-', '*', '/'
	// float int
	// int float
	// int int
	// float float

	// TODO: check left and right with operator
	if (left != right)
	{
		auto fmt = boost::format("can't perform operator '%1%' on operands with types '%2%' and '%3%'")
			% ToString(node.GetOperator())
			% ToString(left)
			% ToString(right);
		throw std::runtime_error(fmt.str());
	}

	m_stack.push_back(left);
}

void TypeEvaluator::Visit(const UnaryAST& node)
{
	ASTExpressionType evaluatedType = Evaluate(node.GetExpr());
	if (evaluatedType == ASTExpressionType::String)
	{
		const std::string operation = node.GetOperator() == UnaryAST::Plus ? "+" : "-";
		throw std::runtime_error("can't perform unary operation '" + operation + "' on string");
	}
	m_stack.push_back(evaluatedType);
}

// Semantics verifier
SemanticsVerifier::SemanticsVerifier()
	: m_scopes(std::make_unique<TypeScopeChain>())
	, m_evaluator(std::make_unique<TypeEvaluator>(*m_scopes))
{
}

void SemanticsVerifier::VerifySemantics(const IStatementAST& node)
{
	m_scopes->PushScope(); // global scope
	node.Accept(*this);
	m_scopes->PopScope();
}

void SemanticsVerifier::Visit(const VariableDeclarationAST& node)
{
	const std::string& name = node.GetIdentifier().GetName();
	const ASTExpressionType type = node.GetType();

	const auto defined = m_scopes->GetValue(node.GetIdentifier().GetName());
	if (bool(defined))
	{
		throw std::runtime_error("variable '" + name + "' is already defined as '" + ToString(*defined) + "'");
	}

	m_scopes->Define(node.GetIdentifier().GetName(), node.GetType());
}

void SemanticsVerifier::Visit(const AssignStatementAST& node)
{
	const auto type = m_scopes->GetValue(node.GetIdentifier().GetName());
	const std::string& name = node.GetIdentifier().GetName();

	if (!bool(type))
	{
		throw std::runtime_error("variable '" + name + "' is not defined");
	}

	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(node.GetExpr());
	if (evaluatedType != type)
	{
		// Если тип вычисленного выражения может быть неявно преобразован в тип присваиваемой переменной
		// то нужно выполнить это преобразование, иначе ошибка
		// TODO: попытаться выполнить преобразование
		auto fmt = boost::format("can't set expression of type '%1%' to variable '%2%' of type '%3%'")
			% ToString(evaluatedType)
			% name
			% ToString(*type);
		throw std::runtime_error(fmt.str());
	}

	bool assigned = m_scopes->Assign(name, evaluatedType);
	assert(assigned);
}

void SemanticsVerifier::Visit(const ReturnStatementAST& node)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(node.GetExpr());
	// TODO: check return value of function
	(void)evaluatedType;
}

void SemanticsVerifier::Visit(const IfStatementAST& node)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(node.GetExpr());
	if (!ConvertibleToBool(evaluatedType))
	{
		throw std::runtime_error("expression in condition statement must be convertible to bool");
	}

	node.GetThenStmt().Accept(*this);
	if (node.GetElseStmt())
	{
		node.GetElseStmt()->Accept(*this);
	}
}

void SemanticsVerifier::Visit(const WhileStatementAST& node)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(node.GetExpr());
	if (!ConvertibleToBool(evaluatedType))
	{
		throw std::runtime_error("expression in while statement must be convertible to bool");
	}
}

void SemanticsVerifier::Visit(const CompositeStatementAST& composite)
{
	m_scopes->PushScope();
	for (size_t i = 0; i < composite.GetCount(); ++i)
	{
		composite.GetStatement(i).Accept(*this);
	}
	m_scopes->PopScope();
}
