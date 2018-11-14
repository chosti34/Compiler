#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cassert>

enum class ValueType
{
	Int,
	Float,
	Bool
};

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
	NumberConstantAST(double value)
		: m_value(value)
	{
	}

	double GetValue()const
	{
		return m_value;
	}

	void Accept(IExpressionVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	double m_value;
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

// Statements
class IStatementAST
{
public:
	virtual ~IStatementAST() = default;
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

private:
	std::unique_ptr<IExpressionAST> m_expr;
	std::unique_ptr<IStatementAST> m_stmt;
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

private:
	std::unique_ptr<IExpressionAST> m_identifier;
	std::unique_ptr<IExpressionAST> m_expr;
};

class VariableDeclarationAST : public IStatementAST
{
public:
	explicit VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ValueType type)
		: m_identifier(std::move(identifier))
		, m_type(type)
	{
	}

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	ValueType m_type;
};

class CompositeStatement : public IStatementAST
{
public:
	void AddStatement(std::unique_ptr<IStatementAST> && stmt)
	{
		m_statements.push_back(std::move(stmt));
	}

private:
	std::vector<std::unique_ptr<IStatementAST>> m_statements;
};

class ReturnStatementAST : public IStatementAST
{
public:
	explicit ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr)
		: m_expr(std::move(expr))
	{
	}

private:
	std::unique_ptr<IExpressionAST> m_expr;
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
