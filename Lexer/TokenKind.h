#pragma once
#include <string>
#include <vector>
#include <utility>

enum class TokenKind
{
	// End of input token kind
	END_OF_INPUT,
	// Keywords
	FUNCTION_KEYWORD,
	INT_KEYWORD,
	FLOAT_KEYWORD,
	BOOL_KEYWORD,
	ARRAY_KEYWORD,
	IF_KEYWORD,
	ELSE_KEYWORD,
	WHILE_KEYWORD,
	VAR_KEYWORD,
	RETURN_KEYWORD,
	TRUE_KEYWORD,
	FALSE_KEYWORD,
	// Expression terms
	IDENTIFIER,
	INT,
	FLOAT,
	// Operators
	ASSIGN,
	// Separators
	LEFT_PARENTHESIS,
	RIGHT_PARENTHESIS,
	LEFT_ANGLE_BRACKET,
	RIGHT_ANGLE_BRACKET,
	LEFT_CURLY,
	RIGHT_CURLY,
	ARROW,
	COLON,
	COMMA,
	SEMICOLON
};

std::string ToString(TokenKind kind);
