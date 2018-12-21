#pragma once
#include "IParser.h"
#include "../AST/AST.h"
#include <ostream>

class ILexer;
class LLParserTable;

class LLParser : public IParser<std::unique_ptr<ProgramAST>>
{
public:
	explicit LLParser(
		std::unique_ptr<ILexer> && lexer,
		std::unique_ptr<LLParserTable> && table,
		std::ostream& output);
	std::unique_ptr<ProgramAST> Parse(const std::string& text) override;

private:
	std::unique_ptr<ILexer> m_lexer;
	std::unique_ptr<LLParserTable> m_table;
	std::ostream& m_output;
};
