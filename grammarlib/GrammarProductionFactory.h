#pragma once
#include "GrammarProduction.h"

class GrammarProductionFactory
{
public:
	std::shared_ptr<GrammarProduction> CreateProduction(const std::string& line);
};
