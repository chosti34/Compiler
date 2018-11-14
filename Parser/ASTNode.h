#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <optional>
#include <unordered_map>

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
	explicit VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ValueType type)
		: m_identifier(std::move(identifier))
		, m_type(type)
	{
	}

	const IdentifierAST& GetIdentifier()const
	{
		return *m_identifier;
	}

	ValueType GetType()const
	{
		return m_type;
	}

	void Accept(IStatementVisitor& visitor)const override
	{
		visitor.Visit(*this);
	}

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	ValueType m_type;
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

class Scope
{
public:
	void Declare(const std::string& name, ValueType type)
	{
		auto found = m_variables.find(name);
		if (found != m_variables.end())
		{
			throw std::runtime_error("variable '" + name + "' already exists");
		}
		m_variables.emplace(name, type);
	}

	std::optional<ValueType> GetType(const std::string& name)
	{
		auto found = m_variables.find(name);
		if (found == m_variables.end())
		{
			return std::nullopt;
		}
		return found->second;
	}

private:
	std::unordered_map<std::string, ValueType> m_variables;
};

class EvaluationContext
{
public:
	explicit EvaluationContext()
	{
		PushScope();
	}

	~EvaluationContext()
	{
		PopScope();
	}

	void PushScope()
	{
		m_scopes.emplace_back();
	}

	void PopScope()
	{
		m_scopes.pop_back();
	}

	void Declare(const std::string& name, ValueType type)
	{
		m_scopes.back().Declare(name, type);
	}

	ValueType GetType(const std::string& name)
	{
		for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
		{
			std::optional<ValueType> type = it->GetType(name);
			if (type)
			{
				return *type;
			}
		}
		throw std::runtime_error("variable '" + name + "' is not defined");
	}

private:
	std::vector<Scope> m_scopes;
};

class ExpressionVisitor : public IExpressionVisitor
{
public:
	explicit ExpressionVisitor(std::shared_ptr<EvaluationContext> context)
		: m_context(std::move(context))
	{
	}

	ValueType GetValue()const
	{
		return m_value;
	}

	void Visit(const IExpressionAST& expr)
	{
		expr.Accept(*this);
	}

	void Visit(const BinaryExpressionAST& binary) override
	{
		Visit(binary.GetLeft());
		Visit(binary.GetRight());

		auto rightType = m_types.back(); m_types.pop_back();
		auto leftType = m_types.back(); m_types.pop_back();

		if (rightType != leftType)
		{
			throw std::runtime_error("types doesn't match");
		}

		m_value = rightType;
	}

	void Visit(const NumberConstantAST& number) override
	{
		m_types.push_back(number.GetType() == NumberConstantAST::Int ? ValueType::Int : ValueType::Float);
		m_value = m_types.back();
	}

	void Visit(const UnaryAST& unary) override
	{
		Visit(unary.GetExpr());
		m_value = m_types.back();
	}

	void Visit(const IdentifierAST& identifier) override
	{
		m_types.push_back(m_context->GetType(identifier.GetName()));
		m_value = m_types.back();
	}

private:
	ValueType m_value;
	std::vector<ValueType> m_types;
	std::shared_ptr<EvaluationContext> m_context;
};

class StatementVisitor : public IStatementVisitor
{
public:
	explicit StatementVisitor()
		: m_context(std::make_shared<EvaluationContext>())
		, m_exprVisitor(std::make_unique<ExpressionVisitor>(m_context))
	{
	}

	void Visit(const IStatementAST& stmt)
	{
		stmt.Accept(*this);
	}

	void Visit(const VariableDeclarationAST& vardecl) override
	{
		m_context->Declare(vardecl.GetIdentifier().GetName(), vardecl.GetType());
	}

	void Visit(const AssignStatementAST& assign) override
	{
		m_exprVisitor->Visit(assign.GetIdentifier());
		auto vartype = m_exprVisitor->GetValue();
		m_exprVisitor->Visit(assign.GetExpr());
		auto exprtype = m_exprVisitor->GetValue();
		if (vartype != exprtype)
		{
			throw std::runtime_error("variable '" + assign.GetIdentifier().GetName() + "' can't be assigned to this type");
		}
	}

	void Visit(const ReturnStatementAST& ret) override
	{
		m_exprVisitor->Visit(ret.GetExpr());
	}

	void Visit(const IfStatementAST& ifstmt) override
	{
		m_exprVisitor->Visit(ifstmt.GetExpr());
		Visit(ifstmt.GetThenStmt());
		if (ifstmt.GetElseStmt())
		{
			Visit(*ifstmt.GetElseStmt());
		}
	}

	void Visit(const WhileStatementAST& loop) override
	{
		m_exprVisitor->Visit(loop.GetExpr());
		Visit(loop.GetStatement());
	}

	void Visit(const CompositeStatementAST& composite) override
	{
		m_context->PushScope();
		for (size_t i = 0; i < composite.GetCount(); ++i)
		{
			Visit(composite.GetStatement(i));
		}
		m_context->PopScope();
	}

private:
	std::shared_ptr<EvaluationContext> m_context;
	std::unique_ptr<ExpressionVisitor> m_exprVisitor;
};
