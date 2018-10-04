#pragma once
#include "IParser.h"
#include "ParserTable.h"
#include "../Lexer/TokenKind.h"
#include <unordered_map>
#include <unordered_set>

class Parser : public IParser
{
public:
	using TerminalTokenMapping = std::unordered_map<std::string, TokenKind>;

	explicit Parser(
		std::unique_ptr<ParserTable> && table,
		std::unique_ptr<ILexer> && lexer,
		const TerminalTokenMapping& mapping);

	bool Parse(const std::string& text) override;

private:
	std::vector<std::unordered_set<TokenKind>> m_beginnings;
	std::unique_ptr<ParserTable> m_table;
	std::unique_ptr<ILexer> m_lexer;
};
