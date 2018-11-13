#include "stdafx.h"

#include "../Lexer/Lexer.h"
#include "../Parser/LLParser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarBuilder.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../utillib/FileUtil.h"
#include "../utillib/FormatUtil.h"
#include "../utillib/StringUtil.h"
#include "../utillib/StreamUtil.h"

namespace
{
void WriteGrammar(const Grammar &grammar, std::ostream &os = std::cout)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		os << "<" << production->GetLeftPart() << ">" << " -> ";

		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const auto& symbol = production->GetSymbol(col);

			if (symbol.GetType() == GrammarSymbolType::Nonterminal)
			{
				os << "<" << symbol.GetText() << ">";
			}
			else
			{
				os << symbol.GetText();
			}

			const auto attribute = symbol.GetAttribute();
			if (attribute)
			{
				os << " {" << *attribute << "}";
			}

			bool last = col == production->GetSymbolsCount() - 1;
			if (!last)
			{
				os << " ";
			}
		}

		StreamUtil::PrintIterable(os, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
	}
}

void WriteParserTable(const LLParserTable &parserTable, std::ostream &os = std::cout)
{
	FormatUtil::Table formatTable;
	formatTable.Append({ "Index", "Name", "Shift", "Push", "Error", "End", "Next", "Beginnings" });

	const auto boolalpha = [](bool value) -> std::string
	{
		return value ? "true" : "false";
	};

	for (size_t i = 0; i < parserTable.GetEntriesCount(); ++i)
	{
		const auto& entry = parserTable.GetEntry(i);
		formatTable.Append({
			std::to_string(i), entry->name,
			boolalpha(entry->shift), boolalpha(entry->push),
			boolalpha(entry->error), boolalpha(entry->end),
			entry->next ? std::to_string(*entry->next) : "none",
			StringUtil::Join(entry->beginnings, ", ", "{ ", " }")});
	}

	using namespace FormatUtil;
	formatTable.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
	os << formatTable << std::endl;
}

void WriteTokens(const std::string &text, std::ostream &os = std::cout)
{
	os << "Tokens for: '" << text << "'" << std::endl;

	auto lexer = std::make_unique<Lexer>();
	lexer->SetText(text);

	do
	{
		auto token = lexer->GetNextToken();
		os << TokenToString(token) << std::endl;
		if (token.type == Token::EndOfFile)
		{
			break;
		}
	}
	while (true);
}

std::unique_ptr<LLParser> CreateYolangParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>    -> <Expr> EndOfFile")
		.AddProduction("<Expr>       -> <Term> <ExprHelper>")
		.AddProduction("<ExprHelper> -> Plus <Term> {CreateBinaryNodePlus} <ExprHelper>")
		.AddProduction("<ExprHelper> -> Minus <Term> {CreateBinaryNodeMinus} <ExprHelper>")
		.AddProduction("<ExprHelper> -> #Eps#")
		.AddProduction("<Term>       -> <Factor> <TermHelper>")
		.AddProduction("<TermHelper> -> Mul <Factor> {CreateBinaryNodeMul} <TermHelper>")
		.AddProduction("<TermHelper> -> Div <Factor> {CreateBinaryNodeDiv} <TermHelper>")
		.AddProduction("<TermHelper> -> #Eps#")
		.AddProduction("<Factor>     -> LeftParenthesis <Expr> RightParenthesis")
		.AddProduction("<Factor>     -> IntegerConstant {CreateNumberNode}")
		.AddProduction("<Factor>     -> Minus <Factor> {CreateUnaryNodeMinus}")
		.Build();

	auto parser = std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar));

	return parser;
}

void Execute()
{
	auto parser = CreateYolangParser();

	if (auto ast = parser->Parse("123 + (1 - 4)"))
	{
		std::cout << "AST has been successfully built!" << std::endl;
		ExpressionCalculator calculator;
		int value = calculator.Calculate(*ast);
		std::cout << value << std::endl;
	}
	else
	{
		std::cout << "AST can't be built..." << std::endl;
	}
}
}

int main()
{
	try
	{
		Execute();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
