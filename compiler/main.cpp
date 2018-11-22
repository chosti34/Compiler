#include "../utils/file_utils.h"
#include "../utils/format_utils.h"
#include "../utils/string_utils.h"
#include "../utils/stream_utils.h"

#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtil.h"
#include "../grammarlib/GrammarBuilder.h"

#include "../lexer/Lexer.h"

#include "LLParser.h"
#include "LLParserTable.h"
#include "SemanticsVerifier.h"
#include "LLVMCodeGenerator.h"

#include <iostream>

namespace
{
bool GrammarTerminalsMatchLexerTokens(const Grammar& grammar)
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

void WriteGrammar(const Grammar& grammar, std::ostream& os)
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

        stream_utils::PrintIterable(os, GatherBeginningSymbolsOfProduction(grammar, int(row)), ", ", " / {", "}");
    }
}

void WriteParserTable(const LLParserTable& parserTable, std::ostream& os)
{
    format_utils::Table formatTable;
    formatTable.Append({ "Index", "Name", "Shift", "Push", "Error", "End", "Next", "Beginnings" });

    const auto toString = [](bool value) -> std::string {
        return value ? "true" : "false";
    };

    for (size_t i = 0; i < parserTable.GetEntriesCount(); ++i)
    {
        auto entry = parserTable.GetEntry(i);
        formatTable.Append({std::to_string(i), entry->name,
        toString(entry->doShift), toString(entry->doPush),
        toString(entry->isError), toString(entry->isEnding),
        entry->next ? std::to_string(*entry->next) : "none",
        string_utils::JoinStrings(entry->beginnings, ", ", "{ ", " }")});
    }

    using namespace format_utils;
    formatTable.SetDisplayMethod(Table::DisplayMethod::ColumnsLineSeparated);
    os << formatTable << std::endl;
}

void WriteTokens(const std::string& text, std::ostream& os)
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
    } while (true);
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

std::unique_ptr<LLParser> CreateCalcParser()
{
    auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
        .AddProduction("<Program>       -> <Expr> EndOfFile")
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
    auto parser = CreateCalcParser();

    const std::string code = R"(
    {
        if (1)
        {
            var a: Int;
            var b: Int;
            var c: Int;
            a = ((b + c));
        }

        var b: Float;

        while (1)
        {
            b = 3;
        }
    }
)";

    std::string line;
    if (std::cout << ">>> " && getline(std::cin, line))
    {
        if (auto ast = parser->Parse(line))
        {
            LLVMCodeGenerator generator;
            generator.CodegenFuncReturningExpression(*ast);

            auto &module = generator.GetLLVMModule();
            module.print(llvm::errs(), nullptr);
        }
        else
        {
            std::cout << "AST can't be built..." << std::endl;
        }
    }
}
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

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
