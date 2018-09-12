#pragma once
#include <string>

enum class GrammarSymbolType
{
	Terminal,
	Nonterminal,
	Epsilon
};

class GrammarSymbol
{
public:
	GrammarSymbol(const std::string& text, GrammarSymbolType type);

	const std::string& GetText()const;
	GrammarSymbolType GetType()const;

private:
	std::string m_text;
	GrammarSymbolType m_type;
};
