#include "stdafx.h"

#include "../Lexer/Lexer.h"
#include "../Parser/LLParser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarBuilder.h"
#include "../grammarlib/GrammarFactory.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../utillib/FileUtil.h"
#include "../utillib/FormatUtil.h"
#include "../utillib/StringUtil.h"
#include "../utillib/StreamUtil.h"

namespace
{
//const std::string LANGUAGE_GRAMMAR = R"(
//<Program>       -> <FunctionList> EOF {AttributeForEofToken}
//<FunctionList>  -> <Function> <FunctionList>
//<FunctionList>  -> #Eps#
//<Function>      -> FUNC IDENTIFIER LPAREN <ParamList> RPAREN ARROW <Type> COLON <Statement>
//<ParamList>     -> <Param> <TailParamList>
//<ParamList>     -> #Eps#
//<TailParamList> -> COMMA <Param> {ParamAttrubute} <TailParamList>
//<TailParamList> -> #Eps#
//<Param>         -> IDENTIFIER COLON <Type>
//<Type>          -> INT
//<Type>          -> FLOAT
//<Type>          -> BOOL
//<Type>          -> ARRAY LABRACKET <Type> RABRACKET
//<Statement>     -> <Condition>
//<Statement>     -> <Loop>
//<Statement>     -> <Decl>
//<Statement>     -> <Assign>
//<Statement>     -> <Return>
//<Statement>     -> <Composite>
//<Condition>     -> IF LPAREN <Expression> RPAREN <Statement> <OptionalElse>
//<OptionalElse>  -> ELSE <Statement>
//<OptionalElse>  -> #Eps# {ElseAttribute}
//<Loop>          -> WHILE LPAREN <Expression> RPAREN <Statement>
//<Decl>          -> VAR IDENTIFIER COLON <Type> SEMICOLON
//<Assign>        -> IDENTIFIER ASSIGN <Expression> SEMICOLON
//<Return>        -> RETURN <Expression> SEMICOLON
//<Composite>     -> LCURLY <StatementList> RCURLY
//<StatementList> -> <Statement> <StatementList>
//<StatementList> -> #Eps#
//<Expression>    -> IDENTIFIER
//<Expression>    -> INTLITERAL
//<Expression>    -> FLOATLITERAL
//<Expression>    -> TRUE
//<Expression>    -> FALSE
//)";

const std::string MATH_GRAMMAR = R"(
<Program>    -> <Expr> EOF
<Expr>       -> <Term> <ExprHelper>
<ExprHelper> -> PLUS <Term> {CreateBinaryNodePlus} <ExprHelper>
<ExprHelper> -> MINUS <Term> {CreateBinaryNodeMinus} <ExprHelper>
<ExprHelper> -> #Eps#
<Term>       -> <Factor> <TermHelper>
<TermHelper> -> MUL <Factor> {CreateBinaryNodeMul} <TermHelper>
<TermHelper> -> DIV <Factor> {CreateBinaryNodeDiv} <TermHelper>
<TermHelper> -> #Eps#
<Factor>     -> LPAREN <Expr> RPAREN
<Factor>     -> INTLITERAL {CreateNumberNode}
<Factor>     -> MINUS <Factor> {CreateUnaryNodeMinus}
)";

const std::unordered_map<std::string, TokenKind> TOKENS_MAP = {
	{ "EndOfFile", TokenKind::EndOfFile },
	{ "Func", TokenKind::Func },
	{ "Identifier", TokenKind::Identifier },
	{ "LeftParenthesis", TokenKind::LeftParenthesis },
	{ "RightParenthesis", TokenKind::RightParenthesis },
	{ "Arrow", TokenKind::Arrow },
	{ "Colon", TokenKind::Colon },
	{ "Comma", TokenKind::Comma },
	{ "Int", TokenKind::Int },
	{ "Float", TokenKind::Float },
	{ "Bool", TokenKind::Bool },
	{ "Array", TokenKind::Array },
	{ "LeftBracket", TokenKind::LeftBracket },
	{ "RightBracket", TokenKind::RightBracket },
	{ "If", TokenKind::If },
	{ "Else", TokenKind::Else },
	{ "While", TokenKind::While },
	{ "Var", TokenKind::Var },
	{ "Semicolon", TokenKind::Semicolon },
	{ "Assign", TokenKind::Assign },
	{ "Return", TokenKind::Return },
	{ "LeftCurly", TokenKind::LeftCurly },
	{ "RightCurly", TokenKind::RightCurly },
	{ "IntegerConstant", TokenKind::IntegerConstant },
	{ "FloatConstant", TokenKind::FloatConstant },
	{ "True", TokenKind::True },
	{ "False", TokenKind::False },
	{ "Minus", TokenKind::Minus },
	{ "Plus", TokenKind::Plus },
	{ "Mul", TokenKind::Mul },
	{ "Div", TokenKind::Div }
};

void DumpGrammar(std::ostream& os, const Grammar& grammar)
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

void DumpParsingTable(std::ostream& os, const LLParserTable& parserTable)
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

void DebugTokenize(const std::string& text)
{
	std::cout << "Tokens for: '" << text << "'" << std::endl;

	auto lexer = std::make_unique<Lexer>();
	lexer->SetText(text);

	do
	{
		auto token = lexer->GetNextToken();
		std::cout << ToString(token) << std::endl;
		if (token.kind == TokenKind::EndOfFile)
		{
			break;
		}
	} while (true);
}

std::unique_ptr<LLParser> CreateYolangParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>    -> <Expr> EOF")
		.AddProduction("<Expr>       -> <Term> <ExprHelper>")
		.AddProduction("<ExprHelper> -> PLUS <Term> {CreateBinaryNodePlus} <ExprHelper>")
		.AddProduction("<ExprHelper> -> MINUS <Term> {CreateBinaryNodeMinus} <ExprHelper>")
		.AddProduction("<ExprHelper> -> #Eps#")
		.AddProduction("<Term>       -> <Factor> <TermHelper>")
		.AddProduction("<TermHelper> -> MUL <Factor> {CreateBinaryNodeMul} <TermHelper>")
		.AddProduction("<TermHelper> -> DIV <Factor> {CreateBinaryNodeDiv} <TermHelper>")
		.AddProduction("<TermHelper> -> #Eps#")
		.AddProduction("<Factor>     -> LPAREN <Expr> RPAREN")
		.AddProduction("<Factor>     -> INTLITERAL {CreateNumberNode}")
		.AddProduction("<Factor>     -> MINUS <Factor> {CreateUnaryNodeMinus}")
		.Build();

	auto parser = std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar));

	parser->SetTokenMapping(TokenKind::EndOfFile, "EOF");
	parser->SetTokenMapping(TokenKind::Plus, "PLUS");
	parser->SetTokenMapping(TokenKind::Minus, "MINUS");
	parser->SetTokenMapping(TokenKind::Mul, "MUL");
	parser->SetTokenMapping(TokenKind::Div, "DIV");
	parser->SetTokenMapping(TokenKind::LeftParenthesis, "LPAREN");
	parser->SetTokenMapping(TokenKind::RightParenthesis, "RPAREN");
	parser->SetTokenMapping(TokenKind::IntegerConstant, "INTLITERAL");

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
