#include "stdafx.h"
#include "GrammarProduction.h"

GrammarProduction::GrammarProduction(const std::string& left, const std::vector<GrammarSymbol>& right)
	: m_left(left)
	, m_right(right)
{
	assert(!m_right.empty());
}

const std::string& GrammarProduction::GetLeftPart()const
{
	return m_left;
}

size_t GrammarProduction::GetSymbolsCount()const
{
	return m_right.size();
}

const GrammarSymbol& GrammarProduction::GetSymbol(size_t index)const
{
	if (index >= m_right.size())
	{
		throw std::out_of_range("index must be less than items count");
	}
	return m_right[index];
}

const GrammarSymbol& GrammarProduction::GetStartSymbol()const
{
	assert(!m_right.empty());
	return m_right.front();
}

const GrammarSymbol& GrammarProduction::GetLastSymbol()const
{
	assert(!m_right.empty());
	return m_right.back();
}
