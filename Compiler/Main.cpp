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
bool GrammarTerminalsMatchLexerTokens(const Grammar &grammar)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		auto production = grammar.GetProduction(row);
		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const GrammarSymbol& symbol = production->GetSymbol(col);
			if (symbol.GetType() == GrammarSymbolType::Terminal &&
				!TokenExists(symbol.GetText()))
			{
				return false;
			}
		}
	}
	return true;
}

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
		auto entry = parserTable.GetEntry(i);
		formatTable.Append({
			std::to_string(i), entry->name,
			boolalpha(entry->doShift), boolalpha(entry->doPush),
			boolalpha(entry->isError), boolalpha(entry->isEnding),
			entry->next ? std::to_string(*entry->next) : "none",
			StringUtil::Join(entry->beginnings, ", ", "{ ", " }")
		});
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
		.AddProduction("<Program>       -> <Statement> EndOfFile")
		.AddProduction("<Statement>     -> <Condition>")
		.AddProduction("<Statement>     -> <Loop>")
		.AddProduction("<Statement>     -> <Decl>")
		.AddProduction("<Statement>     -> <Assign>")
		.AddProduction("<Statement>     -> <Return>")
		.AddProduction("<Statement>     -> <Composite>")
		.AddProduction("<Condition>     -> If LeftParenthesis <Expression> RightParenthesis <Statement> {OnIfStatementParse} <OptionalElse>")
		.AddProduction("<OptionalElse>  -> Else <Statement> {OnOptionalElseClauseParse}")
		.AddProduction("<OptionalElse>  -> #Eps#")
		.AddProduction("<Loop>          -> While LeftParenthesis <Expression> RightParenthesis <Statement> {OnWhileLoopParse}")
		.AddProduction("<Decl>          -> Var Identifier {OnIdentifierParse} Colon <Type> Semicolon {OnVariableDeclarationParse}")
		.AddProduction("<Assign>        -> Identifier {OnIdentifierParse} Assign <Expression> Semicolon {OnAssignStatementParse}")
		.AddProduction("<Return>        -> Return <Expression> Semicolon {OnReturnStatementParse}")
		.AddProduction("<Composite>     -> LeftCurly {OnCompositeStatementBeginParse} <StatementList> RightCurly {OnCompositeStatementParse}")
		.AddProduction("<StatementList> -> <Statement> {OnCompositeStatementPartParse} <StatementList>")
		.AddProduction("<StatementList> -> #Eps#")
		.AddProduction("<Type>          -> Int {OnIntegerTypeParse}")
		.AddProduction("<Type>          -> Float {OnFloatTypeParse}")
		.AddProduction("<Type>          -> Bool {OnBoolTypeParse}")
		.AddProduction("<Expression>    -> <Expr>")
		.AddProduction("<Expr>          -> <Term> <ExprHelper>")
		.AddProduction("<ExprHelper>    -> Plus <Term> {OnBinaryPlusParse} <ExprHelper>")
		.AddProduction("<ExprHelper>    -> Minus <Term> {OnBinaryMinusParse} <ExprHelper>")
		.AddProduction("<ExprHelper>    -> #Eps#")
		.AddProduction("<Term>          -> <Factor> <TermHelper>")
		.AddProduction("<TermHelper>    -> Mul <Factor> {OnBinaryMulParse} <TermHelper>")
		.AddProduction("<TermHelper>    -> Div <Factor> {OnBinaryDivParse} <TermHelper>")
		.AddProduction("<TermHelper>    -> #Eps#")
		.AddProduction("<Factor>        -> LeftParenthesis <Expr> RightParenthesis")
		.AddProduction("<Factor>        -> IntegerConstant {OnIntegerConstantParse}")
		.AddProduction("<Factor>        -> FloatConstant {OnFloatConstantParse}")
		.AddProduction("<Factor>        -> Identifier {OnIdentifierParse}")
		.AddProduction("<Factor>        -> Minus <Factor> {OnUnaryMinusParse}")
		.Build();

	assert(GrammarTerminalsMatchLexerTokens(*grammar));
	return std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar));
}

void Execute()
{
	auto parser = CreateYolangParser();

	auto code = R"(
{
	if (a)
	{
		var a: Int;
		a = 0;
	}

	while (1)
	{
		b = 3;
	}
}
)";

	if (auto ast = parser->Parse(code))
	{
		std::cout << "AST has been successfully built!" << std::endl;
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
