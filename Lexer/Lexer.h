#pragma once
#include "ILexer.h"
#include <cctype>

class Lexer : public ILexer
{
public:
	Lexer() = default;

	explicit Lexer(const std::string& text)
		: mText(text)
	{
	}

	Token Advance() override
	{
		while (mPos < mText.length())
		{
			char ch = mText[mPos];
			if (std::isspace(ch))
			{
				SkipWhitespaces();
				continue;
			}
			if (std::isdigit(ch))
			{
				// int, float
			}
			if (std::isalpha(ch))
			{
				// identifier, keyword
			}
			if (std::ispunct(ch))
			{
				// operator, separator
			}
		}
		return Token(TokenType::End);
	}

	void SetText(const std::string& text) override
	{
		mText = text;
		mPos = 0;
	}

private:
	void SkipWhitespaces()
	{
		while (mPos < mText.length() && std::isspace(mText[mPos]))
		{
			++mPos;
		}
	}

private:
	std::string mText;
	size_t mPos = 0;
};
