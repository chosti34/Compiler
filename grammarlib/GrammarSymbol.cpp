#include "stdafx.h"
#include "GrammarSymbol.h"

GrammarSymbol::GrammarSymbol(const std::string& text, GrammarSymbolType type)
	: m_text(text)
	, m_type(type)
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
