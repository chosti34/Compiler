#pragma once
#include "Grammar.h"
#include "GrammarProductionFactory.h"

class GrammarBuilder
{
public:
	GrammarBuilder(std::unique_ptr<GrammarProductionFactory> && factory)
		: m_factory(std::move(factory))
	{
	}

	GrammarBuilder& AddProduction(const std::string& line)
	{
		m_productions.push_back(std::move(m_factory->CreateProduction(line)));
		return *this;
	}

	std::unique_ptr<Grammar> Build()
	{
		auto grammar = std::make_unique<Grammar>();
		for (const auto& production : m_productions)
		{
			grammar->AddProduction(production);
		}
		return grammar;
	}

private:
	std::vector<std::shared_ptr<GrammarProduction>> m_productions;
	std::unique_ptr<GrammarProductionFactory> m_factory;
};
