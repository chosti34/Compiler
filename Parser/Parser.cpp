#include "stdafx.h"
#include "Parser.h"
#include <iostream>
#include <functional>

namespace
{
std::unordered_set<TokenKind> TransformToTokens(
	const std::set<std::string> &strings,
	const std::unordered_map<std::string, TokenKind> &tokensMap)
{
	std::unordered_set<TokenKind> res;
	for (const std::string& str : strings)
	{
		auto it = tokensMap.find(str);
		if (it == tokensMap.end())
		{
			throw std::logic_error("tokens map should have value for key: '" + str + "'");
		}
		res.insert(it->second);
	}
	return res;
}

void ProcessIntegerLiteral(std::vector<ASTNode::Ptr>& treeStack, const Token& token)
{
	assert(token.kind == TokenKind::IntegerConstant);
	treeStack.push_back(std::make_unique<LeafNumNode>(std::stoi(*token.value)));
}

void ProcessUnaryMinus(std::vector<ASTNode::Ptr>& treeStack, const Token& token)
{
	(void)token;
	auto node = std::move(treeStack.back()); treeStack.pop_back();
	treeStack.push_back(std::make_unique<UnOpNode>(std::move(node), UnOpNode::Minus));
}

void ProcessBinaryOperator(std::vector<ASTNode::Ptr>& treeStack, const Token& token, BinOpNode::Operator op)
{
	(void)token;
	auto right = std::move(treeStack.back()); treeStack.pop_back();
	auto left = std::move(treeStack.back()); treeStack.pop_back();
	treeStack.push_back(std::make_unique<BinOpNode>(std::move(left), std::move(right), op));
}

using namespace std::placeholders;

std::unordered_map<std::string, std::function<void(std::vector<ASTNode::Ptr>&, const Token&)>> ATTRIBUTE_ACTION_MAP = {
	{ "CreateNumberNode", ProcessIntegerLiteral },
	{ "CreateUnaryNodeMinus", ProcessUnaryMinus },
	{ "CreateBinaryNodePlus", std::bind(ProcessBinaryOperator, _1, _2, BinOpNode::Plus) },
	{ "CreateBinaryNodeMinus", std::bind(ProcessBinaryOperator, _1, _2, BinOpNode::Minus) },
	{ "CreateBinaryNodeMul", std::bind(ProcessBinaryOperator, _1, _2, BinOpNode::Mul) },
	{ "CreateBinaryNodeDiv", std::bind(ProcessBinaryOperator, _1, _2, BinOpNode::Div) }
};
}

Parser::Parser(std::unique_ptr<ILexer> && lexer)
	: m_lexer(std::move(lexer))
{
}

void Parser::SetParsingTable(const LLParsingTable& table, const TokensMap& tokensMap)
{
	for (size_t i = 0; i < table.GetEntriesCount(); ++i)
	{
		const auto entry = table.GetEntry(i);
		m_states.push_back(ParserState{
			entry->shift,
			entry->error,
			entry->push,
			entry->end,
			entry->name,
			entry->next,
			TransformToTokens(entry->beginnings, tokensMap)
		});
	}
}

std::unique_ptr<ASTNode> Parser::Parse(const std::string& text)
{
	m_lexer->SetText(text);
	Token token = m_lexer->GetNextToken();

	std::vector<ASTNode::Ptr> nodes;
	std::vector<size_t> addresses;
	size_t index = 0;

	while (true)
	{
		const auto& state = m_states[index];

		// if beginnings is empty, this is attribute state
		if (!state.beginnings.empty() && state.beginnings.find(token.kind) == state.beginnings.end())
		{
			if (!state.error)
			{
				++index;
				continue;
			}
			else
			{
				return nullptr;
			}
		}

		if (state.beginnings.empty())
		{
			std::cout << state.name << std::endl;
			auto it = ATTRIBUTE_ACTION_MAP.find(state.name);
			if (it != ATTRIBUTE_ACTION_MAP.end())
			{
				it->second(nodes, token);
			}
		}

		if (state.end)
		{
			assert(addresses.empty());
			assert(nodes.size() == 1);
			return std::move(nodes.back());
		}
		if (state.push)
		{
			addresses.push_back(index + 1);
		}
		if (state.shift)
		{
			token = m_lexer->GetNextToken();
		}

		if (state.next != std::nullopt)
		{
			index = *state.next;
		}
		else
		{
			assert(!addresses.empty());
			index = addresses.back();
			addresses.pop_back();
		}
	}

	assert(false);
	throw std::logic_error("Parser::Parse - while loop doesn't have a break statement here");
}
