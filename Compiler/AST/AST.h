#pragma once
#include "ASTVisitor.h"
#include "ASTExpressionType.h"

#include <memory>
#include <string>
#include <vector>
#include <boost/variant.hpp>

class IExpressionAST
{
public:
	using Ptr = std::unique_ptr<IExpressionAST>;
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

	explicit BinaryExpressionAST(
		std::unique_ptr<IExpressionAST> && left,
		std::unique_ptr<IExpressionAST> && right,
		Operator op);

	const IExpressionAST& GetLeft()const;
	const IExpressionAST& GetRight()const;
	Operator GetOperator()const;

	void Accept(IExpressionVisitor& visitor)const override;

private:
	std::unique_ptr<IExpressionAST> m_left;
	std::unique_ptr<IExpressionAST> m_right;
	Operator m_op;
};

class LiteralConstantAST : public IExpressionAST
{
public:
	// TODO: add bool, string literal
	using Value = boost::variant<int, double>;

	explicit LiteralConstantAST(const Value& value);
	const Value& GetValue()const;
	void Accept(IExpressionVisitor& visitor)const override;

private:
	Value m_value;
};

class UnaryAST : public IExpressionAST
{
public:
	enum Operator
	{
		Plus,
		Minus
	};

	explicit UnaryAST(std::unique_ptr<IExpressionAST> && expr, Operator op);

	const IExpressionAST& GetExpr()const;
	Operator GetOperator()const;

	void Accept(IExpressionVisitor& visitor)const override;

private:
	std::unique_ptr<IExpressionAST> m_expr;
	Operator m_op;
};

class IdentifierAST : public IExpressionAST
{
public:
	explicit IdentifierAST(const std::string& name);
	const std::string& GetName()const;

	void Accept(IExpressionVisitor& visitor)const override;

private:
	std::string m_name;
};

class FunctionCallExprAST : public IExpressionAST
{
public:
	explicit FunctionCallExprAST(
		const std::string& name,
		std::vector<std::unique_ptr<IExpressionAST>> && params);

	const std::string& GetName()const;
	size_t GetParamsCount()const;
	const IExpressionAST& GetParam(size_t index)const;

	void Accept(IExpressionVisitor& visitor)const override;

private:
	std::string m_name;
	std::vector<std::unique_ptr<IExpressionAST>> m_params;
};

class IStatementAST
{
public:
	using Ptr = std::unique_ptr<IStatementAST>;
	virtual ~IStatementAST() = default;
	virtual void Accept(IStatementVisitor& visitor)const = 0;
};

class VariableDeclarationAST : public IStatementAST
{
public:
	explicit VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ASTExpressionType type);

	void SetExpression(std::unique_ptr<IExpressionAST> && expr);
	const IExpressionAST* GetExpression()const;

	const IdentifierAST& GetIdentifier()const;
	ASTExpressionType GetType()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	ASTExpressionType m_type;
	std::unique_ptr<IExpressionAST> m_expr; // can be nullptr
};

class AssignStatementAST : public IStatementAST
{
public:
	explicit AssignStatementAST(
		std::unique_ptr<IdentifierAST> && identifier,
		std::unique_ptr<IExpressionAST> && expr);

	const IdentifierAST& GetIdentifier()const;
	const IExpressionAST& GetExpr()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<IdentifierAST> m_identifier;
	std::unique_ptr<IExpressionAST> m_expr;
};

class ReturnStatementAST : public IStatementAST
{
public:
	explicit ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr);

	const IExpressionAST& GetExpr()const;
	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<IExpressionAST> m_expr;
};

class IfStatementAST : public IStatementAST
{
public:
	explicit IfStatementAST(
		std::unique_ptr<IExpressionAST> && expr,
		std::unique_ptr<IStatementAST> && then,
		std::unique_ptr<IStatementAST> && elif = nullptr);

	void SetElseClause(std::unique_ptr<IStatementAST> && elif);
	const IExpressionAST& GetExpr()const;
	const IStatementAST& GetThenStmt()const;
	const IStatementAST* GetElseStmt()const;

	void Accept(IStatementVisitor& visitor)const override;

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
		std::unique_ptr<IStatementAST> && stmt);

	const IExpressionAST& GetExpr()const;
	const IStatementAST& GetStatement()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<IExpressionAST> m_expr;
	std::unique_ptr<IStatementAST> m_stmt;
};

class CompositeStatementAST : public IStatementAST
{
public:
	void AddStatement(std::unique_ptr<IStatementAST> && stmt);
	const IStatementAST& GetStatement(size_t index)const;
	size_t GetCount()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::vector<std::unique_ptr<IStatementAST>> m_statements;
};

class PrintAST : public IStatementAST
{
public:
	explicit PrintAST(std::unique_ptr<IExpressionAST> && expression);
	const IExpressionAST& GetExpression()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<IExpressionAST> m_expression;
};

// Function call statement with ignoring returning value of a function
class FunctionCallStatementAST : public IStatementAST
{
public:
	explicit FunctionCallStatementAST(std::unique_ptr<FunctionCallExprAST> && call);
	const IExpressionAST& GetCall()const;

	void Accept(IStatementVisitor& visitor)const override;

private:
	std::unique_ptr<FunctionCallExprAST> m_call;
};

class FunctionAST
{
public:
	using Param = std::pair<std::string, ASTExpressionType>;

	explicit FunctionAST(
		ASTExpressionType returnType,
		std::unique_ptr<IdentifierAST> && identifier,
		std::vector<Param> && params,
		std::unique_ptr<IStatementAST> && statement);

	ASTExpressionType GetReturnType()const;
	const IdentifierAST& GetIdentifier()const;
	const std::vector<Param>& GetParams()const;
	const IStatementAST& GetStatement()const;

private:
	ASTExpressionType m_returnType;
	std::vector<Param> m_params;
	std::unique_ptr<IdentifierAST> m_identifier;
	std::unique_ptr<IStatementAST> m_statement;
};

class ProgramAST
{
public:
	void AddFunction(std::unique_ptr<FunctionAST> && function);

	size_t GetFunctionsCount()const;
	const FunctionAST& GetFunction(size_t index)const;

private:
	std::vector<std::unique_ptr<FunctionAST>> m_functions;
};
