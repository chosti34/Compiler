#pragma once

#include "IParser.h"
#include "../Lexer/ILexer.h"
#include "../Lexer/TokenKind.h"
#include "../grammarlib/Grammar.h"

#include <set>
#include <memory>
#include <vector>
#include <optional>

struct ParserTableEntry
{
	bool shift;
	bool error;
	bool push;
	bool end;
	std::string name;
	std::optional<size_t> next;
	std::set<std::string> beginnings;
};

class ParserTable
{
public:
	void AddState(std::shared_ptr<ParserTableEntry> state);

	std::shared_ptr<ParserTableEntry> GetState(size_t index);
	std::shared_ptr<const ParserTableEntry> GetState(size_t index)const;
	size_t GetStatesCount()const;

	static std::unique_ptr<ParserTable> Create(const Grammar& grammar);

private:
	std::vector<std::shared_ptr<ParserTableEntry>> m_table;
};
