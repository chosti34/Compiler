#include "stdafx.h"
#include "LLParser.h"
#include <iostream>
#include <functional>

namespace
{
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
		auto expr = std::move(m_expressions.back()); m_expressions.pop_back();
		return std::move(expr);
	}

	void OnBinaryPlusParse()
	{
		auto right = std::move(m_expressions.back()); m_expressions.pop_back();
		auto left = std::move(m_expressions.back()); m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(
			std::move(left), std::move(right), BinaryExpressionAST::Plus));
	}

	void OnBinaryMinusParse()
	{
		auto right = std::move(m_expressions.back()); m_expressions.pop_back();
		auto left = std::move(m_expressions.back()); m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(
			std::move(left), std::move(right), BinaryExpressionAST::Minus));
	}

	void OnBinaryMulParse()
	{
		auto right = std::move(m_expressions.back()); m_expressions.pop_back();
		auto left = std::move(m_expressions.back()); m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(
			std::move(left), std::move(right), BinaryExpressionAST::Mul));
	}

	void OnBinaryDivParse()
	{
		auto right = std::move(m_expressions.back()); m_expressions.pop_back();
		auto left = std::move(m_expressions.back()); m_expressions.pop_back();
		m_expressions.push_back(std::make_unique<BinaryExpressionAST>(
			std::move(left), std::move(right), BinaryExpressionAST::Div));
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
	const Token &m_token;
	std::vector<std::unique_ptr<IExpressionAST>> m_expressions;
	std::vector<std::unique_ptr<IStatementAST>> m_statements;
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

	auto onAttributeEntry = [&actions](const LLParserTable::Entry &entry) {
		assert(entry.isAttribute);
		auto it = actions.find(entry.name);
		if (it != actions.end())
		{
			auto& action = it->second;
			action();
		}
		else
		{
			throw std::logic_error("attribute '" + entry.name + "' doesn't have action");
		}
	};

	while (true)
	{
		auto state = m_table->GetEntry(index);

		if (state->isAttribute)
		{
			onAttributeEntry(*state);
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
