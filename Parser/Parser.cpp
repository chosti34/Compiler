#include "stdafx.h"
#include "Parser.h"
#include "../Lexer/Lexer.h"

namespace
{
std::vector<std::unordered_set<TokenKind>> TransformBeginnings(const ParserTable& table, const Parser::TerminalTokenMapping& mapping)
{
	std::vector<std::unordered_set<TokenKind>> transformed;
	for (size_t i = 0; i < table.GetStatesCount(); ++i)
	{
		const auto state = table.GetState(i);
		std::unordered_set<TokenKind> beginnings;
		for (const auto& beginning : state->beginnings)
		{
			try
			{
				beginnings.insert(mapping.at(beginning));
			}
			catch (const std::exception& ex)
			{
				throw std::invalid_argument("terminal to token mapping must map '" + beginning + "' to something");
			}
		}
		transformed.push_back(std::move(beginnings));
	}
	return transformed;
}
}

Parser::Parser(
	std::unique_ptr<ParserTable> && table,
	std::unique_ptr<ILexer> && lexer,
	const TerminalTokenMapping& mapping)
	: m_beginnings(TransformBeginnings(*table, mapping))
	, m_table(std::move(table))
	, m_lexer(std::move(lexer))
{
}

bool Parser::Parse(const std::string& text)
{
	m_lexer->SetText(text);

	Token token = m_lexer->Advance();
	std::vector<size_t> stack;
	size_t index = 0;

	while (true)
	{
		const auto state = m_table->GetState(index);

		if (m_beginnings[index].find(token.kind) == m_beginnings[index].end())
		{
			if (!state->error)
			{
				++index;
				continue;
			}
			else
			{
				return false;
			}
		}

		if (state->end)
		{
			assert(stack.empty());
			return true;
		}
		if (state->push)
		{
			stack.push_back(index + 1);
		}
		if (state->shift)
		{
			token = m_lexer->Advance();
		}

		if (state->next != std::nullopt)
		{
			index = *state->next;
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
