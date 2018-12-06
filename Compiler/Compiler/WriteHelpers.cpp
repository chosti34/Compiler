#include "stdafx.h"
#include "WriteHelpers.h"

#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtils.h"

#include "../Utils/StreamUtils.h"
#include "../Utils/FormatUtils.h"
#include "../Utils/StringUtils.h"

#include "../Parser/LLParserTable.h"

void WriteGrammar(const Grammar& grammar, std::ostream& out)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		out << "<" << production->GetLeftPart() << ">" << " -> ";

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const auto& symbol = production->GetSymbol(col);

			boost::format fmt(symbol.GetType() == GrammarSymbolType::Nonterminal ? "<%1%>" : "%1%");
			out << fmt % symbol.GetText();

			if (symbol.GetAttribute() != boost::none)
			{
				out << " {" << *symbol.GetAttribute() << "}";
			}
			if (col != (production->GetSymbolsCount() - 1))
			{
				out << " ";
			}
		}

		stream_utils::PrintIterable(
			out, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
	}
}

void WriteParserTable(const LLParserTable& parserTable, std::ostream& out)
{
	format_utils::Table formatTable;
	formatTable.Append({ "Index", "Name", "Shift", "Push", "Error", "End", "Next", "Beginnings" });

	const auto toString = [](bool value) -> std::string {
		return value ? "true" : "false";
	};

	for (size_t i = 0; i < parserTable.GetEntriesCount(); ++i)
	{
		auto entry = parserTable.GetEntry(i);
		formatTable.Append({ std::to_string(i), entry->name,
			toString(entry->doShift), toString(entry->doPush),
			toString(entry->isError), toString(entry->isEnding),
			entry->next ? std::to_string(*entry->next) : "none",
			string_utils::JoinStrings(entry->beginnings, ", ", "{ ", " }") });
	}

	using namespace format_utils;
	formatTable.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
	out << formatTable << std::endl;
}
