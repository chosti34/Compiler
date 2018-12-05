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

TypeEvaluator::TypeEvaluator(ScopeChain& scopes)
	: m_scopes(scopes)
{
}

ASTExpressionType TypeEvaluator::Evaluate(const IExpressionAST& expr)
{
	expr.Accept(*this);
	if (!m_stack.empty())
	{
		const ASTExpressionType evaluated = m_stack[m_stack.size() - 1];
		m_stack.pop_back();
		return evaluated;
	}
	else
	{
		throw std::logic_error("type evaluating error: stack is empty");
	}
}

void TypeEvaluator::Visit(const IdentifierAST &identifier)
{
	const std::string& name = identifier.GetName();
	const Value* value = m_scopes.GetValue(name);

	if (!value)
	{
		throw std::runtime_error("identifier '" + name + "' is undefined");
	}

	m_stack.push_back(value->GetExpressionType());
}

void TypeEvaluator::Visit(const LiteralConstantAST& literal)
{
	const LiteralConstantAST::Value& value = literal.GetValue();
	if (value.type() == typeid(int))
	{
		m_stack.push_back(ASTExpressionType::Int);
	}
	else if (value.type() == typeid(float))
	{
		m_stack.push_back(ASTExpressionType::Float);
	}
	else
	{
		assert(false);
		throw std::logic_error("type evaluation error: undefined constant literal type");
	}
}

void TypeEvaluator::Visit(const BinaryExpressionAST &binary)
{
	const ASTExpressionType left = Evaluate(binary.GetLeft());
	const ASTExpressionType right = Evaluate(binary.GetRight());

	// '+', '-', '*', '/'
	// float int
	// int float
	// int int
	// float float

	// TODO: check left and right with operator
	if (left != right)
	{
		auto fmt = boost::format("can't perform operator '%1%' on operands with types '%2%' and '%3%'")
			% ToString(binary.GetOperator())
			% ToString(left)
			% ToString(right);
		throw std::runtime_error(fmt.str());
	}

	m_stack.push_back(left);
}

void TypeEvaluator::Visit(const UnaryAST &unary)
{
	ASTExpressionType evaluatedType = Evaluate(unary.GetExpr());
	if (evaluatedType == ASTExpressionType::String)
	{
		const std::string operation = unary.GetOperator() == UnaryAST::Plus ? "+" : "-";
		throw std::runtime_error("can't perform unary operation '" + operation + "' on string");
	}
	m_stack.push_back(evaluatedType);
}


SemanticsVerifier::SemanticsVerifier()
	: m_scopes(std::make_unique<ScopeChain>())
	, m_evaluator(std::make_unique<TypeEvaluator>(*m_scopes))
{
}

void SemanticsVerifier::VerifySemantics(const IStatementAST &statement)
{
	m_scopes->PushScope(); // global scope
	Visit(statement);
	m_scopes->PopScope();
}

void SemanticsVerifier::Visit(const IStatementAST &stmt)
{
	stmt.Accept(*this);
}

void SemanticsVerifier::Visit(const VariableDeclarationAST &variableDeclaration)
{
	const Value* value = m_scopes->GetValue(variableDeclaration.GetIdentifier().GetName());
	const std::string& name = variableDeclaration.GetIdentifier().GetName();
	const ASTExpressionType type = variableDeclaration.GetType();

	if (value)
	{
		throw std::runtime_error("variable '" + name + "' is already defined as '" + ToString(type) + "'");
	}

	m_scopes->Define(
		variableDeclaration.GetIdentifier().GetName(), Value(variableDeclaration.GetType())
	);
}

void SemanticsVerifier::Visit(const AssignStatementAST& assignment)
{
	Value* value = m_scopes->GetValue(assignment.GetIdentifier().GetName());
	const std::string& name = assignment.GetIdentifier().GetName();

	if (!value)
	{
		throw std::runtime_error("variable '" + name + "' is not defined");
	}

	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(assignment.GetExpr());
	if (evaluatedType != value->GetExpressionType())
	{
		// Если тип вычисленного выражения может быть неявно преобразован в тип присваиваемой переменной
		// то нужно выполнить это преобразование, иначе ошибка
		// TODO: попытаться выполнить преобразование
		auto fmt = boost::format("can't set expression of type '%1%' to variable '%2' of type '%3%'")
			% ToString(evaluatedType)
			% name
			% ToString(value->GetExpressionType());
		throw std::runtime_error(fmt.str());
	}

	// TODO: set value
}

void SemanticsVerifier::Visit(const ReturnStatementAST &returnStmt)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(returnStmt.GetExpr());
	// TODO: check return value of function
	(void)evaluatedType;
}

void SemanticsVerifier::Visit(const IfStatementAST &condition)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(condition.GetExpr());
	if (!ConvertibleToBool(evaluatedType))
	{
		throw std::runtime_error("expression in condition statement must be convertible to bool");
	}

	Visit(condition.GetThenStmt());
	if (condition.GetElseStmt())
	{
		Visit(*condition.GetElseStmt());
	}
}

void SemanticsVerifier::Visit(const WhileStatementAST& whileStmt)
{
	const ASTExpressionType evaluatedType = m_evaluator->Evaluate(whileStmt.GetExpr());
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
		Visit(composite.GetStatement(i));
	}
	m_scopes->PopScope();
}
