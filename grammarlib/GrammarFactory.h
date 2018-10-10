#pragma once
#include "GrammarFwd.h"
#include <memory>
#include <string>

class GrammarFactory
{
public:
	std::unique_ptr<Grammar> CreateGrammar(const std::string& text);
};
