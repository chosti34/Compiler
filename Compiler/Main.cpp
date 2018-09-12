#include "stdafx.h"

#include "../Parser/LLParser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtils.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "Table.h"
#include "FileUtils.h"

namespace
{
std::unique_ptr<Grammar> CreateGrammar(std::istream& strm)
{
	auto grammar = std::make_unique<Grammar>();
	auto factory = std::make_unique<GrammarProductionFactory>();

	std::string line;
	while (getline(strm, line))
	{
		grammar->AddProduction(factory->CreateProduction(line));
	}

	return grammar;
}

// Чтобы файл был закрыт после выхода из функции
std::unique_ptr<Grammar> CreateGrammarFromFile(const std::string& filepath)
{
	auto input = FileUtils::OpenFileForReading(filepath);
	return CreateGrammar(*input);
}

template <typename Iterable>
void PrintIterable(
	std::ostream& output,
	const Iterable& iterable,
	const std::string& separator = " ",
	const std::string& prefix = "",
	const std::string& suffix = "",
	bool newline = true)
{
	output << prefix;
	for (auto it = iterable.cbegin(); it != iterable.cend(); ++it)
	{
		output << *it;
		if (std::next(it) != iterable.cend())
		{
			output << separator;
		}
	}
	output << suffix;
	if (newline)
	{
		output << "\n";
	}
}

void DumpGrammar(const Grammar& grammar)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		std::cout << production->GetLeftPart() << " -> ";

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			std::cout << production->GetSymbol(col).GetText();

			bool last = col == production->GetSymbolsCount() - 1;
			if (!last)
			{
				std::cout << " ";
			}
		}

		PrintIterable(std::cout, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
	}
}
}

int main(int argc, char* argv[])
{
	// TODO: think about compiler flags
	(void)argc;
	(void)argv;

	try
	{
		auto grammar = CreateGrammarFromFile("misc/input.txt");
		DumpGrammar(*grammar);
		// create parser
		// parse text
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
