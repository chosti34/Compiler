#include "stdafx.h"
#include "AST.h"

// Binary expression
BinaryExpressionAST::BinaryExpressionAST(
	std::unique_ptr<IExpressionAST> && left,
	std::unique_ptr<IExpressionAST> && right,
	BinaryExpressionAST::Operator op)
	: m_left(std::move(left))
	, m_right(std::move(right))
	, m_op(op)
{
}

const IExpressionAST& BinaryExpressionAST::GetLeft()const
{
	return *m_left;
}

const IExpressionAST& BinaryExpressionAST::GetRight()const
{
	return *m_right;
}

BinaryExpressionAST::Operator BinaryExpressionAST::GetOperator()const
{
	return m_op;
}

void BinaryExpressionAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Literal constant
LiteralConstantAST::LiteralConstantAST(const LiteralConstantAST::Value& value)
	: m_value(value)
{
}

const LiteralConstantAST::Value& LiteralConstantAST::GetValue()const
{
	return m_value;
}

void LiteralConstantAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Unary operator
UnaryAST::UnaryAST(std::unique_ptr<IExpressionAST> && expr, UnaryAST::Operator op)
	: m_expr(std::move(expr))
	, m_op(op)
{
}

const IExpressionAST& UnaryAST::GetExpr()const
{
	return *m_expr;
}

UnaryAST::Operator UnaryAST::GetOperator()const
{
	return m_op;
}

void UnaryAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Identifier node
IdentifierAST::IdentifierAST(const std::string &name)
	: m_name(name)
{
}

const std::string& IdentifierAST::GetName()const
{
	return m_name;
}

void IdentifierAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

FunctionCallExprAST::FunctionCallExprAST(
	const std::string& name,
	std::vector<std::unique_ptr<IExpressionAST>> && params)
	: m_name(name)
	, m_params(std::move(params))
{
}

const std::string& FunctionCallExprAST::GetName()const
{
	return m_name;
}

size_t FunctionCallExprAST::GetParamsCount() const
{
	return m_params.size();
}

const IExpressionAST& FunctionCallExprAST::GetParam(size_t index)const
{
	if (index < m_params.size())
	{
		return *m_params[index];
	}
	throw std::runtime_error("index must be less than function call's params count");
}

void FunctionCallExprAST::Accept(IExpressionVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Variable declaration node
VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<IdentifierAST> && identifier, ExpressionType type)
	: m_identifier(std::move(identifier))
	, m_type(type)
	, m_expr(nullptr)
{
}

void VariableDeclarationAST::SetExpression(std::unique_ptr<IExpressionAST> && expr)
{
	m_expr = std::move(expr);
}

const IExpressionAST* VariableDeclarationAST::GetExpression()const
{
	return m_expr.get();
}

const IdentifierAST& VariableDeclarationAST::GetIdentifier()const
{
	return *m_identifier;
}

ExpressionType VariableDeclarationAST::GetType()const
{
	return m_type;
}

void VariableDeclarationAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Assign statement node
AssignStatementAST::AssignStatementAST(
	std::unique_ptr<IdentifierAST> && identifier,
	std::unique_ptr<IExpressionAST> && expr)
	: m_identifier(std::move(identifier))
	, m_expr(std::move(expr))
{
}

const IdentifierAST& AssignStatementAST::GetIdentifier()const
{
	return *m_identifier;
}

const IExpressionAST& AssignStatementAST::GetExpr()const
{
	return *m_expr;
}

void AssignStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Return statement node
ReturnStatementAST::ReturnStatementAST(std::unique_ptr<IExpressionAST> && expr)
	: m_expr(std::move(expr))
{
}

const IExpressionAST& ReturnStatementAST::GetExpr()const
{
	return *m_expr;
}

void ReturnStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// If statement node
IfStatementAST::IfStatementAST(
	std::unique_ptr<IExpressionAST> && expr,
	std::unique_ptr<IStatementAST> && then,
	std::unique_ptr<IStatementAST> && elif)
	: m_expr(std::move(expr))
	, m_then(std::move(then))
	, m_elif(std::move(elif))
{
}

void IfStatementAST::SetElseClause(std::unique_ptr<IStatementAST> && elif)
{
	m_elif = std::move(elif);
}

const IExpressionAST& IfStatementAST::GetExpr()const
{
	return *m_expr;
}

const IStatementAST& IfStatementAST::GetThenStmt()const
{
	return *m_then;
}

const IStatementAST* IfStatementAST::GetElseStmt()const
{
	return m_elif ? m_elif.get() : nullptr;
}

void IfStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// While statement node
WhileStatementAST::WhileStatementAST(
	std::unique_ptr<IExpressionAST> && expr,
	std::unique_ptr<IStatementAST> && stmt)
	: m_expr(std::move(expr))
	, m_stmt(std::move(stmt))
{
}

const IExpressionAST& WhileStatementAST::GetExpr()const
{
	return *m_expr;
}

const IStatementAST& WhileStatementAST::GetStatement()const
{
	return *m_stmt;
}

void WhileStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Composite statement node
void CompositeStatementAST::AddStatement(std::unique_ptr<IStatementAST> && stmt)
{
	m_statements.push_back(std::move(stmt));
}

const IStatementAST& CompositeStatementAST::GetStatement(size_t index)const
{
	if (index >= m_statements.size())
	{
		throw std::out_of_range("index must be less that statements count");
	}
	return *m_statements[index];
}

size_t CompositeStatementAST::GetCount()const
{
	return m_statements.size();
}

void CompositeStatementAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

// Function node
FunctionAST::FunctionAST(
	ExpressionType returnType,
	std::unique_ptr<IdentifierAST> && identifier,
	std::vector<Param> && params,
	std::unique_ptr<IStatementAST> && statement)
	: m_returnType(returnType)
	, m_identifier(std::move(identifier))
	, m_params(std::move(params))
	, m_statement(std::move(statement))
{
}

ExpressionType FunctionAST::GetReturnType()const
{
	return m_returnType;
}

const IdentifierAST& FunctionAST::GetIdentifier()const
{
	return *m_identifier;
}

const std::vector<FunctionAST::Param>& FunctionAST::GetParams()const
{
	return m_params;
}

const IStatementAST& FunctionAST::GetStatement()const
{
	return *m_statement;
}

// Program node (root)
void ProgramAST::AddFunction(std::unique_ptr<FunctionAST> && function)
{
	m_functions.push_back(std::move(function));
}

size_t ProgramAST::GetFunctionsCount()const
{
	return m_functions.size();
}

const FunctionAST& ProgramAST::GetFunction(size_t index)const
{
	if (index >= m_functions.size())
	{
		throw std::invalid_argument("index must be less than functions count");
	}
	return *m_functions[index];
}

PrintAST::PrintAST(std::unique_ptr<IExpressionAST>&& expression)
	: m_expression(std::move(expression))
{
}

const IExpressionAST& PrintAST::GetExpression()const
{
	return *m_expression;
}

void PrintAST::Accept(IStatementVisitor& visitor)const
{
	visitor.Visit(*this);
}

FunctionCallStatementAST::FunctionCallStatementAST(std::unique_ptr<FunctionCallExprAST> && call)
	: m_call(std::move(call))
{
}

const IExpressionAST& FunctionCallStatementAST::GetCall()const
{
	return *m_call;
}

void FunctionCallStatementAST::Accept(IStatementVisitor& visitor) const
{
	visitor.Visit(*this);
}

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
	throw std::logic_error("ToString: can't cast undefined binary operator to string");
}

std::string ToString(UnaryAST::Operator operation)
{
	switch (operation)
	{
	case UnaryAST::Plus:
		return "+";
	case UnaryAST::Minus:
		return "-";
	case UnaryAST::Negation:
		return "!";
	}
	throw std::logic_error("ToString: can't cast undefined unary operator to string");
}
