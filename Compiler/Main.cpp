#include "stdafx.h"

#include "../Lexer/Lexer.h"
#include "../Parser/Parser.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarFactory.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../utillib/FileUtil.h"
#include "../utillib/FormatUtil.h"
#include "../utillib/StringUtil.h"
#include "../utillib/StreamUtil.h"

namespace
{
const std::string LANGUAGE_GRAMMAR = R"(
<Program>       -> <FunctionList> EOF {AttributeForEofToken}
<FunctionList>  -> <Function> <FunctionList>
<FunctionList>  -> #Eps#
<Function>      -> FUNC IDENTIFIER LPAREN <ParamList> RPAREN ARROW <Type> COLON <Statement>
<ParamList>     -> <Param> <TailParamList>
<ParamList>     -> #Eps#
<TailParamList> -> COMMA <Param> {ParamAttrubute} <TailParamList>
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
<OptionalElse>  -> #Eps# {ElseAttribute}
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

const std::string MATH_GRAMMAR = R"(
<Program> -> <Expr> EOF
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

const std::string FUNCTION_DEFINITION_CODE_EXAMPLE = R"(
func AlwaysTrueFunc() -> Bool: return 1;

func Main(args: Array<Int>) -> Int:
{
	var a: Int;
	a = 0;
	return a;
}
)";

class GrammarBuilder
{
public:
	GrammarBuilder(std::unique_ptr<GrammarProductionFactory> && factory)
		: m_factory(std::move(factory))
	{
	}

	GrammarBuilder& AddProduction(const std::string& line)
	{
		m_productions.push_back(std::move(m_factory->CreateProduction(line)));
		return *this;
	}

	std::unique_ptr<Grammar> Build()
	{
		auto grammar = std::make_unique<Grammar>();
		for (const auto& production : m_productions)
		{
			grammar->AddProduction(production);
		}
		return grammar;
	}

private:
	std::vector<std::shared_ptr<GrammarProduction>> m_productions;
	std::unique_ptr<GrammarProductionFactory> m_factory;
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

void DumpParsingTable(std::ostream& os, const LLParsingTable& parsingTable)
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
	os << formatTable << std::endl;
}

void DebugTokenize(const std::string& text)
{
	std::cout << "Tokens for: '" << text << "'" << std::endl;

	auto lexer = std::make_unique<Lexer>();
	lexer->SetText(text);

	do
	{
		auto token = lexer->Advance();
		std::cout << ToString(token) << std::endl;
		if (token.kind == TokenKind::END_OF_INPUT)
		{
			break;
		}
	} while (true);
}

std::unique_ptr<Grammar> CreateExpressionGrammar()
{
	return GrammarBuilder(std::make_unique<GrammarProductionFactory>())
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
}

void ExecuteApp()
{
	const auto grammar = CreateExpressionGrammar();
	const auto table = LLParsingTable::Create(*grammar);
	const auto parser = std::make_unique<Parser>(std::make_unique<Lexer>());
	parser->SetParsingTable(*table, TOKENS_MAP);

	const std::string code = "1 + 1 + 1 * 2 - 3 - -3";

#ifdef _DEBUG
	DumpGrammar(std::cout, *grammar);
	DumpParsingTable(std::cout, *table);
	DebugTokenize(code);
#endif

	std::cout << std::boolalpha << parser->Parse(code) << std::endl;

	if (auto ast = parser->GetAST())
	{
		ExpressionCalculator calculator;
		int value = calculator.Calculate(*ast);
		std::cout << value << std::endl;
	}
	else
	{
		std::cout << "AST isn't built" << std::endl;
	}
}
}

int main()
{
	try
	{
		ExecuteApp();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
