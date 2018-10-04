#include "stdafx.h"
#include <sstream>

#include "../Lexer/Lexer.h"
#include "../Parser/Parser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../utillib/FileUtil.h"
#include "../utillib/FormatUtil.h"
#include "../utillib/StringUtil.h"
#include "../utillib/StreamUtil.h"

namespace
{
const std::string LANGUAGE_GRAMMAR = R"(<Program>       -> <FunctionList> EOF
<FunctionList>  -> <Function> <FunctionList>
<FunctionList>  -> #Eps#
<Function>      -> FUNC IDENTIFIER LPAREN <ParamList> RPAREN ARROW <Type> COLON <Statement>
<ParamList>     -> <Param> <TailParamList>
<ParamList>     -> #Eps#
<TailParamList> -> COMMA <Param> <TailParamList>
<TailParamList> -> #Eps#
<Param>         -> IDENTIFIER COLON <Type>
<Type>          -> INT
<Type>          -> FLOAT
<Type>          -> BOOL
<Type>          -> ARRAY LABRACKET <Type> RABRACKET
<Statement>     -> <Condition>
<Statement>     -> <Loop>
<Statement>     -> <Decl>
<Statement>     -> <Assign>
<Statement>     -> <Return>
<Statement>     -> <Composite>
<Condition>     -> IF LPAREN <Expression> RPAREN <Statement> <OptionalElse>
<OptionalElse>  -> ELSE <Statement>
<OptionalElse>  -> #Eps#
<Loop>          -> WHILE LPAREN <Expression> RPAREN <Statement>
<Decl>          -> VAR IDENTIFIER COLON <Type> SEMICOLON
<Assign>        -> IDENTIFIER ASSIGN <Expression> SEMICOLON
<Return>        -> RETURN <Expression> SEMICOLON
<Composite>     -> LCURLY <StatementList> RCURLY
<StatementList> -> <Statement> <StatementList>
<StatementList> -> #Eps#
<Expression>    -> IDENTIFIER
<Expression>    -> INTLITERAL
<Expression>    -> FLOATLITERAL
<Expression>    -> TRUE
<Expression>    -> FALSE
)";

const std::unordered_map<std::string, TokenKind> TERMINAL_TO_TOKEN_KIND_MAP = {
	{ "EOF", TokenKind::END_OF_INPUT },
	{ "FUNC", TokenKind::FUNCTION_KEYWORD },
	{ "IDENTIFIER", TokenKind::IDENTIFIER },
	{ "LPAREN", TokenKind::LEFT_PARENTHESIS },
	{ "RPAREN", TokenKind::RIGHT_PARENTHESIS },
	{ "ARROW", TokenKind::ARROW },
	{ "COLON", TokenKind::COLON },
	{ "COMMA", TokenKind::COMMA },
	{ "INT", TokenKind::INT_KEYWORD },
	{ "FLOAT", TokenKind::FLOAT_KEYWORD },
	{ "BOOL", TokenKind::BOOL_KEYWORD },
	{ "ARRAY", TokenKind::ARRAY_KEYWORD },
	{ "LABRACKET", TokenKind::LEFT_ANGLE_BRACKET },
	{ "RABRACKET", TokenKind::RIGHT_ANGLE_BRACKET },
	{ "IF", TokenKind::IF_KEYWORD },
	{ "ELSE", TokenKind::ELSE_KEYWORD },
	{ "WHILE", TokenKind::WHILE_KEYWORD },
	{ "VAR", TokenKind::VAR_KEYWORD },
	{ "SEMICOLON", TokenKind::SEMICOLON },
	{ "ASSIGN", TokenKind::ASSIGN },
	{ "RETURN", TokenKind::RETURN_KEYWORD },
	{ "LCURLY", TokenKind::LEFT_CURLY },
	{ "RCURLY", TokenKind::RIGHT_CURLY },
	{ "INTLITERAL", TokenKind::INT },
	{ "FLOATLITERAL", TokenKind::FLOAT },
	{ "TRUE", TokenKind::TRUE_KEYWORD },
	{ "FALSE", TokenKind::FALSE_KEYWORD }
};

const std::string TEST_CODE_EXAMPLE = "func AlwaysTrueFunc() -> Bool: return True;";

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

void PrintParserTableToStdout(const ParserTable& parser)
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
			StringUtil::Join(state->beginnings, ", ", "{ ", " }")});
	}

	using namespace FormatUtil;
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
		// Чтение грамматики из файла
		auto input = std::istringstream(LANGUAGE_GRAMMAR);
		const auto grammar = CreateGrammar(input);
		PrintGrammarToStdout(*grammar);

		// Генерация таблицы для парсера
		auto table = ParserTable::Create(*grammar);
		PrintParserTableToStdout(*table);

		// Создаём парсер связывая таблицу построенную по грамматике и лексер с его токенами
		const auto parser = std::make_unique<Parser>(
			std::move(table), std::make_unique<Lexer>(), TERMINAL_TO_TOKEN_KIND_MAP);

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
