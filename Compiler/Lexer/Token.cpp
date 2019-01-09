#include "stdafx.h"
#include "Token.h"

std::string TokenToString(const Token& token)
{
	auto fmt = boost::format("Token(%1%%2%)")
		% TokenTypeToString(token.type)
		% (token.value ? ", " + *token.value : "");
	return fmt.str();
}
