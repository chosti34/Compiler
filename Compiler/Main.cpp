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

const std::string MATH_GRAMMAR = R"(<Program> -> <Expr> EOF  
<Expr>       -> <Term> <ExprHelper>
<ExprHelper> -> PLUS <Term> <ExprHelper>
<ExprHelper> -> MINUS <Term> <ExprHelper>
<ExprHelper> -> #Eps#
<Term>       -> <Factor> <TermHelper>
<TermHelper> -> MUL <Factor> <TermHelper>
<TermHelper> -> DIV <Factor> <TermHelper>
<TermHelper> -> #Eps#
<Factor>     -> LPAREN <Expr> RPAREN
<Factor>     -> INTLITERAL
<Factor>     -> MINUS <Factor>
)";

const std::unordered_map<std::string, TokenKind> TOKENS_MAP = {
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
	{ "FALSE", TokenKind::FALSE_KEYWORD },
	{ "MINUS", TokenKind::MINUS },
	{ "PLUS", TokenKind::PLUS },
	{ "MUL", TokenKind::MUL },
	{ "DIV", TokenKind::DIV }
};

const std::string FUNCTION_DEFINITION_CODE_EXAMPLE = "func AlwaysTrueFunc() -> Bool: return True;";
const std::string EXPRESSION_CODE_EXAMPLE = "12 - -1";

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

void PrintParserTableToStdout(const LLParsingTable& parsingTable)
{
	FormatUtil::Table formatTable;
	formatTable.Append({ "Index", "Name", "Shift", "Push", "Error", "End", "Next", "Beginnings" });

	const auto boolalpha = [](bool value) -> std::string
	{
		return value ? "true" : "false";
	};

	for (size_t i = 0; i < parsingTable.GetEntriesCount(); ++i)
	{
		const auto& entry = parsingTable.GetEntry(i);
		formatTable.Append({
			std::to_string(i), entry->name,
			boolalpha(entry->shift), boolalpha(entry->push),
			boolalpha(entry->error), boolalpha(entry->end),
			entry->next ? std::to_string(*entry->next) : "none",
			StringUtil::Join(entry->beginnings, ", ", "{ ", " }")});
	}

	using namespace FormatUtil;
	formatTable.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
	std::cout << formatTable << std::endl;
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
		auto input = std::istringstream(MATH_GRAMMAR);
		const auto grammar = CreateGrammar(input);
		PrintGrammarToStdout(*grammar);

		// Генерация таблицы для парсера
		auto table = LLParsingTable::Create(*grammar);
		PrintParserTableToStdout(*table);

		// Создаём парсер связывая таблицу построенную по грамматике и лексер с его токенами
		const auto parser = std::make_unique<Parser>(std::make_unique<Lexer>());
		parser->SetParsingTable(*table, TOKENS_MAP);

		// Токенизируем текст (для отладки)
		std::cout << EXPRESSION_CODE_EXAMPLE << std::endl;
		auto lexer = std::make_unique<Lexer>();
		lexer->SetText(EXPRESSION_CODE_EXAMPLE);

		do
		{
			auto token = lexer->Advance();
			std::cout << ToString(token) << std::endl;
			if (token.kind == TokenKind::END_OF_INPUT)
			{
				break;
			}
		} while (true);

		// Запускаем парсер
		std::cout << std::boolalpha << parser->Parse(EXPRESSION_CODE_EXAMPLE) << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
