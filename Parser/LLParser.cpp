#include "stdafx.h"
#include "LLParser.h"
#include <iostream>
#include <functional>

namespace
{
template <typename T>
T Pop(std::vector<T> &vect)
{
	auto value = std::move(vect.back());
	vect.pop_back();
	return std::move(value);
}

template <typename Derived, typename Base>
std::unique_ptr<Derived> DowncastUniquePtr(std::unique_ptr<Base> && base)
{
	std::unique_ptr<Derived> derived = nullptr;
	if (Derived* ptr = dynamic_cast<Derived*>(base.get()))
	{
		base.release();
		derived.reset(ptr);
		return std::move(derived);
	}
	assert(false);
	return nullptr;
}

class ASTBuilder
{
public:
	ASTBuilder(const Token &token)
		: m_token(token)
	{
	}

	std::unique_ptr<IExpressionAST> PopBuiltExpression()
	{
		assert(m_expressions.size() == 1);
		return std::move(Pop(m_expressions));
	}

	std::unique_ptr<IStatementAST> PopBuiltStatement()
	{
		assert(m_statements.size() == 1);
		return std::move(Pop(m_statements));
	}

	void OnIntegerTypeParse()
	{
		assert(m_token.type == Token::Int);
		m_types.push_back(ValueType::Int);
	}

	void OnFloatTypeParse()
	{
		assert(m_token.type == Token::Float);
		m_types.push_back(ValueType::Float);
	}

	void OnBoolTypeParse()
	{
		assert(m_token.type == Token::Bool);
		m_types.push_back(ValueType::Bool);
	}

	void OnIfStatementParse()
	{
		assert(!m_expressions.empty());
		assert(!m_statements.empty());
		auto expr = Pop(m_expressions);
		auto then = Pop(m_statements);
		m_statements.push_back(std::make_unique<IfStatementAST>(std::move(expr), std::move(then)));
	}

	void OnOptionalElseClauseParse()
	{
		assert(m_statements.size() >= 2);
		auto stmt = Pop(m_statements);
		auto ifStmt = DowncastUniquePtr<IfStatementAST>(Pop(m_statements));
		ifStmt->SetElseClause(std::move(stmt));
		m_statements.push_back(std::move(ifStmt));
	}

	void OnWhileLoopParse()
	{
		assert(!m_expressions.empty());
		assert(!m_statements.empty());
		auto expr = Pop(m_expressions);
		auto stmt = Pop(m_statements);
		m_statements.push_back(std::make_unique<WhileStatementAST>(
			std::move(expr), std::move(stmt)));
	}

	void OnVariableDeclarationParse()
	{
		assert(!m_expressions.empty());
		assert(!m_types.empty());
		auto type = Pop(m_types);
		auto identifier = DowncastUniquePtr<IdentifierAST>(std::move(Pop(m_expressions)));
		m_statements.push_back(std::make_unique<VariableDeclarationAST>(
			std::move(identifier), type));
	}

	void OnAssignStatementParse()
	{
		assert(m_expressions.size() >= 2);
		auto expr = Pop(m_expressions);
		auto identifier = DowncastUniquePtr<IdentifierAST>(std::move(Pop(m_expressions)));

		m_statements.push_back(std::make_unique<AssignStatementAST>(
			std::move(identifier), std::move(expr)));
	}

	void OnReturnStatementParse()
	{
		assert(!m_expressions.empty());
		m_statements.push_back(std::make_unique<ReturnStatementAST>(Pop(m_expressions)));
	}

	void OnCompositeStatementBeginParse()
	{
		m_compositeCache.emplace_back();
	}

	void OnCompositeStatementParse()
	{
		auto composite = std::make_unique<CompositeStatement>();
		for (auto& stmt : m_compositeCache.back())
		{
			composite->AddStatement(std::move(stmt));
		}
		m_compositeCache.pop_back();
		m_statements.push_back(std::move(composite));
	}

	void OnCompositeStatementPartParse()
	{
		assert(!m_statements.empty());
		m_compositeCache.back().push_back(Pop(m_statements));
	}

	void OnBinaryPlusParse()
	{
		OnBinaryExpr(BinaryExpressionAST::Plus);
	}

	void OnBinaryMinusParse()
	{
		OnBinaryExpr(BinaryExpressionAST::Minus);
	}

	void OnBinaryMulParse()
	{
		OnBinaryExpr(BinaryExpressionAST::Mul);
	}

	void OnBinaryDivParse()
	{
		OnBinaryExpr(BinaryExpressionAST::Div);
	}

	void OnIdentifierParse()
	{
		assert(m_token.type == Token::Identifier);
		m_expressions.push_back(std::make_unique<IdentifierAST>(*m_token.value));
	}

	void OnIntegerConstantParse()
	{
		assert(m_token.type == Token::IntegerConstant);
		m_expressions.push_back(std::make_unique<NumberConstantAST>(std::stod(*m_token.value)));
	}

	void OnFloatConstantParse()
	{
		assert(m_token.type == Token::FloatConstant);
		m_expressions.push_back(std::make_unique<NumberConstantAST>(std::stod(*m_token.value)));
	}

	void OnUnaryMinusParse()
	{
		auto node = std::move(m_expressions.back()); m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<UnaryAST>(std::move(node), UnaryAST::Minus));
	}

private:
	void OnBinaryExpr(BinaryExpressionAST::Operator op)
	{
		auto right = Pop(m_expressions);
		auto left = Pop(m_expressions);

		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(
			std::move(left), std::move(right), op));
	}

private:
	const Token& m_token;
	std::vector<std::unique_ptr<IExpressionAST>> m_expressions;
	std::vector<std::unique_ptr<IStatementAST>> m_statements;
	std::vector<std::vector<std::unique_ptr<IStatementAST>>> m_compositeCache;
	std::vector<ValueType> m_types;
};
}

LLParser::LLParser(std::unique_ptr<ILexer> && lexer, std::unique_ptr<LLParserTable> && table)
	: m_lexer(std::move(lexer))
	, m_table(std::move(table))
{
}

std::unique_ptr<IStatementAST> LLParser::Parse(const std::string& text)
{
	m_lexer->SetText(text);
	Token token = m_lexer->GetNextToken();

	std::vector<size_t> addresses;
	size_t index = 0;

	ASTBuilder astBuilder(token);
	std::unordered_map<std::string, std::function<void()>> actions = {
		{ "OnIfStatementParse", std::bind(&ASTBuilder::OnIfStatementParse, &astBuilder) },
		{ "OnOptionalElseClauseParse", std::bind(&ASTBuilder::OnOptionalElseClauseParse, &astBuilder) },
		{ "OnWhileLoopParse", std::bind(&ASTBuilder::OnWhileLoopParse, &astBuilder) },
		{ "OnVariableDeclarationParse", std::bind(&ASTBuilder::OnVariableDeclarationParse, &astBuilder) },
		{ "OnAssignStatementParse", std::bind(&ASTBuilder::OnAssignStatementParse, &astBuilder) },
		{ "OnReturnStatementParse", std::bind(&ASTBuilder::OnReturnStatementParse, &astBuilder) },
		{ "OnCompositeStatementBeginParse", std::bind(&ASTBuilder::OnCompositeStatementBeginParse, &astBuilder) },
		{ "OnCompositeStatementParse", std::bind(&ASTBuilder::OnCompositeStatementParse, &astBuilder) },
		{ "OnCompositeStatementPartParse", std::bind(&ASTBuilder::OnCompositeStatementPartParse, &astBuilder) },
		{ "OnIntegerTypeParse", std::bind(&ASTBuilder::OnIntegerTypeParse, &astBuilder) },
		{ "OnFloatTypeParse", std::bind(&ASTBuilder::OnFloatTypeParse, &astBuilder) },
		{ "OnBoolTypeParse", std::bind(&ASTBuilder::OnBoolTypeParse, &astBuilder) },
		{ "OnBinaryPlusParse", std::bind(&ASTBuilder::OnBinaryPlusParse, &astBuilder) },
		{ "OnBinaryMinusParse", std::bind(&ASTBuilder::OnBinaryMinusParse, &astBuilder) },
		{ "OnBinaryMulParse", std::bind(&ASTBuilder::OnBinaryMulParse, &astBuilder) },
		{ "OnBinaryDivParse", std::bind(&ASTBuilder::OnBinaryDivParse, &astBuilder) },
		{ "OnIdentifierParse", std::bind(&ASTBuilder::OnIdentifierParse, &astBuilder) },
		{ "OnIntegerConstantParse", std::bind(&ASTBuilder::OnIntegerConstantParse, &astBuilder) },
		{ "OnFloatConstantParse", std::bind(&ASTBuilder::OnFloatConstantParse, &astBuilder) },
		{ "OnUnaryMinusParse", std::bind(&ASTBuilder::OnUnaryMinusParse, &astBuilder) },
	};

	while (true)
	{
		auto state = m_table->GetEntry(index);
		std::cout << index << std::endl;


		if (state->isAttribute)
		{
			auto it = actions.find(state->name);
			if (it != actions.end())
			{
				it->second();
			}
			else
			{
				throw std::logic_error("attribute '" + state->name + "' doesn't have action");
			}
		}
		else if (!EntryAcceptsTerminal(*state, TokenTypeToString(token.type)))
		{
			if (!state->isError)
			{
				++index;
				continue;
			}
			else
			{
				return nullptr;
			}
		}

		if (state->isEnding)
		{
			assert(addresses.empty());
			return astBuilder.PopBuiltStatement();
		}
		if (state->doPush)
		{
			addresses.push_back(index + 1);
		}
		if (state->doShift)
		{
			token = m_lexer->GetNextToken();
		}

		if (state->next != std::nullopt)
		{
			index = *state->next;
		}
		else
		{
			assert(!addresses.empty());
			index = addresses.back();
			addresses.pop_back();
		}
	}

	assert(false);
	throw std::logic_error("LLParser::Parse - while loop doesn't have a break statement here");
}
