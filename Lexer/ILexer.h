#pragma once
#include "Token.h"

class ILexer
{
public:
	virtual ~ILexer() = default;
	virtual Token GetNextToken() = 0;
	virtual void SetText(const std::string& text) = 0;
};
