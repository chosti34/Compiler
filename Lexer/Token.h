#pragma once
#include <optional>
#include <string>

struct Token
{
	enum Type
	{
		// Meta
		EndOfFile = 0,
		// Keyword
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
		// Mutable
		Identifier,
		IntegerConstant,
		FloatConstant,
		// Operator
		Assign,
		Plus,
		Minus,
		Mul,
		Div,
		// Separator
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

	Type type = EndOfFile;
	std::optional<std::string> value = std::nullopt;
	size_t offset = 0;
	size_t line = 0;
	size_t column = 0;
};

std::string TokenTypeToString(Token::Type type);
Token::Type StringToTokenType(const std::string &str);
std::string TokenToString(const Token &token);
