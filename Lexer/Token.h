#pragma once
#include "TokenKind.h"
#include <optional>
#include <string>

struct Token
{
public:
	explicit Token(TokenKind kind, std::optional<std::string> value = std::nullopt)
		: kind(kind)
		, value(std::move(value))
	{
	}

	// TODO: add offset, line, column properties
	TokenKind kind;
	std::optional<std::string> value;
};

std::string ToString(const Token& token);
