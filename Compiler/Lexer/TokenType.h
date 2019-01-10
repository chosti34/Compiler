#pragma once
#include <string>

enum class TokenType
{
	EndOfFile,

	// Keywords
	Func,
	Int,
	Float,
	Bool,
	String,
	Array,
	If,
	Else,
	While,
	Var,
	Return,
	True,
	False,
	Print,
	Scan,

	// Has mutable values
	Identifier,
	IntegerConstant,
	FloatConstant,
	StringConstant,

	Assign,
	LeftParenthesis,
	RightParenthesis,
	LeftAngleBracket,
	RightAngleBracket,
	LeftSquareBracket,
	RightSquareBracket,
	LeftCurlyBrace,
	RightCurlyBrace,
	Arrow,
	Colon,
	Comma,
	Semicolon,

	Plus,
	Minus,
	Mul,
	Div,
	Mod,
	Or,
	And,
	Equals,
	Negation
};

bool TokenTypeExists(const std::string& name);
std::string TokenTypeToString(TokenType type);
TokenType StringToTokenType(const std::string& str);
