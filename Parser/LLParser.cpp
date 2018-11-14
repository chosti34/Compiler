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

	void OnAssignStatementParse()
	{
		assert(m_expressions.size() >= 2);
		auto expr = Pop(m_expressions);
		auto baseIdentifier = Pop(m_expressions);

		// Намеренно использую dynamic_cast (можно было бы сохранять идентификаторы в стеке?)
		// Указатели baseIdentifier и derivedIdentifier меняются правом владения
		std::unique_ptr<IdentifierAST> derivedIdentifier;
		if (IdentifierAST* ptr = dynamic_cast<IdentifierAST*>(baseIdentifier.get()))
		{
			baseIdentifier.release();
			derivedIdentifier.reset(ptr);
		}
		else
		{
			assert(false);
		}

		m_statements.push_back(std::make_unique<AssignStatementAST>(
			std::move(derivedIdentifier), std::move(expr)));
	}

	void OnReturnStatementParse()
	{
		assert(!m_expressions.empty());
		m_statements.push_back(std::make_unique<ReturnStatementAST>(Pop(m_expressions)));
	}

	void OnCompositeStatementParse()
	{
		auto composite = std::make_unique<CompositeStatement>();
		for (auto& stmt : m_compositeCache)
		{
			composite->AddStatement(std::move(stmt));
		}
		m_compositeCache.clear();
		m_statements.push_back(std::move(composite));
	}

	void OnCompositeStatementPartParse()
	{
		assert(!m_statements.empty());
		m_compositeCache.push_back(Pop(m_statements));
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
	std::vector<std::unique_ptr<IStatementAST>> m_compositeCache;
	std::vector<ValueType> m_types;
};
}

LLParser::LLParser(std::unique_ptr<ILexer> && lexer, std::unique_ptr<LLParserTable> && table)
	: m_lexer(std::move(lexer))
	, m_table(std::move(table))
{
}

std::unique_ptr<IExpressionAST> LLParser::Parse(const std::string& text)
{
	m_lexer->SetText(text);
	Token token = m_lexer->GetNextToken();

	std::vector<size_t> addresses;
	size_t index = 0;

	ASTBuilder astBuilder(token);
	std::unordered_map<std::string, std::function<void()>> actions = {
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
			return astBuilder.PopBuiltExpression();
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
