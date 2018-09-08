#pragma once
#include "Token.h"

class ILexer
{
public:
	virtual ~ILexer() = default;
	virtual Token Advance() = 0;
	virtual void SetText(const std::string& text) = 0;
};
