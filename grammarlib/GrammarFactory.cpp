#include "stdafx.h"
#include "GrammarFactory.h"
#include "GrammarProductionFactory.h"
#include "Grammar.h"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace
{
bool ConsistsOfWhitespaces(const std::string& str)
{
	return std::all_of(str.begin(), str.end(), std::isspace);
}
}

// TODO: add support for multiline productions
std::unique_ptr<Grammar> GrammarFactory::CreateGrammar(const std::string& text)
{
	std::istringstream is(text);

	auto factory = std::make_unique<GrammarProductionFactory>();
	auto grammar = std::make_unique<Grammar>();

	std::string line;
	while (std::getline(is, line))
	{
		if (line.empty() || ConsistsOfWhitespaces(line))
		{
			continue;
		}
		grammar->AddProduction(factory->CreateProduction(line));
	}

	return grammar;
}
