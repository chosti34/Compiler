#pragma once
#include "TokenType.h"
#include <optional>
#include <string>

struct Token
{
public:
	explicit Token(TokenType type, std::optional<std::string> value = std::nullopt)
		: type(type)
		, value(std::move(value))
	{
	}

	// TODO: add offset, line, column properties
	TokenType type;
	std::optional<std::string> value;
};
