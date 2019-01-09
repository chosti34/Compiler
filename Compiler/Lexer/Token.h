#pragma once
#include "TokenType.h"
#include <boost/optional.hpp>

struct Token
{
	TokenType type = TokenType::EndOfFile;
	boost::optional<std::string> value = boost::none;
	size_t offset = 0;
	size_t line = 0;
	size_t column = 0;
};

std::string TokenToString(const Token& token);
