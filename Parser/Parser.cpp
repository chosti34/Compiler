#include "stdafx.h"
#include "Parser.h"

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

bool Parser::Parse(const std::string& text)
{
	m_lexer->SetText(text);

	Token token = m_lexer->Advance();
	std::vector<size_t> stack;
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
				return false;
			}
		}

		if (state.end)
		{
			assert(stack.empty());
			return true;
		}
		if (state.push)
		{
			stack.push_back(index + 1);
		}
		if (state.shift)
		{
			token = m_lexer->Advance();
		}

		if (state.next != std::nullopt)
		{
			index = *state.next;
		}
		else
		{
			assert(!stack.empty());
			index = stack.back();
			stack.pop_back();
		}
	}

	assert(false);
	throw std::logic_error("while loop doesn't have a break statement here");
}
