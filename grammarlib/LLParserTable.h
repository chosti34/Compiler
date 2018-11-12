#pragma once
#include "GrammarFwd.h"

#include <set>
#include <memory>
#include <vector>
#include <optional>

class LLParserTable
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

private:
	std::vector<std::shared_ptr<Entry>> m_table;
};

std::unique_ptr<LLParserTable> CreateParserTable(const Grammar& grammar);
