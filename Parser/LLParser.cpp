#include "stdafx.h"
#include "LLParser.h"

namespace
{
const std::string END_OF_CHAIN_SYMBOL = "end";
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

bool LLParser::Parse(const std::string& text)
{
	(void)text;
	return false;
}

std::unique_ptr<LLParser> CreateLLParser(const Grammar& grammar)
{
	auto parser = std::make_unique<LLParser>();

	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		auto state = std::make_shared<LLParser::State>();
		state->shift = false;
		state->push = false;
		state->error = !ProductionHasNextAlternative(grammar, i);
		state->end = false;
		state->beginnings = GatherBeginningSymbolsOfProduction(grammar, int(i));
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
				state->shift = true;
				state->push = false;
				state->error = true;
				state->end = symbol.GetText() == END_OF_CHAIN_SYMBOL;
				state->next = (col == production->GetSymbolsCount() - 1u) ?
					std::nullopt : std::make_optional<size_t>(parser->GetStatesCount() + 1u);
				state->beginnings = { symbol.GetText() };
				break;
			case GrammarSymbolType::Nonterminal:
				state->shift = false;
				state->push = col < (production->GetSymbolsCount() - 1u);
				state->error = true;
				state->end = false;
				state->next = GetProductionIndex(grammar, symbol.GetText());
				state->beginnings = GatherBeginningSymbolsOfNonterminalEx(grammar, symbol.GetText());
			case GrammarSymbolType::Epsilon:
				state->shift = false;
				state->push = false;
				state->error = true;
				state->end = false;
				state->next = std::nullopt;
				state->beginnings = GatherBeginningSymbolsOfProduction(grammar, int(row));
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
