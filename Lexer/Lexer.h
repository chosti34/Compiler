#pragma once
#include "ILexer.h"
#include <cctype>
#include <vector>
#include <algorithm>

class Lexer : public ILexer
{
public:
	static const std::vector<std::pair<std::string, TokenType>> TOKENS;

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
				std::string value(1, mText[mPos++]);
				while (std::isdigit(mText[mPos]))
				{
					value += mText[mPos++];
				}

				if (mPos < mText.length() && mText[mPos] == '.')
				{
					value += mText[mPos++];
					while (std::isdigit(mText[mPos]))
					{
						value += mText[mPos++];
					}
					return Token(TokenType::FloatLiteral, value);
				}

				return Token(TokenType::IntegerLiteral, value);
			}
			if (std::isalpha(ch))
			{
				std::string value(1, mText[mPos++]);
				while (std::isalnum(mText[mPos]))
				{
					value += mText[mPos++];
				}

				auto found = std::find_if(TOKENS.begin(), TOKENS.end(), [&value](const auto& pair) {
					return value == pair.first;
				});

				if (found != TOKENS.end())
				{
					return Token(found->second);
				}
				return Token(TokenType::Identifier, value);
			}
			if (std::ispunct(ch))
			{
				if (ch == ';')
				{
					++mPos;
					return Token(TokenType::SemicolonSeparator);
				}
				if (ch == ':')
				{
					++mPos;
					return Token(TokenType::ColonSeparator);
				}
				if (ch == '=')
				{
					++mPos;
					return Token(TokenType::AssignOperator);
				}
				if (ch == ',')
				{
					++mPos;
					return Token(TokenType::CommaSeparator);
				}
				if (ch == '(')
				{
					++mPos;
					return Token(TokenType::LeftParenthesis);
				}
				if (ch == ')')
				{
					++mPos;
					return Token(TokenType::RightParenthesis);
				}
				if (ch == '{')
				{
					++mPos;
					return Token(TokenType::LeftCurly);
				}
				if (ch == '}')
				{
					++mPos;
					return Token(TokenType::RightCurly);
				}
				if (ch == '<')
				{
					++mPos;
					return Token(TokenType::LeftAngleBracket);
				}
				if (ch == '>')
				{
					++mPos;
					return Token(TokenType::RightAngleBracket);
				}
				if (ch == '-')
				{
					++mPos;
					if (mPos < mText.length() && mText[mPos] == '>')
					{
						++mPos;
						return Token(TokenType::ArrowSeparator);
					}
				}
			}
			throw std::runtime_error("illegal character at pos " + std::to_string(mPos) + ": " + mText[mPos]);
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
