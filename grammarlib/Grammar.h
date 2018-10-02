#pragma once
#include "GrammarProduction.h"

// TODO: add remove production method
class Grammar
{
public:
	void AddProduction(std::shared_ptr<GrammarProduction> production);
	std::shared_ptr<const GrammarProduction> GetProduction(size_t index)const;

	size_t GetProductionsCount()const;
	const std::string& GetStartSymbol()const;

private:
	std::vector<std::shared_ptr<GrammarProduction>> m_productions;
	std::vector<std::pair<std::string, int>> m_tokens;
};
