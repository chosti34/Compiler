#pragma once
#include "IParser.h"
#include "../AST/AST.h"

class ILexer;
class LLParserTable;
class IStatementAST;
class IExpressionAST;

class LLParser : public IParser<std::unique_ptr<IExpressionAST>>
{
public:
	explicit LLParser(std::unique_ptr<ILexer> && lexer, std::unique_ptr<LLParserTable> && table);
	std::unique_ptr<IExpressionAST> Parse(const std::string& text) override;

private:
	std::unique_ptr<ILexer> m_lexer;
	std::unique_ptr<LLParserTable> m_table;
};
