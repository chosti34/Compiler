#include "stdafx.h"

#include "../Parser/TableParser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarProductionFactory.h"
#include "../Lexer/Lexer.h"

#include "../utillib/FileUtil.h"
#include "../utillib/FormatUtil.h"
#include "../utillib/StringUtil.h"
#include "../utillib/StreamUtil.h"

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
	using namespace FileUtil;
	const auto input = OpenFileForReading(filepath);
	return CreateGrammar(*input);
}

void PrintGrammarToStdout(const Grammar& grammar)
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

		StreamUtil::PrintIterable(std::cout, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
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

void DumpParser(const TableParser& parser)
{
	FormatUtil::Table table;
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
			StringUtil::Join(ConvertToStrings(state->beginnings), ", ", "{", "}")});
	}

	using namespace FormatUtil;
	table.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
	std::cout << table << std::endl;
}

const std::string TEST_CODE_EXAMPLE = R"(

func AlwaysTrueFunc() -> Bool: return True;

func Main(args: Array<Int>) -> Int:
{
	if (True)
	{
		return 0;
	}
	return 1;
}

)";
}

int main(int argc, char* argv[])
{
	// TODO: think about compiler flags
	(void)argc;
	(void)argv;

	try
	{
		// Чтение грамматики из файла
		auto grammar = CreateGrammarFromFile("misc/input.txt");
		PrintGrammarToStdout(*grammar);

		// Генерация парсера
		auto parser = CreateTableParser(*grammar);
		DumpParser(*parser);

		// Запускаем парсер
		std::cout << std::boolalpha << parser->Parse(TEST_CODE_EXAMPLE) << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
