#pragma once
#include "ILexer.h"

class Lexer : public ILexer
{
public:
	Lexer() = default;
	explicit Lexer(const std::string& text);

	Token Advance() override;
	void SetText(const std::string& text) override;

private:
	void SkipWhitespaces();

private:
	std::string mText;
	size_t mPos = 0;
};
