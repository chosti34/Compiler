#pragma once
#include <string>
#include <vector>
#include <utility>

enum class TokenType
{
	FunñKeyword,
	IntKeyword,
	FloatKeyword,
	BoolKeyword,
	ArrayKeyword,
	IfKeyword,
	ElseKeyword,
	WhileKeyword,
	VarKeyword,
	ReturnKeyword,
	TrueKeyword,
	FalseKeyword,
	Identifier,
	IntegerLiteral,
	FloatLiteral,
	LeftParenthesis,
	RightParenthesis,
	LeftAngleBracket,
	RightAngleBracket,
	LeftCurly,
	RightCurly,
	ArrowSeparator,
	ColonSeparator,
	CommaSeparator,
	SemicolonSeparator,
	AssignOperator,
	End
};

using GrammarTokens = std::vector<std::pair<std::string, TokenType>>;
const GrammarTokens& GetGrammarTokens();

std::string TokenTypeToString(TokenType type);
TokenType StringToTokenType(const std::string& str);
