#include "stdafx.h"
#include "GrammarSymbol.h"
#include <cassert>

GrammarSymbol::GrammarSymbol(std::string text, GrammarSymbolType type, std::optional<std::string> attribute /* = nullopt */)
	: m_text(std::move(text))
	, m_type(type)
	, m_attribute(std::move(attribute))
{
}

const std::string& GrammarSymbol::GetText()const
{
	return m_text;
}

GrammarSymbolType GrammarSymbol::GetType()const
{
	return m_type;
}

void GrammarSymbol::SetAttribute(std::string attribute)
{
	m_attribute = std::move(attribute);
}

std::optional<std::string> GrammarSymbol::GetAttribute()const
{
	return m_attribute;
}

std::string ToString(GrammarSymbolType type)
{
	switch (type)
	{
	case GrammarSymbolType::Terminal:
		return "Terminal";
	case GrammarSymbolType::Nonterminal:
		return "Nonterminal";
	case GrammarSymbolType::Epsilon:
		return "Epsilon";
	default:
		assert(false);
		throw std::logic_error("can't cast undefined grammar symbol type to string");
	}
}
