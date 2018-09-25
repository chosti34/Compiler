#include "stdafx.h"
#include "LLParser.h"

#include "../grammarlib/GrammarUtils.h"
#include "../Lexer/Lexer.h"

#include <stack>
#include <sstream>

namespace
{
const std::string END_OF_CHAIN_SYMBOL = "end";

bool ProductionHasAlternative(const Grammar& grammar, size_t index)
{
	const auto lhs = grammar.GetProduction(index++);
	while (index < grammar.GetProductionsCount())
	{
		const auto rhs = grammar.GetProduction(index++);
		if (lhs->GetLeftPart() == rhs->GetLeftPart())
		{
			return true;
		}
	}
	return false;
}

size_t GetProductionIndex(const Grammar& grammar, const std::string& nonterminal)
{
	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		if (grammar.GetProduction(i)->GetLeftPart() == nonterminal)
		{
			return i;
		}
	}
	throw std::invalid_argument("grammar doesn't have such nonterminal");
}

// Получить направляющее множестводля нетерминала, и, если он может быть пустым, добавить к направляющему множеству символы следователи
std::set<std::string> GatherBeginningsAndFollowingsOnEmptiness(const Grammar& grammar, const std::string& nonterminal)
{
	std::set<std::string> symbols;

	if (NonterminalHasEmptiness(grammar, nonterminal))
	{
		const auto followings = GatherFollowingSymbols(grammar, nonterminal);
		symbols.insert(followings.begin(), followings.end());
	}

	const auto beginnings = GatherBeginningSymbolsOfNonterminal(grammar, nonterminal);
	symbols.insert(beginnings.begin(), beginnings.end());
	return symbols;
}

std::set<TokenType> ConvertToTokenTypes(const std::set<std::string>& terms)
{
	std::set<TokenType> types;
	for (const auto& term : terms)
	{
		types.insert(StringToTokenType(term));
	}
	return types;
}
}

void LLParser::AddState(std::shared_ptr<LLParser::State> state)
{
	m_states.push_back(std::move(state));
}

std::shared_ptr<LLParser::State> LLParser::GetState(size_t index)
{
	if (index >= m_states.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_states[index];
}

std::shared_ptr<const LLParser::State> LLParser::GetState(size_t index)const
{
	if (index >= m_states.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_states[index];
}

size_t LLParser::GetStatesCount()const
{
	return m_states.size();
}

// TODO: use lexer instead of operator >>
bool LLParser::Parse(const std::string& text)
{
	auto lexer = std::make_unique<Lexer>(text);

	size_t index = 0;
	std::stack<size_t> stack;
	Token token = lexer->Advance();

	while (true)
	{
		const auto state = m_states[index];

		if (state->beginnings.find(token.type) == state->beginnings.end())
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
			stack.push(index + 1);
		}

		if (state->shift)
		{
			token = lexer->Advance();
		}

		if (state->next != std::nullopt)
		{
			index = *state->next;
		}
		else
		{
			assert(!stack.empty());
			index = stack.top();
			stack.pop();
		}
	}
}

std::unique_ptr<LLParser> CreateLLParser(const Grammar& grammar)
{
	auto parser = std::make_unique<LLParser>();

	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		auto state = std::make_shared<LLParser::State>();
		state->name = grammar.GetProduction(i)->GetLeftPart();
		state->shift = false;
		state->push = false;
		state->error = !ProductionHasAlternative(grammar, i);
		state->end = false;
		state->beginnings = ConvertToTokenTypes(GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(i)));
		state->next = std::nullopt; // will be added later
		parser->AddState(std::move(state));
	}

	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const auto& symbol = production->GetSymbol(col);
			auto state = std::make_shared<LLParser::State>();

			switch (symbol.GetType())
			{
			case GrammarSymbolType::Terminal:
				state->name = symbol.GetText();
				state->shift = true;
				state->push = false;
				state->error = true;
				state->end = symbol.GetText() == END_OF_CHAIN_SYMBOL;
				state->next = (col == production->GetSymbolsCount() - 1u) ?
					std::nullopt : std::make_optional<size_t>(parser->GetStatesCount() + 1u);
				state->beginnings = ConvertToTokenTypes({ symbol.GetText() });
				break;
			case GrammarSymbolType::Nonterminal:
				state->name = symbol.GetText();
				state->shift = false;
				state->push = col < (production->GetSymbolsCount() - 1u);
				state->error = true;
				state->end = false;
				state->next = GetProductionIndex(grammar, symbol.GetText());
				state->beginnings = ConvertToTokenTypes(GatherBeginningsAndFollowingsOnEmptiness(grammar, symbol.GetText()));
				break;
			case GrammarSymbolType::Epsilon:
				state->name = symbol.GetText();
				state->shift = false;
				state->push = false;
				state->error = true;
				state->end = false;
				state->next = std::nullopt;
				state->beginnings = ConvertToTokenTypes(GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(row)));
				break;
			default:
				assert(false);
				throw std::logic_error("CreateParser: default switch branch should be unreachable");
			}

			parser->AddState(std::move(state));
		}

		// adding index that we skipped on first loop
		parser->GetState(row)->next = parser->GetStatesCount() - production->GetSymbolsCount();
	}

	return parser;
}
