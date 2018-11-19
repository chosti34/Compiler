#ifndef COMPILER_60MIN_LLPARSER_H
#define COMPILER_60MIN_LLPARSER_H

#include "IParser.h"
#include "AST.h"

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

#endif //COMPILER_60MIN_LLPARSER_H
