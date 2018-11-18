#include "LLParserTable.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/Grammar.h"

namespace
{
// Имеет ли данный нетерминал альтернативы с бОльшим индексом (ниже по списку правил)
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

void LLParserTable::AddEntry(std::shared_ptr<Entry> state)
{
    m_table.push_back(std::move(state));
}

std::shared_ptr<LLParserTable::Entry> LLParserTable::GetEntry(size_t index)
{
    if (index >= m_table.size())
    {
        throw std::out_of_range("index must be less than states count");
    }
    return m_table[index];
}

std::shared_ptr<const LLParserTable::Entry> LLParserTable::GetEntry(size_t index)const
{
    if (index >= m_table.size())
    {
        throw std::out_of_range("index must be less than states count");
    }
    return m_table[index];
}

size_t LLParserTable::GetEntriesCount()const
{
    return m_table.size();
}

std::unique_ptr<LLParserTable> CreateParserTable(const Grammar& grammar)
{
    auto table = std::make_unique<LLParserTable>();

    for (size_t i = 0; i < grammar.GetProductionsCount(); ++i)
    {
        auto entry = std::make_shared<LLParserTable::Entry>();
        entry->name = grammar.GetProduction(i)->GetLeftPart();
        entry->doShift = false;
        entry->doPush = false;
        entry->isError = !ProductionHasAlternative(grammar, i);
        entry->isEnding = false;
        entry->beginnings = GatherBeginningSymbolsOfProduction(grammar, static_cast<int>(i));
        entry->next = boost::none; // Will be added later
        table->AddEntry(std::move(entry));
    }

    const auto createAttributeEntry = [&grammar, &table](int row, int col, bool fromTerminal) {
        auto production = grammar.GetProduction(row);
        assert(production->GetSymbol(col).GetAttribute());
        auto entry = std::make_shared<LLParserTable::Entry>();
        entry->doShift = fromTerminal;
        entry->doPush = false;
        entry->isError = false;
        entry->isEnding = production->GetSymbol(col).GetText() == grammar.GetEndSymbol();
        entry->next = (col == production->GetSymbolsCount() - 1u) ?
                      boost::none : boost::make_optional<size_t>(table->GetEntriesCount() + 1);
        entry->name = *production->GetSymbol(col).GetAttribute();
        entry->isAttribute = true;
        return entry;
    };

    for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
    {
        const auto production = grammar.GetProduction(row);
        unsigned attributesCount = 0u;

        for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
        {
            const auto& symbol = production->GetSymbol(col);
            auto entry = std::make_shared<LLParserTable::Entry>();

            switch (symbol.GetType())
            {
                case GrammarSymbolType::Terminal:
                    entry->name = symbol.GetText();
                    entry->isAttribute = false;
                    entry->doShift = !static_cast<bool>(symbol.GetAttribute());
                    entry->doPush = false;
                    entry->isError = true;
                    entry->isEnding = (symbol.GetText() == grammar.GetEndSymbol() && !symbol.GetAttribute());
                    entry->next = (col == production->GetSymbolsCount() - 1u && !symbol.GetAttribute()) ?
                                  boost::none : boost::make_optional<size_t>(table->GetEntriesCount() + 1u);
                    entry->beginnings = { symbol.GetText() };
                    break;
                case GrammarSymbolType::Nonterminal:
                    entry->name = symbol.GetText();
                    entry->isAttribute = false;
                    entry->doShift = false;
                    entry->doPush = col < (production->GetSymbolsCount() - 1u) || symbol.GetAttribute();
                    entry->isError = true;
                    entry->isEnding = false;
                    entry->next = GetProductionIndex(grammar, symbol.GetText());
                    entry->beginnings = GatherBeginSetAndFollowIfHasEmptiness(grammar, symbol.GetText());
                    break;
                case GrammarSymbolType::Epsilon:
                    entry->name = symbol.GetText();
                    entry->isAttribute = false;
                    entry->doShift = false;
                    entry->doPush = false;
                    entry->isError = true;
                    entry->isEnding = false;
                    entry->next = symbol.GetAttribute() ? boost::make_optional<size_t>(col + 1u) : boost::none;
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

        // Adding index that we skipped on first loop
        table->GetEntry(row)->next = table->GetEntriesCount() - production->GetSymbolsCount() - attributesCount;
    }

    return table;
}

bool EntryAcceptsTerminal(const LLParserTable::Entry& entry, const std::string& terminal)
{
    return entry.beginnings.find(terminal) != entry.beginnings.end();
}
