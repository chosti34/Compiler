#pragma once

class LiteralConstantAST;
class IdentifierAST;
class BinaryExpressionAST;
class UnaryAST;
class FunctionCallExpressionAST;
class ArrayElementAccessAST;

class IExpressionVisitor
{
public:
	virtual ~IExpressionVisitor() = default;
	virtual void Visit(const BinaryExpressionAST& node) = 0;
	virtual void Visit(const LiteralConstantAST& node) = 0;
	virtual void Visit(const UnaryAST& node) = 0;
	virtual void Visit(const IdentifierAST& node) = 0;
	virtual void Visit(const FunctionCallExpressionAST& node) = 0;
	virtual void Visit(const ArrayElementAccessAST& node) = 0;
};

class VariableDeclarationAST;
class AssignStatementAST;
class ArrayElementAssignAST;
class ReturnStatementAST;
class IfStatementAST;
class WhileStatementAST;
class CompositeStatementAST;
class BuiltinCallStatementAST;
class FunctionCallStatementAST;

class IStatementVisitor
{
public:
	virtual ~IStatementVisitor() = default;
	virtual void Visit(const VariableDeclarationAST& node) = 0;
	virtual void Visit(const AssignStatementAST& node) = 0;
	virtual void Visit(const ArrayElementAssignAST& node) = 0;
	virtual void Visit(const ReturnStatementAST& node) = 0;
	virtual void Visit(const IfStatementAST& node) = 0;
	virtual void Visit(const WhileStatementAST& node) = 0;
	virtual void Visit(const CompositeStatementAST& node) = 0;
	virtual void Visit(const BuiltinCallStatementAST& node) = 0;
	virtual void Visit(const FunctionCallStatementAST& node) = 0;
};
