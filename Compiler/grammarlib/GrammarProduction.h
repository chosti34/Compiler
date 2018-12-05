#pragma once
#include "GrammarSymbol.h"
#include <vector>

class GrammarProduction
{
public:
	GrammarProduction(const std::string& left, const std::vector<GrammarSymbol>& right);

	const std::string& GetLeftPart()const;

	size_t GetSymbolsCount()const;
	const GrammarSymbol& GetSymbol(size_t index)const;

	const GrammarSymbol& GetStartSymbol()const;
	const GrammarSymbol& GetLastSymbol()const;

private:
	std::string m_left;
	std::vector<GrammarSymbol> m_right;
};
