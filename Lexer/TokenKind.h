#pragma once
#include <string>

enum class TokenKind
{
	// Metas
	EndOfFile,

	// Keywords
	Func,
	Int,
	Float,
	Bool,
	Array,
	If,
	Else,
	While,
	Var,
	Return,
	True,
	False,

	// Mutables
	Identifier,
	IntegerConstant,
	FloatConstant,

	// Operators
	Assign,
	Plus,
	Minus,
	Mul,
	Div,

	// Separators
	LeftParenthesis,
	RightParenthesis,
	LeftBracket,
	RightBracket,
	LeftCurly,
	RightCurly,
	Arrow,
	Colon,
	Comma,
	Semicolon
};

std::string ToString(TokenKind kind);
