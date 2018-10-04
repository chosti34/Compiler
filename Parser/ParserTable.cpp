#include "stdafx.h"
#include "ParserTable.h"

#include "../grammarlib/GrammarUtil.h"
#include "../Lexer/Lexer.h"

namespace
{
// Имеет ли данный нетерминал альтернативы с большим индексом (ниже по списку правил)
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

// Получить индекс продукции с данным нетерминалом в левой части
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

// Получить направляющее множество для нетерминала, и, если он может быть пустым, добавить к направляющему множеству символы следователи
std::set<std::string> GatherBeginSetAndFollowIfHasEmptiness(const Grammar& grammar, const std::string& nonterminal)
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
}

void ParserTable::AddState(std::shared_ptr<ParserTableEntry> state)
{
	m_table.push_back(std::move(state));
}

std::shared_ptr<ParserTableEntry> ParserTable::GetState(size_t index)
{
	if (index >= m_table.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_table[index];
}

std::shared_ptr<const ParserTableEntry> ParserTable::GetState(size_t index)const
{
	if (index >= m_table.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_table[index];
}

size_t ParserTable::GetStatesCount()const
{
	return m_table.size();
}

std::unique_ptr<ParserTable> ParserTable::Create(const Grammar& grammar)
{
	auto table = std::make_unique<ParserTable>();

	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		auto entry = std::make_shared<ParserTableEntry>();
		entry->name = grammar.GetProduction(i)->GetLeftPart();
		entry->shift = false;
		entry->push = false;
		entry->error = !ProductionHasAlternative(grammar, i);
		entry->end = false;
		entry->beginnings = GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(i));
		entry->next = std::nullopt; //! will be added later
		table->AddState(std::move(entry));
	}

	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const auto& symbol = production->GetSymbol(col);
			auto state = std::make_shared<ParserTableEntry>();

			switch (symbol.GetType())
			{
			case GrammarSymbolType::Terminal:
				state->name = symbol.GetText();
				state->shift = true;
				state->push = false;
				state->error = true;
				state->end = (symbol.GetText() == grammar.GetEndSymbol());
				state->next = (col == production->GetSymbolsCount() - 1u) ?
					std::nullopt : std::make_optional<size_t>(table->GetStatesCount() + 1u);
				state->beginnings = { symbol.GetText() };
				break;
			case GrammarSymbolType::Nonterminal:
				state->name = symbol.GetText();
				state->shift = false;
				state->push = col < (production->GetSymbolsCount() - 1u);
				state->error = true;
				state->end = false;
				state->next = GetProductionIndex(grammar, symbol.GetText());
				state->beginnings = GatherBeginSetAndFollowIfHasEmptiness(grammar, symbol.GetText());
				break;
			case GrammarSymbolType::Epsilon:
				state->name = symbol.GetText();
				state->shift = false;
				state->push = false;
				state->error = true;
				state->end = false;
				state->next = std::nullopt;
				state->beginnings = GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(row));
				break;
			default:
				assert(false);
				throw std::logic_error("CreateParser: default switch branch should be unreachable");
			}

			table->AddState(std::move(state));
		}

		//! adding index that we skipped on first loop
		table->GetState(row)->next = table->GetStatesCount() - production->GetSymbolsCount();
	}

	return table;
}
