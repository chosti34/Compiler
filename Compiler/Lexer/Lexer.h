#pragma once
#include "ILexer.h"
#include <boost/optional.hpp>

class Lexer : public ILexer
{
public:
	Lexer() = default;
	explicit Lexer(const std::string& text);

	Token GetNextToken() override;
	void SetText(const std::string& text) override;

private:
	void SkipWhitespaces();
	void SkipUntil(char ch);

	Token OnDigit();
	Token OnAlphaOrUnderscore();
	Token OnPunct();

	void Advance();
	void UpdateCh();

private:
	std::string m_text;
	boost::optional<char> m_ch;
	size_t m_pos = 0;
	size_t m_column = 0;
	size_t m_line = 0;
};
