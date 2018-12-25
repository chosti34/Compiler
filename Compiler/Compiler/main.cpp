#include "stdafx.h"

#include "../Utils/FileUtils.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarUtils.h"
#include "../grammarlib/GrammarBuilder.h"
#include "../grammarlib/GrammarProductionFactory.h"

#include "../Lexer/Lexer.h"
#include "../Parser/LLParser.h"
#include "../Parser/LLParserTable.h"

#include "CodegenVisitor.h"
#include "WriteHelpers.h"

namespace
{
bool VerifyGrammarTerminalsMatchLexerTokens(const Grammar& grammar, std::string& unmatch)
{
	for (size_t row = 0; row < grammar.GetProductionsCount(); ++row)
	{
		const auto production = grammar.GetProduction(row);
		for (size_t col = 0; col < production->GetSymbolsCount(); ++col)
		{
			const GrammarSymbol& symbol = production->GetSymbol(col);
			if (symbol.GetType() == GrammarSymbolType::Terminal &&
				!TokenExists(symbol.GetText()))
			{
				unmatch = symbol.GetText();
				return false;
			}
		}
	}
	return true;
}

std::unique_ptr<LLParser> CreateYolangParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>          -> <FunctionList> EndOfFile")
		// Функции
		.AddProduction("<FunctionList>     -> <Function> <FunctionList>")
		.AddProduction("<FunctionList>     -> #Eps#")
		.AddProduction("<Function>         -> Func <Identifier> LeftParenthesis <ParamList> RightParenthesis <OptionalFunctionReturnType> Colon <Statement> {OnFunctionParsed}")
		.AddProduction("<OptionalFunctionReturnType> -> Arrow <Type> {FunctionReturnType}")
		.AddProduction("<OptionalFunctionReturnType> -> #Eps#")
		// Параметры функции
		.AddProduction("<ParamList>        -> <Param> <ParamListTail>")
		.AddProduction("<ParamList>        -> #Eps#")
		.AddProduction("<ParamListTail>    -> Comma <Param> <ParamListTail>")
		.AddProduction("<ParamListTail>    -> #Eps#")
		.AddProduction("<Param>            -> <Identifier> Colon <Type> {OnFunctionParamParsed}")
		// Типы
		.AddProduction("<Type>             -> Int {OnIntegerTypeParsed}")
		.AddProduction("<Type>             -> Float {OnFloatTypeParsed}")
		.AddProduction("<Type>             -> Bool {OnBoolTypeParsed}")
		.AddProduction("<Type>             -> String {OnStringTypeParsed}")
		// Инструкции
		.AddProduction("<Statement>        -> <Condition>")
		.AddProduction("<Statement>        -> <Loop>")
		.AddProduction("<Statement>        -> <Decl>")
		.AddProduction("<Statement>        -> <Return>")
		.AddProduction("<Statement>        -> <Composite>")
		.AddProduction("<Statement>        -> <Print>")
		.AddProduction("<Statement>        -> <StmtStartsWithId>")
		// Условная инструкция
		.AddProduction("<Condition>        -> If LeftParenthesis <Expression> RightParenthesis <Statement> {OnIfStatementParsed} <OptionalElse>")
		.AddProduction("<OptionalElse>     -> Else <Statement> {OnOptionalElseClauseParsed}")
		.AddProduction("<OptionalElse>     -> #Eps#")
		// Циклическая инструкция
		.AddProduction("<Loop>             -> While LeftParenthesis <Expression> RightParenthesis <Statement> {OnWhileLoopParsed}")
		// Инструкция объявления переменной
		.AddProduction("<Decl>             -> Var <Identifier> Colon <Type> <OptionalAssign> Semicolon {OnVariableDeclarationParsed}")
		.AddProduction("<OptionalAssign>   -> Assign <Expression> {OnOptionalAssignParsed}")
		.AddProduction("<OptionalAssign>   -> #Eps#")
		// Инструкция возврата
		.AddProduction("<Return>           -> Return <ReturnExpression> Semicolon {OnReturnStatementParsed}")
		.AddProduction("<ReturnExpression> -> <Expression> {OnReturnExpression}")
		.AddProduction("<ReturnExpression> -> #Eps#")
		// Композитная инструкция
		.AddProduction("<Composite>        -> LeftCurly {PrepareCompositeStatementParsing} <StatementList> RightCurly {OnCompositeStatementParsed}")
		.AddProduction("<StatementList>    -> <Statement> {OnCompositeStatementPartParsed} <StatementList>")
		.AddProduction("<StatementList>    -> #Eps#")
		// Инструкция печати
		.AddProduction("<Print>            -> Print LeftParenthesis {PrepareFnCallParamsParsing} <ExpressionList> RightParenthesis Semicolon {OnPrintStatementParsed}")
		// Инструкция, начинающаяся с идентификатора (присваивание, либо вызов функции)
		.AddProduction("<StmtStartsWithId> -> <Identifier> <AfterIdStmt>")
		.AddProduction("<AfterIdStmt>      -> LeftSquareBracket <Expression> RightSquareBracket Assign <Expression> Semicolon {OnArrayElementAssignStatement}")
		.AddProduction("<AfterIdStmt>      -> Assign <Expression> Semicolon {OnAssignStatementParsed}")
		.AddProduction("<AfterIdStmt>      -> LeftParenthesis {PrepareFnCallParamsParsing} <ExpressionList> RightParenthesis Semicolon {OnFunctionCallStatementParsed}")
		// Выражения
		.AddProduction("<Expression>       -> <OrExpr>")
		// Логические выражения
		.AddProduction("<OrExpr>           -> <AndExpr> <OrExprTail>")
		.AddProduction("<OrExprTail>       -> Or <AndExpr> {OnBinaryOrParsed} <OrExprTail>")
		.AddProduction("<OrExprTail>       -> #Eps#")
		.AddProduction("<AndExpr>          -> <EqualsExpr> <AndExprTail>")
		.AddProduction("<AndExprTail>      -> And <EqualsExpr> {OnBinaryAndParsed} <AndExprTail>")
		.AddProduction("<AndExprTail>      -> #Eps#")
		.AddProduction("<EqualsExpr>       -> <LessThanExpr> <EqualsExprTail>")
		.AddProduction("<EqualsExprTail>   -> Equals <LessThanExpr> {OnBinaryEqualsParsed} <EqualsExprTail>")
		.AddProduction("<EqualsExprTail>   -> #Eps#")
		.AddProduction("<LessThanExpr>     -> <AddSubExpr> <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> LeftBracket <AddSubExpr> {OnBinaryLessParsed} <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> #Eps#")
		// Арифметические выражения
		.AddProduction("<AddSubExpr>       -> <MulDivExpr> <AddSubExprTail>")
		.AddProduction("<AddSubExprTail>   -> Plus <MulDivExpr> {OnBinaryPlusParsed} <AddSubExprTail>")
		.AddProduction("<AddSubExprTail>   -> Minus <MulDivExpr> {OnBinaryMinusParsed} <AddSubExprTail>")
		.AddProduction("<AddSubExprTail>   -> #Eps#")
		.AddProduction("<MulDivExpr>       -> <AtomExpr> <MulDivExprTail>")
		.AddProduction("<MulDivExprTail>   -> Mul <AtomExpr> {OnBinaryMulParsed} <MulDivExprTail>")
		.AddProduction("<MulDivExprTail>   -> Div <AtomExpr> {OnBinaryDivParsed} <MulDivExprTail>")
		.AddProduction("<MulDivExprTail>   -> Mod <AtomExpr> {OnBinaryModuloParsed} <MulDivExprTail>")
		.AddProduction("<MulDivExprTail>   -> #Eps#")
		.AddProduction("<AtomExpr>         -> LeftParenthesis <Expression> RightParenthesis")
		.AddProduction("<AtomExpr>         -> IntegerConstant {OnIntegerConstantParsed}")
		.AddProduction("<AtomExpr>         -> FloatConstant {OnFloatConstantParsed}")
		.AddProduction("<AtomExpr>         -> Minus <AtomExpr> {OnUnaryMinusParsed}")
		.AddProduction("<AtomExpr>         -> Plus <AtomExpr> {OnUnaryPlusParsed}")
		.AddProduction("<AtomExpr>         -> Negation <AtomExpr> {OnUnaryNegationParsed}")
		.AddProduction("<AtomExpr>         -> <Identifier> <AfterIdExpr>")
		.AddProduction("<AtomExpr>         -> True {OnTrueConstantParsed}")
		.AddProduction("<AtomExpr>         -> False {OnFalseConstantParsed}")
		.AddProduction("<AtomExpr>         -> StringConstant {OnStringConstantParsed}")
		.AddProduction("<AfterIdExpr>      -> LeftParenthesis {PrepareFnCallParamsParsing} <ExpressionList> RightParenthesis {OnFunctionCallExprParsed}")
		.AddProduction("<AfterIdExpr>      -> LeftSquareBracket <Expression> RightSquareBracket {ArrayElementAccess}")
		.AddProduction("<AfterIdExpr>      -> #Eps#")
		// Вспомогательные правила
		.AddProduction("<ExpressionList>   -> <ExprListMember> <ExprListTail>")
		.AddProduction("<ExpressionList>   -> #Eps#")
		.AddProduction("<ExprListTail>     -> Comma <ExprListMember> <ExprListTail>")
		.AddProduction("<ExprListTail>     -> #Eps#")
		.AddProduction("<ExprListMember>   -> <Expression> {OnExprListMemberParsed}")
		.AddProduction("<Identifier>       -> Identifier {OnIdentifierParsed}")
		.Build();

#ifdef _DEBUG
	WriteGrammar(*grammar, std::cout);
#endif

	std::string unmatch;
	if (VerifyGrammarTerminalsMatchLexerTokens(*grammar, unmatch))
	{
		return std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar), std::cout);
	}
	throw std::logic_error("token '" + unmatch + "' that was specified in grammar, doesn't exist in lexer");
}

std::string GetStreamContent(std::istream& is)
{
	std::stringstream strm;
	strm << is.rdbuf();
	return strm.str();
}

void Compile(const std::string& text, std::ostream& out)
{
	auto parser = CreateYolangParser();
	auto ast = parser->Parse(text);

	if (!ast)
	{
		throw std::runtime_error("ast can't be generated...");
	}

	std::cout << "Generating code...\n";
	CodegenContext context;
	Codegen generator(context);

	generator.Generate(*ast);

	llvm::Module& llvmModule = context.GetUtils().GetModule();
	llvm::raw_os_ostream os(out);
	llvmModule.print(os, nullptr);

	auto TargetTriple = llvm::sys::getDefaultTargetTriple();
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string Error;
	auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
	llvm::Triple hostTriple(TargetTriple);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!Target) {
		throw std::runtime_error(Error);
	}

	auto CPU = "generic";
	auto Features = "";

	llvm::TargetOptions opt;
	auto RM = llvm::Optional<llvm::Reloc::Model>();
	auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

	auto Filename = "output.o";
	std::error_code EC;
	llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);

	if (EC)
	{
		throw std::runtime_error("Could not open file: " + EC.message());
	}

	llvm::legacy::PassManager pass;

	llvm::TargetLibraryInfoImpl TLII(hostTriple);
	pass.add(new llvm::TargetLibraryInfoWrapperPass(TLII));

	// Передаём в модуль IR-кода данные целевой платформы.
	llvmModule.setDataLayout(TargetMachine->createDataLayout());

	if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile, false, nullptr)) {
		throw std::runtime_error("TargetMachine can't emit a file of this type");
	}

	pass.run(llvmModule);
	dest.flush();
}
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		auto input = file_utils::OpenFileForReading("input.txt");
		auto output = file_utils::OpenFileForWriting("output.ll");
		Compile(GetStreamContent(*input), *output);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
