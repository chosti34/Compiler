#pragma once
#include "IParser.h"
#include "ParsingTable.h"
#include "../Lexer/TokenKind.h"
#include <unordered_map>
#include <unordered_set>

class Parser : public IParser
{
	struct State
	{
		bool shift;
		bool error;
		bool push;
		bool end;
		std::string name;
		std::optional<size_t> next;
		std::unordered_set<TokenKind> beginnings;
	};

public:
	using TokensMap = std::unordered_map<std::string, TokenKind>;

	explicit Parser(std::unique_ptr<ILexer> && lexer);
	void SetParsingTable(const ParsingTable& table, const TokensMap& tokensMap);

	bool Parse(const std::string& text) override;

private:
	std::unique_ptr<ILexer> m_lexer;
	std::vector<State> m_states;
};
