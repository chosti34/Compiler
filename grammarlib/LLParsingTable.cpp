#include "stdafx.h"
#include "LLParsingTable.h"
#include "GrammarUtil.h"

namespace
{
// »меет ли данный нетерминал альтернативы с большим индексом (ниже по списку правил)
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

// ѕолучить индекс продукции с данным нетерминалом в левой части
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

// ѕолучить направл€ющее множество дл€ нетерминала, и, если он может быть пустым, добавить к направл€ющему множеству символы следователи
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

void LLParsingTable::AddEntry(std::shared_ptr<LLParsingTableEntry> state)
{
	m_table.push_back(std::move(state));
}

std::shared_ptr<LLParsingTableEntry> LLParsingTable::GetEntry(size_t index)
{
	if (index >= m_table.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_table[index];
}

std::shared_ptr<const LLParsingTableEntry> LLParsingTable::GetEntry(size_t index)const
{
	if (index >= m_table.size())
	{
		throw std::out_of_range("index must be less than states count");
	}
	return m_table[index];
}

size_t LLParsingTable::GetEntriesCount()const
{
	return m_table.size();
}

std::unique_ptr<LLParsingTable> LLParsingTable::Create(const Grammar& grammar)
{
	// not using std::make_unique because constructor is private
	auto table = std::unique_ptr<LLParsingTable>(new LLParsingTable);

	for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
	{
		auto entry = std::make_shared<LLParsingTableEntry>();
		entry->name = grammar.GetProduction(i)->GetLeftPart();
		entry->shift = false;
		entry->push = false;
		entry->error = !ProductionHasAlternative(grammar, i);
		entry->end = false;
		entry->beginnings = GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(i));
		entry->next = std::nullopt; //! will be added later
		table->AddEntry(std::move(entry));
	}

	const auto createAttributeEntry = [&grammar](int row, int col, bool fromTerminal) {
		assert(grammar.GetProduction(row)->GetSymbol(col).GetAttribute());
		auto entry = std::make_shared<LLParsingTableEntry>();
		entry->shift = fromTerminal;
		entry->push = false;
		entry->error = false;
		entry->end = grammar.GetProduction(row)->GetSymbol(col).GetText() == grammar.GetEndSymbol();
		entry->next = (col == grammar.GetProduction(row)->GetSymbolsCount() - 1u) ?
			std::nullopt : std::make_optional<size_t>(col + 1);
		entry->name = *grammar.GetProduction(row)->GetSymbol(col).GetAttribute();
		return entry;
	};

	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		unsigned attributesCount = 0u;

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const auto& symbol = production->GetSymbol(col);
			auto entry = std::make_shared<LLParsingTableEntry>();

			switch (symbol.GetType())
			{
			case GrammarSymbolType::Terminal:
				entry->name = symbol.GetText();
				entry->shift = !static_cast<bool>(symbol.GetAttribute());
				entry->push = false;
				entry->error = true;
				entry->end = (symbol.GetText() == grammar.GetEndSymbol() && !symbol.GetAttribute());
				entry->next = (col == production->GetSymbolsCount() - 1u && !symbol.GetAttribute()) ?
					std::nullopt : std::make_optional<size_t>(table->GetEntriesCount() + 1u);
				entry->beginnings = { symbol.GetText() };
				break;
			case GrammarSymbolType::Nonterminal:
				entry->name = symbol.GetText();
				entry->shift = false;
				entry->push = col < (production->GetSymbolsCount() - 1u) || symbol.GetAttribute();
				entry->error = true;
				entry->end = false;
				entry->next = GetProductionIndex(grammar, symbol.GetText());
				entry->beginnings = GatherBeginSetAndFollowIfHasEmptiness(grammar, symbol.GetText());
				break;
			case GrammarSymbolType::Epsilon:
				entry->name = symbol.GetText();
				entry->shift = false;
				entry->push = false;
				entry->error = true;
				entry->end = false;
				entry->next = symbol.GetAttribute() ? std::make_optional<size_t>(col + 1u) : std::nullopt;
				entry->beginnings = GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(row));
				break;
			default:
				assert(false);
				throw std::logic_error("CreateParser: default switch branch should be unreachable");
			}

			table->AddEntry(std::move(entry));

			const auto attribute = symbol.GetAttribute();
			if (attribute)
			{
				table->AddEntry(createAttributeEntry(
					static_cast<int>(row),
					static_cast<int>(col),
					symbol.GetType() == GrammarSymbolType::Terminal));
				++attributesCount;
			}
		}

		//! adding index that we skipped on first loop
		table->GetEntry(row)->next = table->GetEntriesCount() - production->GetSymbolsCount() - attributesCount;
	}

	return table;
}
