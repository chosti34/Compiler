#pragma once
#include "GrammarFwd.h"

#include <set>
#include <memory>
#include <vector>
#include <optional>

struct LLParsingTableEntry
{
	bool shift;
	bool error;
	bool push;
	bool end;
	std::string name;
	std::optional<size_t> next;
	std::set<std::string> beginnings;
};

class LLParsingTable
{
public:
	void AddEntry(std::shared_ptr<LLParsingTableEntry> entry);
	std::shared_ptr<LLParsingTableEntry> GetEntry(size_t index);
	std::shared_ptr<const LLParsingTableEntry> GetEntry(size_t index)const;
	size_t GetEntriesCount()const;

	static std::unique_ptr<LLParsingTable> Create(const Grammar& grammar);

private:
	LLParsingTable() = default;

private:
	std::vector<std::shared_ptr<LLParsingTableEntry>> m_table;
};
