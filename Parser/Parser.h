#pragma once
#include "IParser.h"

#include "../grammarlib/LLParsingTable.h"
#include "../Lexer/ILexer.h"

#include <unordered_map>
#include <unordered_set>

struct ParserState
{
	bool shift;
	bool error;
	bool push;
	bool end;
	std::string name;
	std::optional<size_t> next;
	std::unordered_set<TokenKind> beginnings;
};

class Parser : public IParser
{
public:
	using TokensMap = std::unordered_map<std::string, TokenKind>;

	explicit Parser(std::unique_ptr<ILexer> && lexer);
	void SetParsingTable(const LLParsingTable& table, const TokensMap& tokensMap);

	bool Parse(const std::string& text) override;

private:
	std::unique_ptr<ILexer> m_lexer;
	std::vector<ParserState> m_states;
};
