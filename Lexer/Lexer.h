#pragma once
#include "ILexer.h"

class Lexer : public ILexer
{
public:
	Lexer() = default;
	explicit Lexer(const std::string& text);

	Token GetNextToken() override;
	void SetText(const std::string& text) override;

private:
	void SkipWhitespaces();

private:
	std::string m_text;
	size_t m_pos = 0;
};
