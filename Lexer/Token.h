#pragma once
#include <string>

enum class TokenKind
{
	EndOfFile,
	Regular
};

struct Token
{
	TokenKind kind;
	std::string text;
};
