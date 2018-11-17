#pragma once

#include "ScopeChain.h"

#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <boost/format.hpp>

class BinaryExpressionAST;
class NumberConstantAST;
class UnaryAST;
class IdentifierAST;

class IExpressionVisitor
{
public:
	virtual ~IExpressionVisitor() = default;
	virtual void Visit(const BinaryExpressionAST& node) = 0;
	virtual void Visit(const NumberConstantAST& node) = 0;
	virtual void Visit(const UnaryAST& node) = 0;
	virtual void Visit(const IdentifierAST& node) = 0;
};

// Expression
class IExpressionAST
{
public:
	virtual ~IExpressionAST() = default;
	virtual void Accept(IExpressionVisitor& visitor)const = 0;
};

class BinaryExpressionAST : public IExpressionAST
{
public:
	enum Operator
	{
		Plus,
		Minus,
		Mul,
		Div
	};

	BinaryExpressionAST(
		std::unique_ptr<IExpressionAST> && left,
		std::unique_ptr<IExpressionAST> && right,
		Operator op)
		: m_left(std::move(left))
		, m_right(std::move(right))
		, m_op(op)
	{
	}

	const IExpressionAST& GetLeft()const
	{
		return *m_left;
	}

	const IExpressionAST& GetRight()const
	{
		return *m_right;
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	void Accept(IExpressionVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IExpressionAST> m_left;
	std::unique_ptr<IExpressionAST> m_right;
	Operator m_op;
};

class NumberConstantAST : public IExpressionAST
{
public:
	enum Type
	{
		Int,
		Float
	};

	NumberConstantAST(double value, Type type)
		: m_value(value)
		, m_type(type)
	{
	}

	double GetValue()const
	{
		return m_value;
	}

	Type GetType()const
	{
		return m_type;
	}

	void Accept(IExpressionVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	double m_value;
	Type m_type;
};

class UnaryAST : public IExpressionAST
{
public:
	enum Operator
	{
		Plus,
		Minus
	};

	UnaryAST(std::unique_ptr<IExpressionAST> && expr, Operator op)
		: m_expr(std::move(expr))
		, m_op(op)
	{
	}

	const IExpressionAST& GetExpr()const
	{
		return *m_expr;
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	void Accept(IExpressionVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IExpressionAST> m_expr;
	Operator m_op;
};

class IdentifierAST : public IExpressionAST
{
public:
	explicit IdentifierAST(const std::string &name)
		: m_name(name)
	{
	}

	const std::string& GetName()const
	{
		return m_name;
	}

	void Accept(IExpressionVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::string m_name;
};

class VariableDeclarationAST;
class AssignStatementAST;
class ReturnStatementAST;
class IfStatementAST;
class WhileStatementAST;
class CompositeStatementAST;

class IStatementVisitor
{
public:
	virtual ~IStatementVisitor() = default;
	virtual void Visit(const VariableDeclarationAST& vardecl) = 0;
	virtual void Visit(const AssignStatementAST& assign) = 0;
	virtual void Visit(const ReturnStatementAST& ret) = 0;
	virtual void Visit(const IfStatementAST& ifstmt) = 0;
	virtual void Visit(const WhileStatementAST& loop) = 0;
	virtual void Visit(const CompositeStatementAST& composite) = 0;
};

// Statements
class IStatementAST
{
public:
	virtual ~IStatementAST() = default;
	virtual void Accept(IStatementVisitor& visitor)const = 0;
};

class VariableDeclarationAST : public IStatementAST
{
public:
	explicit VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ExpressionType type)
		: m_identifier(std::move(identifier))
		, m_type(type)
	{
	}

	const IdentifierAST& GetIdentifier()const
	{
		return *m_identifier;
	}

	ExpressionType GetType()const
	{
		return m_type;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	ExpressionType m_type;
};

class AssignStatementAST : public IStatementAST
{
public:
	explicit AssignStatementAST(
		std::unique_ptr<IdentifierAST> && identifier, std::unique_ptr<IExpressionAST> && expr)
		: m_identifier(std::move(identifier))
		, m_expr(std::move(expr))
	{
	}

	const IdentifierAST& GetIdentifier()const
	{
		return *m_identifier;
	}

	const IExpressionAST& GetExpr()const
	{
		return *m_expr;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	std::unique_ptr<IExpressionAST> m_expr;
};

class ReturnStatementAST : public IStatementAST
{
public:
	explicit ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr)
		: m_expr(std::move(expr))
	{
	}

	const IExpressionAST& GetExpr()const
	{
		return *m_expr;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IExpressionAST> m_expr;
};

class IfStatementAST : public IStatementAST
{
public:
	explicit IfStatementAST(
		std::unique_ptr<IExpressionAST> && expr,
		std::unique_ptr<IStatementAST> && then,
		std::unique_ptr<IStatementAST> && elif = nullptr)
		: m_expr(std::move(expr))
		, m_then(std::move(then))
		, m_elif(std::move(elif))
	{
	}

	void SetElseClause(std::unique_ptr<IStatementAST> && elif)
	{
		m_elif = std::move(elif);
	}

	const IExpressionAST& GetExpr()const
	{
		return *m_expr;
	}

	const IStatementAST& GetThenStmt()const
	{
		return *m_then;
	}

	const IStatementAST* GetElseStmt()const
	{
		return m_elif ? m_elif.get() : nullptr;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IExpressionAST> m_expr;
	std::unique_ptr<IStatementAST> m_then;
	std::unique_ptr<IStatementAST> m_elif;
};

class WhileStatementAST : public IStatementAST
{
public:
	explicit WhileStatementAST(
		std::unique_ptr<IExpressionAST> && expr,
		std::unique_ptr<IStatementAST> && stmt)
		: m_expr(std::move(expr))
		, m_stmt(std::move(stmt))
	{
	}

	const IExpressionAST& GetExpr()const
	{
		return *m_expr;
	}

	const IStatementAST& GetStatement()const
	{
		return *m_stmt;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IExpressionAST> m_expr;
	std::unique_ptr<IStatementAST> m_stmt;
};

class CompositeStatementAST : public IStatementAST
{
public:
	void AddStatement(std::unique_ptr<IStatementAST> && stmt)
	{
		m_statements.push_back(std::move(stmt));
	}

	const IStatementAST& GetStatement(size_t index)const
	{
		if (index >= m_statements.size())
		{
			throw std::out_of_range("index must be less that statements count");
		}
		return *m_statements[index];
	}

	size_t GetCount()const
	{
		return m_statements.size();
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::vector<std::unique_ptr<IStatementAST>> m_statements;
};

// Function
class FunctionAST
{
public:
	explicit FunctionAST(std::unique_ptr<IStatementAST> && stmt)
		: m_stmt(std::move(stmt))
	{
	}

private:
	std::unique_ptr<IStatementAST> m_stmt;
};

class ProgramAST
{
public:
	void AddFunction(std::unique_ptr<FunctionAST> && function)
	{
		m_functions.push_back(std::move(function));
	}

private:
	std::vector<std::unique_ptr<FunctionAST>> m_functions;
};

// Visitors
class ExpressionCalculator : private IExpressionVisitor
{
public:
	double Calculate(const IExpressionAST& node)
	{
		node.Accept(*this);
		return m_acc;
	}

	void Visit(const BinaryExpressionAST& node) override
	{
		switch (node.GetOperator())
		{
		case BinaryExpressionAST::Plus:
			m_acc = Calculate(node.GetLeft()) + Calculate(node.GetRight());
			break;
		case BinaryExpressionAST::Minus:
			m_acc = Calculate(node.GetLeft()) - Calculate(node.GetRight());
			break;
		case BinaryExpressionAST::Mul:
			m_acc = Calculate(node.GetLeft()) * Calculate(node.GetRight());
			break;
		case BinaryExpressionAST::Div:
			m_acc = Calculate(node.GetLeft()) / Calculate(node.GetRight());
			break;
		default:
			assert(false);
			throw std::logic_error("undefined operator");
		}
	}

	void Visit(const UnaryAST& node) override
	{
		m_acc = -Calculate(node.GetExpr());
	}

	void Visit(const NumberConstantAST& node) override
	{
		m_acc = node.GetValue();
	}

	void Visit(const IdentifierAST& identifier) override
	{
		throw std::logic_error(
			"can't get value of '" + identifier.GetName() + "': symbol table is not implemented");
	}

private:
	double m_acc;
};

class TypeEvaluator : public IExpressionVisitor
{
public:
	explicit TypeEvaluator(ScopeChain& scopes)
		: m_scopes(scopes)
	{
	}

	ExpressionType Evaluate(const IExpressionAST& expr)
	{
		Visit(expr);
		assert(!m_stack.empty());
		const ExpressionType type = m_stack.back();
		m_stack.pop_back();
		return type;
	}

private:
	void Visit(const IExpressionAST& expr)
	{
		expr.Accept(*this);
	}

	void Visit(const BinaryExpressionAST& binary) override
	{
		const ExpressionType left = Evaluate(binary.GetLeft());
		const ExpressionType right = Evaluate(binary.GetRight());

		std::string operation;
		switch (binary.GetOperator())
		{
		case BinaryExpressionAST::Plus:
			operation = "+";
			break;
		case BinaryExpressionAST::Minus:
			operation = "-";
			break;
		case BinaryExpressionAST::Mul:
			operation = "*";
			break;
		case BinaryExpressionAST::Div:
			operation = "/";
			break;
		default:
			throw std::logic_error("undefined binary operator");
		}

		if (left != right)
		{
			auto fmt = boost::format("can't perform operator '%1%' on operands with types '%2%' and '%3%'")
				% operation
				% ToString(left)
				% ToString(right);
			throw std::runtime_error(fmt.str());
		}

		// TODO: check left and right with operator
		m_stack.push_back(left);
	}

	void Visit(const NumberConstantAST& number) override
	{
		m_stack.push_back(number.GetType() == NumberConstantAST::Int ? ExpressionType::Int : ExpressionType::Float);
	}

	void Visit(const UnaryAST& unary) override
	{
		ExpressionType evaluatedType = Evaluate(unary.GetExpr());
		if (evaluatedType == ExpressionType::String)
		{
			const std::string operation = unary.GetOperator() == UnaryAST::Plus ? "+" : "-";
			throw std::runtime_error("can't perform unary operation '" + operation + "' on string");
		}
		m_stack.push_back(evaluatedType);
	}

	void Visit(const IdentifierAST& identifier) override
	{
		const std::string& name = identifier.GetName();
		const Value* value = m_scopes.GetValue(name);

		if (!value)
		{
			throw std::runtime_error("identifier '" + name + "' is undefined");
		}

		m_stack.push_back(value->GetExpressionType());
	}

private:
	std::vector<ExpressionType> m_stack;
	ScopeChain& m_scopes;
};

class SemanticsVerifier : public IStatementVisitor
{
public:
	explicit SemanticsVerifier()
		: m_scopes(std::make_unique<ScopeChain>())
		, m_evaluator(std::make_unique<TypeEvaluator>(*m_scopes))
	{
	}

	void VerifySemantics(const IStatementAST& statement)
	{
		m_scopes->PushScope(); // global scope
		Visit(statement);
		m_scopes->PopScope();
	}

private:
	void Visit(const IStatementAST& stmt)
	{
		stmt.Accept(*this);
	}

	void Visit(const VariableDeclarationAST& variableDeclaration) override
	{
		const Value* value = m_scopes->GetValue(variableDeclaration.GetIdentifier().GetName());
		const std::string& name = variableDeclaration.GetIdentifier().GetName();
		const ExpressionType type = variableDeclaration.GetType();

		if (value)
		{
			throw std::runtime_error("variable '" + name + "' is already defined as '" + ToString(type) + "'");
		}

		m_scopes->Define(
			variableDeclaration.GetIdentifier().GetName(), Value(variableDeclaration.GetType())
		);
	}

	void Visit(const AssignStatementAST& assignment) override
	{
		Value* value = m_scopes->GetValue(assignment.GetIdentifier().GetName());
		const std::string& name = assignment.GetIdentifier().GetName();

		if (!value)
		{
			throw std::runtime_error("variable '" + name + "' is not defined");
		}

		const ExpressionType evaluatedType = m_evaluator->Evaluate(assignment.GetExpr());
		if (evaluatedType != value->GetExpressionType())
		{
			auto fmt = boost::format("can't set expression of type '%1%' to variable '%2' of type '%3%'")
				% ToString(evaluatedType)
				% name
				% ToString(value->GetExpressionType());
			throw std::runtime_error(fmt.str());
		}

		// TODO: set value
	}

	void Visit(const ReturnStatementAST& returnStmt) override
	{
		const ExpressionType evaluatedType = m_evaluator->Evaluate(returnStmt.GetExpr());
		// TODO: check return value of function
		(void)evaluatedType;
	}

	void Visit(const IfStatementAST& condition) override
	{
		const ExpressionType evaluatedType = m_evaluator->Evaluate(condition.GetExpr());
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

	void Visit(const WhileStatementAST& whileStmt) override
	{
		const ExpressionType evaluatedType = m_evaluator->Evaluate(whileStmt.GetExpr());
		if (!ConvertibleToBool(evaluatedType))
		{
			throw std::runtime_error("expression in while statement must be convertible to bool");
		}
	}

	void Visit(const CompositeStatementAST& composite) override
	{
		m_scopes->PushScope();
		for (size_t i = 0; i < composite.GetCount(); ++i)
		{
			Visit(composite.GetStatement(i));
		}
		m_scopes->PopScope();
	}

private:
	std::unique_ptr<ScopeChain> m_scopes;
	std::unique_ptr<TypeEvaluator> m_evaluator;
};
