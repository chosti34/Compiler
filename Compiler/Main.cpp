#include "stdafx.h"

#include "../Parser/LLParser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtils.h"
#include "../grammarlib/GrammarProductionFactory.h"
#include "../Lexer/Lexer.h"

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

std::string JoinIterable(
	const std::set<std::string>& iterable,
	const std::string& separator = ",",
	const std::string& prefix = "",
	const std::string& suffix = "")
{
	std::string result(prefix);
	for (auto it = iterable.cbegin(); it != iterable.cend(); ++it)
	{
		result.append(*it);
		if (std::next(it) != iterable.cend())
		{
			result.append(separator);
		}
	}
	return result + suffix;
}

void DumpGrammar(const Grammar& grammar)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		std::cout << "<" << production->GetLeftPart() << ">" << " -> ";

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			if (production->GetSymbol(col).GetType() == GrammarSymbolType::Nonterminal)
			{
				std::cout << "<" << production->GetSymbol(col).GetText() << ">";
			}
			else
			{
				std::cout << production->GetSymbol(col).GetText();
			}

			bool last = col == production->GetSymbolsCount() - 1;
			if (!last)
			{
				std::cout << " ";
			}
		}

		PrintIterable(std::cout, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
	}
}

std::set<std::string> ConvertToStrings(const std::set<TokenType>& beginnings)
{
	std::set<std::string> strings;
	for (const auto& beginning : beginnings)
	{
		strings.insert(TokenTypeToString(beginning));
	}
	return strings;
}

void DumpParser(const LLParser& parser)
{
	FormatUtils::Table table;
	table.Append({ "Index", "Name", "Shift", "Push", "Error", "End", "Next", "Beginnings" });

	const auto boolalpha = [](bool value) -> std::string
	{
		return value ? "true" : "false";
	};

	for (size_t i = 0; i < parser.GetStatesCount(); ++i)
	{
		const auto& state = parser.GetState(i);
		table.Append({
			std::to_string(i), state->name,
			boolalpha(state->shift), boolalpha(state->push),
			boolalpha(state->error), boolalpha(state->end),
			state->next ? std::to_string(*state->next) : "none",
			JoinIterable(ConvertToStrings(state->beginnings), ", ", "{", "}")});
	}

	using namespace FormatUtils;
	table.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
	std::cout << table << std::endl;
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
		auto parser = CreateLLParser(*grammar);
		DumpParser(*parser);
		std::cout << std::boolalpha << parser->Parse(R"(
fun yo() -> Bool: return true;

fun main(args: Array<Int>) -> Int:
{
	var a: Float;
	a = 3.2;
	return 0;
}
)") << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
