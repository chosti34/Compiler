#include "stdafx.h"
#include "GrammarBuilder.h"

#include "Grammar.h"
#include "GrammarProductionFactory.h"

GrammarBuilder::GrammarBuilder(std::unique_ptr<GrammarProductionFactory> && factory)
	: m_factory(std::move(factory))
{
}

GrammarBuilder& GrammarBuilder::AddProduction(const std::string& line)
{
	m_productions.push_back(std::move(m_factory->CreateProduction(line)));
	return *this;
}

std::unique_ptr<Grammar> GrammarBuilder::Build()const
{
	auto grammar = std::make_unique<Grammar>();
	for (const auto& production : m_productions)
	{
		grammar->AddProduction(production);
	}
	return grammar;
}
