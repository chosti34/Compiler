#pragma once

#include "IParser.h"
#include "../Lexer/ILexer.h"
#include "../Lexer/TokenKind.h"
#include "../grammarlib/Grammar.h"

#include <set>
#include <memory>
#include <vector>
#include <optional>

class ParsingTable
{
public:
	struct Entry
	{
		bool shift;
		bool error;
		bool push;
		bool end;
		std::string name;
		std::optional<size_t> next;
		std::set<std::string> beginnings;
	};

	void AddEntry(std::shared_ptr<Entry> entry);

	std::shared_ptr<Entry> GetEntry(size_t index);
	std::shared_ptr<const Entry> GetEntry(size_t index)const;
	size_t GetEntriesCount()const;

	static std::unique_ptr<ParsingTable> Create(const Grammar& grammar);

private:
	std::vector<std::shared_ptr<Entry>> m_table;
};
