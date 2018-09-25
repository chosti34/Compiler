#pragma once
#include <string>

enum class TokenType
{
	FunKeyword,
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

std::string TokenTypeToString(TokenType type);
TokenType StringToTokenType(const std::string& str);
