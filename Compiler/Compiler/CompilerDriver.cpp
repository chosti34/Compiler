#include "stdafx.h"
#include "CompilerDriver.h"
#include "CodegenVisitor.h"
#include "../grammarlib/Grammar.h"
#include "../grammarlib/GrammarBuilder.h"
#include "../grammarlib/GrammarProductionFactory.h"
#include "../Lexer/Lexer.h"
#include "../Parser/LLParser.h"
#include "../Parser/LLParserTable.h"
#include "../Utils/file_utils.h"

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
				!TokenTypeExists(symbol.GetText()))
			{
				unmatch = symbol.GetText();
				return false;
			}
		}
	}
	return true;
}

std::unique_ptr<LLParser> CreateParser()
{
	auto grammar = GrammarBuilder(std::make_unique<GrammarProductionFactory>())
		.AddProduction("<Program>          -> <FunctionList> EndOfFile")
		// Функции
		.AddProduction("<FunctionList>               -> <Function> <FunctionList>")
		.AddProduction("<FunctionList>               -> #Eps#")
		.AddProduction("<Function>                   -> Func <Identifier> LeftParenthesis <ParamList> RightParenthesis <OptionalFunctionReturnType> Colon <Statement> {OnFunctionParsed}")
		.AddProduction("<OptionalFunctionReturnType> -> Arrow <Type> {OnFunctionReturnTypeParsed}")
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
		.AddProduction("<Type>             -> Array LeftBracket <ArrayType>")
		.AddProduction("<ArrayType>        -> Int RightBracket {OnArrayIntTypeParsed}")
		.AddProduction("<ArrayType>        -> Float RightBracket {OnArrayFloatTypeParsed}")
		.AddProduction("<ArrayType>        -> Bool RightBracket {OnArrayBoolTypeParsed}")
		.AddProduction("<ArrayType>        -> String RightBracket {OnArrayStringTypeParsed}")
		// Инструкции
		.AddProduction("<Statement>        -> <Condition>")
		.AddProduction("<Statement>        -> <Loop>")
		.AddProduction("<Statement>        -> <Decl>")
		.AddProduction("<Statement>        -> <Return>")
		.AddProduction("<Statement>        -> <Composite>")
		.AddProduction("<Statement>        -> <Print>")
		.AddProduction("<Statement>        -> <Scan>")
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
		// Встроенные в язык функции
		.AddProduction("<Print>            -> Print LeftParenthesis {PrepareFnCallParamsParsing} <FunctionCallParamList> RightParenthesis Semicolon {OnPrintStatementParsed}")
		.AddProduction("<Scan>             -> Scan LeftParenthesis {PrepareFnCallParamsParsing} <FunctionCallParamList> RightParenthesis Semicolon {OnScanStatementParsed}")
		// Инструкция, начинающаяся с идентификатора (присваивание, либо вызов функции)
		.AddProduction("<StmtStartsWithId> -> <Identifier> <AfterIdStmt>")
		.AddProduction("<AfterIdStmt>      -> LeftSquareBracket <Expression> RightSquareBracket Assign <Expression> Semicolon {OnArrayElementAssignStatement}")
		.AddProduction("<AfterIdStmt>      -> Assign <Expression> Semicolon {OnAssignStatementParsed}")
		.AddProduction("<AfterIdStmt>      -> LeftParenthesis {PrepareFnCallParamsParsing} <FunctionCallParamList> RightParenthesis Semicolon {OnFunctionCallStatementParsed}")
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
		.AddProduction("<EqualsExprTail>   -> NotEquals <LessThanExpr> {OnBinaryNotEqualsParsed} <EqualsExprTail>")
		.AddProduction("<EqualsExprTail>   -> #Eps#")
		.AddProduction("<LessThanExpr>     -> <AddSubExpr> <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> LeftBracket <AddSubExpr> {OnBinaryLessParsed} <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> RightBracket <AddSubExpr> {OnBinaryMoreParsed} <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> LessOrEquals <AddSubExpr> {OnBinaryLessOrEqualsParsed} <LessThanExprTail>")
		.AddProduction("<LessThanExprTail> -> MoreOrEquals <AddSubExpr> {OnBinaryMoreOrEqualsParsed} <LessThanExprTail>")
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
		.AddProduction("<AtomExpr>         -> LeftSquareBracket <ArrayExpressionList> RightSquareBracket {OnArrayLiteralConstantParsed}")
		.AddProduction("<AfterIdExpr>      -> LeftParenthesis {PrepareFnCallParamsParsing} <FunctionCallParamList> RightParenthesis {OnFunctionCallExprParsed}")
		.AddProduction("<AfterIdExpr>      -> LeftSquareBracket <Expression> RightSquareBracket {ArrayElementAccess}")
		.AddProduction("<AfterIdExpr>      -> #Eps#")
		// Вспомогательные правила
		//  Список параметров для вызова функций
		.AddProduction("<FunctionCallParamList>       -> <FunctionCallParamListMember> <FunctionCallParamListTail>")
		.AddProduction("<FunctionCallParamList>       -> #Eps#")
		.AddProduction("<FunctionCallParamListTail>   -> Comma <FunctionCallParamListMember> <FunctionCallParamListTail>")
		.AddProduction("<FunctionCallParamListTail>   -> #Eps#")
		.AddProduction("<FunctionCallParamListMember> -> <Expression> {OnFunctionCallParamListMemberParsed}")
		//  Идентификатор
		.AddProduction("<Identifier>            -> Identifier {OnIdentifierParsed}")
		.Build();

	std::string unmatch;
	if (VerifyGrammarTerminalsMatchLexerTokens(*grammar, unmatch))
	{
		return std::make_unique<LLParser>(std::make_unique<Lexer>(), CreateParserTable(*grammar), std::cout);
	}
	throw std::logic_error("lexer doesn't know about '" + unmatch + "' token, but grammar does");
}
}

CompilerDriver::CompilerDriver(std::ostream& log)
	: m_log(log)
{
}

void CompilerDriver::Compile(const std::string & text)
{
	auto parser = CreateParser();
	auto ast = parser->Parse(text);

	if (!ast)
	{
		throw std::runtime_error("ast can't be generated...");
	}

	Codegen generator(m_context);
	generator.Generate(*ast);
}

void CompilerDriver::SaveObjectCodeToFile(const std::string& filepath)
{
	auto targetTriple = llvm::sys::getDefaultTargetTriple();

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	std::string error;
	auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
	llvm::Triple hostTriple(targetTriple);

	// Print an error and exit if we couldn't find the requested target.
	// This generally occurs if we've forgotten to initialise the
	// TargetRegistry or we have a bogus target triple.
	if (!target)
	{
		throw std::runtime_error(error);
	}

	auto cpu = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto rm = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

	std::error_code errorCode;
	llvm::raw_fd_ostream dest(filepath.c_str(), errorCode, llvm::sys::fs::F_None);

	if (errorCode)
	{
		throw std::runtime_error("Could not open file: " + errorCode.message());
	}

	llvm::legacy::PassManager pass;

	llvm::TargetLibraryInfoImpl TLII(hostTriple);
	pass.add(new llvm::TargetLibraryInfoWrapperPass(TLII));

	// Передаём в модуль IR-кода данные целевой платформы.
	llvm::Module& llvmModule = m_context.GetUtils().GetModule();
	llvmModule.setDataLayout(targetMachine->createDataLayout());

	if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile, false, nullptr))
	{
		throw std::runtime_error("TargetMachine can't emit a file of this type");
	}

	pass.run(llvmModule);
	dest.flush();
}

void CompilerDriver::SaveIRToFile(const std::string& filepath)
{
	auto file = file_utils::OpenFileForWriting(filepath);
	CodegenUtils& codegenUtils = m_context.GetUtils();
	llvm::Module& llvmModule = codegenUtils.GetModule();
	llvm::raw_os_ostream os(*file);
	llvmModule.print(os, nullptr);
}
