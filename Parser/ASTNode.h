#pragma once
#include <string>
#include <memory>
#include <cassert>

class BinOpNode;
class LeafNumNode;
class UnOpNode;

class IASTVisitor
{
public:
	virtual ~IASTVisitor() = default;
	virtual void Visit(const BinOpNode& node) = 0;
	virtual void Visit(const LeafNumNode& node) = 0;
	virtual void Visit(const UnOpNode& node) = 0;
};

class ASTNode
{
public:
	using Ptr = std::unique_ptr<ASTNode>;
	virtual ~ASTNode() = default;
	virtual void Accept(IASTVisitor& visitor)const = 0;
};

class BinOpNode : public ASTNode
{
public:
	enum Operator
	{
		Plus,
		Minus,
		Mul,
		Div
	};

	BinOpNode(ASTNode::Ptr&& left, ASTNode::Ptr&& right, Operator op)
		: m_left(std::move(left))
		, m_right(std::move(right))
		, m_op(op)
	{
	}

	const ASTNode& GetLeft()const
	{
		return *m_left;
	}

	const ASTNode& GetRight()const
	{
		return *m_right;
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	void Accept(IASTVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	ASTNode::Ptr m_left;
	ASTNode::Ptr m_right;
	Operator m_op;
};

class LeafNumNode : public ASTNode
{
public:
	LeafNumNode(int value)
		: m_value(value)
	{
	}

	int GetValue()const
	{
		return m_value;
	}

	void Accept(IASTVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	int m_value;
};

class UnOpNode : public ASTNode
{
public:
	enum Operator
	{
		Plus,
		Minus
	};

	UnOpNode(ASTNode::Ptr&& expr, Operator op)
		: m_expr(std::move(expr))
		, m_op(op)
	{
	}

	const ASTNode& GetExpr()const
	{
		return *m_expr;
	}

	Operator GetOperator()const
	{
		return m_op;
	}

	void Accept(IASTVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	ASTNode::Ptr m_expr;
	Operator m_op;
};

class IStatementAST
{
public:
	virtual ~IStatementAST() = default;
};

class IfStatementAST : public IStatementAST
{
public:
	explicit IfStatementAST(
		std::unique_ptr<ASTNode> && expr,
		std::unique_ptr<IStatementAST> && then,
		std::unique_ptr<IStatementAST> && elif)
		: m_expr(std::move(expr))
		, m_then(std::move(then))
		, m_elif(std::move(elif))
	{
	}

private:
	std::unique_ptr<ASTNode> m_expr;
	std::unique_ptr<IStatementAST> m_then;
	std::unique_ptr<IStatementAST> m_elif;
};

class WhileStatementAST : public IStatementAST
{
public:
	explicit WhileStatementAST(
		std::unique_ptr<ASTNode> && expr,
		std::unique_ptr<IStatementAST> && stmt)
		: m_expr(std::move(expr))
		, m_stmt(std::move(stmt))
	{
	}

private:
	std::unique_ptr<ASTNode> m_expr;
	std::unique_ptr<IStatementAST> m_stmt;
};

class AssignStatementAST : public IStatementAST
{
public:
	explicit AssignStatementAST(const std::string &left, std::unique_ptr<ASTNode> && expr)
		: m_left(left)
		, m_expr(std::move(expr))
	{
	}

private:
	std::string m_left;
	std::unique_ptr<ASTNode> m_expr;
};

class VariableDeclStatementAST : public IStatementAST
{
public:
	explicit VariableDeclStatementAST(const std::string &name, const std::string &type)
		: m_name(name)
		, m_type(type)
	{
	}

private:
	std::string m_name;
	std::string m_type;
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
	explicit ReturnStatementAST(std::unique_ptr<ASTNode> && expr)
		: m_expr(std::move(expr))
	{
	}

private:
	std::unique_ptr<ASTNode> m_expr;
};

class ExpressionCalculator : private IASTVisitor
{
public:
	int Calculate(const ASTNode& node)
	{
		node.Accept(*this);
		return m_acc;
	}

	void Visit(const BinOpNode& node) override
	{
		switch (node.GetOperator())
		{
		case BinOpNode::Plus:
			m_acc = Calculate(node.GetLeft()) + Calculate(node.GetRight());
			break;
		case BinOpNode::Minus:
			m_acc = Calculate(node.GetLeft()) - Calculate(node.GetRight());
			break;
		case BinOpNode::Mul:
			m_acc = Calculate(node.GetLeft()) * Calculate(node.GetRight());
			break;
		case BinOpNode::Div:
			m_acc = Calculate(node.GetLeft()) / Calculate(node.GetRight());
			break;
		default:
			assert(false);
			throw std::logic_error("undefined operator");
		}
	}

	void Visit(const UnOpNode& node) override
	{
		m_acc = -Calculate(node.GetExpr());
	}

	void Visit(const LeafNumNode& node) override
	{
		m_acc = node.GetValue();
	}

private:
	int m_acc;
};
