#include "stdafx.h"
#include "Token.h"
#include <sstream>

std::string ToString(const Token& token)
{
	std::ostringstream os;
	os << "Token(" << ToString(token.kind) << ", " << (token.value ? *token.value + ")" : "None)");
	return os.str();
}
