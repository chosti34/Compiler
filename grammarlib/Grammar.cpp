#include "Grammar.h"

void Grammar::AddProduction(std::shared_ptr<GrammarProduction> production)
{
    m_productions.push_back(std::move(production));
}

std::shared_ptr<const GrammarProduction> Grammar::GetProduction(size_t index)const
{
    if (index >= m_productions.size())
    {
        throw std::out_of_range("index must be less than productions count");
    }
    return m_productions[index];
}

size_t Grammar::GetProductionsCount()const
{
    return m_productions.size();
}

const std::string& Grammar::GetStartSymbol()const
{
    if (m_productions.empty())
    {
        throw std::logic_error("add production before getting start symbol");
    }
    return m_productions.front()->GetLeftPart();
}

const std::string& Grammar::GetEndSymbol()const
{
    assert(!m_productions.empty());
    assert(m_productions.front()->GetLastSymbol().GetType() == GrammarSymbolType::Terminal);
    return m_productions.front()->GetLastSymbol().GetText();
}
