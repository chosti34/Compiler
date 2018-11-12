#include "stdafx.h"
#include "LLParser.h"
#include <iostream>
#include <functional>

namespace
{
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

LLParser::LLParser(std::unique_ptr<ILexer> && lexer, std::unique_ptr<LLParserTable> && table)
	: m_lexer(std::move(lexer))
	, m_table(std::move(table))
{
}

std::unique_ptr<ASTNode> LLParser::Parse(const std::string& text)
{
	m_lexer->SetText(text);
	Token token = m_lexer->GetNextToken();

	std::vector<ASTNode::Ptr> nodes;
	std::vector<size_t> addresses;
	size_t index = 0;

	while (true)
	{
		auto state = m_table->GetEntry(index);

		// if beginnings is empty, this is attribute state
		if (!state->beginnings.empty() && state->beginnings.find(m_tokensMap.at(token.kind)) == state->beginnings.end())
		{
			if (!state->error)
			{
				++index;
				continue;
			}
			else
			{
				return nullptr;
			}
		}

		if (state->beginnings.empty())
		{
			std::cout << state->name << std::endl;
			auto it = ATTRIBUTE_ACTION_MAP.find(state->name);
			if (it != ATTRIBUTE_ACTION_MAP.end())
			{
				it->second(nodes, token);
			}
		}

		if (state->end)
		{
			assert(addresses.empty());
			assert(nodes.size() == 1);
			return std::move(nodes.back());
		}
		if (state->push)
		{
			addresses.push_back(index + 1);
		}
		if (state->shift)
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

void LLParser::SetTokenMapping(TokenKind kind, const std::string& mapping)
{
	m_tokensMap.emplace(kind, mapping);
}
