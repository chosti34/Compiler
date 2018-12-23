#pragma once
#include <string>
#include <boost/optional.hpp>

struct Token
{
	enum Type
	{
		// Meta
		EndOfFile,

		// Keyword
		Func, Int, Float, Bool, Array, If, Print,
		Else, While, Var, Return, True, False, String,

		// Mutable
		Identifier, IntegerConstant, FloatConstant, StringConstant,

		// Separator
		Assign, LeftParenthesis, RightParenthesis, LeftBracket,
		RightBracket, LeftCurly, RightCurly, Arrow,
		Colon, Comma, Semicolon, LeftSquareBracket, RightSquareBracket,

		// Operator
		Plus, Minus, Mul, Div, Mod,
		Or, And, Equals, Negation
	};

	Type type = EndOfFile;
	boost::optional<std::string> value = boost::none;

	// TODO: fill this properties
	size_t offset = 0;
	size_t line = 0;
	size_t column = 0;
};

bool TokenExists(const std::string& token);
std::string TokenTypeToString(Token::Type type);
Token::Type StringToTokenType(const std::string& str);
std::string TokenToString(const Token& token);
