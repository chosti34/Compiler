#include "SemanticsVerifier.h"
#include <boost/format.hpp>
#include <cassert>
#include <llvm/IR/Value.h>

TypeEvaluator::TypeEvaluator(ScopeChain& scopes)
    : m_scopes(scopes)
{
}

ExpressionType TypeEvaluator::Evaluate(const IExpressionAST &expr)
{
    Visit(expr);
    assert(!m_stack.empty());
    const ExpressionType type = m_stack.back();
    m_stack.pop_back();
    return type;
}

void TypeEvaluator::Visit(const IExpressionAST &expr)
{
    expr.Accept(*this);
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
        m_stack.push_back(ExpressionType::Int);
    }
    else if (value.type() == typeid(float))
    {
        m_stack.push_back(ExpressionType::Float);
    }
    else
    {
        assert(false);
        throw std::logic_error("type evaluation error: undefined constant literal type");
    }
}

void TypeEvaluator::Visit(const BinaryExpressionAST &binary)
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

void TypeEvaluator::Visit(const UnaryAST &unary)
{
    ExpressionType evaluatedType = Evaluate(unary.GetExpr());
    if (evaluatedType == ExpressionType::String)
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
    const ExpressionType type = variableDeclaration.GetType();

    if (value)
    {
        throw std::runtime_error("variable '" + name + "' is already defined as '" + ToString(type) + "'");
    }

    m_scopes->Define(
            variableDeclaration.GetIdentifier().GetName(), Value(variableDeclaration.GetType())
    );
}

void SemanticsVerifier::Visit(const AssignStatementAST &assignment)
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

void SemanticsVerifier::Visit(const ReturnStatementAST &returnStmt)
{
    const ExpressionType evaluatedType = m_evaluator->Evaluate(returnStmt.GetExpr());
    // TODO: check return value of function
    (void)evaluatedType;
}

void SemanticsVerifier::Visit(const IfStatementAST &condition)
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

void SemanticsVerifier::Visit(const WhileStatementAST& whileStmt)
{
    const ExpressionType evaluatedType = m_evaluator->Evaluate(whileStmt.GetExpr());
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
