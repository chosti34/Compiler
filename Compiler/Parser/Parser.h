#pragma once
#include "IParser.h"
#include "../Lexer/ILexer.h"

class Parser : public IParser
{
public:
	Parser(ILexer& lexer)
		: m_lexer(lexer)
	{
	}

	// Возвращает true, если в цепочке лексер распознает хоть один не конечный токен
	bool Parse(const std::string& text) override
	{
		m_lexer.SetText(text);
		const Token token = m_lexer.Advance();
		return token.kind != TokenKind::EndOfFile;
	}

private:
	ILexer& m_lexer;
};
