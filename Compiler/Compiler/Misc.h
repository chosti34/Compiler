#pragma once
#include <ostream>
#include "../Parser/LLParserFwd.h"
#include "../grammarlib/GrammarFwd.h"

void WriteGrammar(const Grammar& grammar, std::ostream& out);
void WriteParserTable(const LLParserTable& parserTable, std::ostream& out);
