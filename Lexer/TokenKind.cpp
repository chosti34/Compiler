#include "stdafx.h"
#include "TokenKind.h"

namespace
{
const std::unordered_map<TokenKind, std::string> TOKEN_KIND_TO_STRING_MAP = {
	{ TokenKind::END_OF_INPUT, "END_OF_INPUT"},
	{ TokenKind::FUNCTION_KEYWORD, "FUNCTION_KEYWORD" },
	{ TokenKind::INT_KEYWORD, "INT_KEYWORD" },
	{ TokenKind::FLOAT_KEYWORD, "FLOAT_KEYWORD" },
	{ TokenKind::BOOL_KEYWORD, "BOOL_KEYWORD" },
	{ TokenKind::ARRAY_KEYWORD, "ARRAY_KEYWORD" },
	{ TokenKind::IF_KEYWORD, "IF_KEYWORD" },
	{ TokenKind::ELSE_KEYWORD, "ELSE_KEYWORD" },
	{ TokenKind::WHILE_KEYWORD, "WHILE_KEYWORD" },
	{ TokenKind::VAR_KEYWORD, "VAR_KEYWORD" },
	{ TokenKind::RETURN_KEYWORD, "RETURN_KEYWORD" },
	{ TokenKind::TRUE_KEYWORD, "TRUE_KEYWORD" },
	{ TokenKind::FALSE_KEYWORD, "FALSE_KEYWORD" },
	{ TokenKind::IDENTIFIER, "IDENTIFIER" },
	{ TokenKind::INT, "INT" },
	{ TokenKind::FLOAT, "FLOAT" },
	{ TokenKind::ASSIGN, "ASSIGN" },
	{ TokenKind::LEFT_PARENTHESIS, "LEFT_PARENTHESIS" },
	{ TokenKind::RIGHT_PARENTHESIS, "RIGHT_PARENTHESIS" },
	{ TokenKind::LEFT_ANGLE_BRACKET, "LEFT_ANGLE_BRACKET" },
	{ TokenKind::RIGHT_ANGLE_BRACKET, "RIGHT_ANGLE_BRACKET" },
	{ TokenKind::LEFT_CURLY, "LEFT_CURLY" },
	{ TokenKind::RIGHT_CURLY, "RIGHT_CURLY" },
	{ TokenKind::ARROW, "ARROW" },
	{ TokenKind::COLON, "COLON" },
	{ TokenKind::COMMA, "COMMA" },
	{ TokenKind::SEMICOLON, "SEMICOLON" }
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
