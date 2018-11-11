#include "stdafx.h"
#include "TokenKind.h"

namespace
{
const std::unordered_map<TokenKind, std::string> TOKEN_KIND_TO_STRING_MAP = {
	{ TokenKind::EndOfFile, "END_OF_INPUT"},
	{ TokenKind::Func, "FUNCTION_KEYWORD" },
	{ TokenKind::Int, "INT_KEYWORD" },
	{ TokenKind::Float, "FLOAT_KEYWORD" },
	{ TokenKind::Bool, "BOOL_KEYWORD" },
	{ TokenKind::Array, "ARRAY_KEYWORD" },
	{ TokenKind::If, "IF_KEYWORD" },
	{ TokenKind::Else, "ELSE_KEYWORD" },
	{ TokenKind::While, "WHILE_KEYWORD" },
	{ TokenKind::Var, "VAR_KEYWORD" },
	{ TokenKind::Return, "RETURN_KEYWORD" },
	{ TokenKind::True, "TRUE_KEYWORD" },
	{ TokenKind::False, "FALSE_KEYWORD" },
	{ TokenKind::Identifier, "IDENTIFIER" },
	{ TokenKind::IntegerConstant, "INT" },
	{ TokenKind::FloatConstant, "FLOAT" },
	{ TokenKind::Assign, "ASSIGN" },
	{ TokenKind::LeftParenthesis, "LEFT_PARENTHESIS" },
	{ TokenKind::RightParenthesis, "RIGHT_PARENTHESIS" },
	{ TokenKind::LeftBracket, "LEFT_ANGLE_BRACKET" },
	{ TokenKind::RightBracket, "RIGHT_ANGLE_BRACKET" },
	{ TokenKind::LeftCurly, "LEFT_CURLY" },
	{ TokenKind::RightCurly, "RIGHT_CURLY" },
	{ TokenKind::Arrow, "ARROW" },
	{ TokenKind::Colon, "COLON" },
	{ TokenKind::Comma, "COMMA" },
	{ TokenKind::Semicolon, "SEMICOLON" },
	{ TokenKind::Minus, "MINUS" },
	{ TokenKind::Plus, "PLUS" },
	{ TokenKind::Div, "DIV" },
	{ TokenKind::Mul, "MUL" }
};
}

std::string ToString(TokenKind kind)
{
	auto it = TOKEN_KIND_TO_STRING_MAP.find(kind);
	if (it == TOKEN_KIND_TO_STRING_MAP.end())
	{
		throw std::logic_error("string for passed token kind is undefined");
	}
	return it->second;
}
