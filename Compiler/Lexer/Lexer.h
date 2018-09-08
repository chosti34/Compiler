#pragma once
#include "ILexer.h"
#include <sstream>

class Lexer : public ILexer
{
public:
	Token Advance() override
	{
		std::string lexem;
		if (m_is >> lexem)
		{
			return { TokenKind::Regular, lexem };
		}
		return { TokenKind::EndOfFile, "" };
	}

	void SetText(const std::string& text) override
	{
		m_is = std::istringstream(text);
	}

private:
	std::istringstream m_is;
};
