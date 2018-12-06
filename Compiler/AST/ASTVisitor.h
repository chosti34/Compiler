#pragma once

class LiteralConstantAST;
class IdentifierAST;
class BinaryExpressionAST;
class UnaryAST;

class IExpressionVisitor
{
public:
	virtual ~IExpressionVisitor() = default;
	virtual void Visit(const BinaryExpressionAST& node) = 0;
	virtual void Visit(const LiteralConstantAST& node) = 0;
	virtual void Visit(const UnaryAST& node) = 0;
	virtual void Visit(const IdentifierAST& node) = 0;
};

class VariableDeclarationAST;
class AssignStatementAST;
class ReturnStatementAST;
class IfStatementAST;
class WhileStatementAST;
class CompositeStatementAST;
class PrintAST;

class IStatementVisitor
{
public:
	virtual ~IStatementVisitor() = default;
	virtual void Visit(const VariableDeclarationAST& node) = 0;
	virtual void Visit(const AssignStatementAST& node) = 0;
	virtual void Visit(const ReturnStatementAST& node) = 0;
	virtual void Visit(const IfStatementAST& node) = 0;
	virtual void Visit(const WhileStatementAST& node) = 0;
	virtual void Visit(const CompositeStatementAST& node) = 0;
	virtual void Visit(const PrintAST& node) = 0;
};
