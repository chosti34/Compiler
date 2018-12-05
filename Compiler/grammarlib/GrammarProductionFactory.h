#pragma once
#include "GrammarFwd.h"
#include <memory>

class GrammarProductionFactory
{
public:
	std::shared_ptr<GrammarProduction> CreateProduction(const std::string& line);
};
