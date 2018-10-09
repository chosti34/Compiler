#include "stdafx.h"
#include "Lexer.h"

#include <cctype>
#include <vector>
#include <algorithm>

namespace
{
const std::unordered_map<std::string, TokenKind> KEYWORDS = {
	{ "func", TokenKind::FUNCTION_KEYWORD },
	{ "Int", TokenKind::INT_KEYWORD },
	{ "Float", TokenKind::FLOAT_KEYWORD },
	{ "Bool", TokenKind::BOOL_KEYWORD },
	{ "Array", TokenKind::ARRAY_KEYWORD },
	{ "if", TokenKind::IF_KEYWORD },
	{ "else", TokenKind::ELSE_KEYWORD },
	{ "while", TokenKind::WHILE_KEYWORD },
	{ "var", TokenKind::VAR_KEYWORD },
	{ "return", TokenKind::RETURN_KEYWORD },
	{ "True", TokenKind::TRUE_KEYWORD },
	{ "False", TokenKind::FALSE_KEYWORD },
};
}

Lexer::Lexer(const std::string& text)
	: mText(text)
{
}

Token Lexer::Advance()
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
				return Token(TokenKind::FLOAT, value);
			}

			return Token(TokenKind::INT, value);
		}
		if (std::isalpha(ch))
		{
			std::string value(1, mText[mPos++]);
			while (std::isalnum(mText[mPos]))
			{
				value += mText[mPos++];
			}

			auto found = std::find_if(KEYWORDS.begin(), KEYWORDS.end(), [&value](const auto& pair) {
				return value == pair.first;
			});

			if (found != KEYWORDS.end())
			{
				return Token(found->second);
			}
			return Token(TokenKind::IDENTIFIER, value);
		}
		if (std::ispunct(ch))
		{
			if (ch == ';')
			{
				++mPos;
				return Token(TokenKind::SEMICOLON);
			}
			if (ch == ':')
			{
				++mPos;
				return Token(TokenKind::COLON);
			}
			if (ch == '=')
			{
				++mPos;
				return Token(TokenKind::ASSIGN);
			}
			if (ch == ',')
			{
				++mPos;
				return Token(TokenKind::COMMA);
			}
			if (ch == '(')
			{
				++mPos;
				return Token(TokenKind::LEFT_PARENTHESIS);
			}
			if (ch == ')')
			{
				++mPos;
				return Token(TokenKind::RIGHT_PARENTHESIS);
			}
			if (ch == '{')
			{
				++mPos;
				return Token(TokenKind::LEFT_CURLY);
			}
			if (ch == '}')
			{
				++mPos;
				return Token(TokenKind::RIGHT_CURLY);
			}
			if (ch == '<')
			{
				++mPos;
				return Token(TokenKind::LEFT_ANGLE_BRACKET);
			}
			if (ch == '>')
			{
				++mPos;
				return Token(TokenKind::RIGHT_ANGLE_BRACKET);
			}
			if (ch == '-')
			{
				++mPos;
				if (mPos < mText.length() && mText[mPos] == '>')
				{
					++mPos;
					return Token(TokenKind::ARROW);
				}
				return Token(TokenKind::MINUS);
			}
			if (ch == '+')
			{
				++mPos;
				return Token(TokenKind::PLUS);
			}
			if (ch == '*')
			{
				++mPos;
				return Token(TokenKind::MUL);
			}
			if (ch == '/')
			{
				++mPos;
				return Token(TokenKind::DIV);
			}
		}
		throw std::runtime_error("illegal character at pos " + std::to_string(mPos) + ": " + mText[mPos]);
	}
	return Token(TokenKind::END_OF_INPUT);
}

void Lexer::SetText(const std::string& text)
{
	mText = text;
	mPos = 0;
}

void Lexer::SkipWhitespaces()
{
	while (mPos < mText.length() && std::isspace(mText[mPos]))
	{
		++mPos;
	}
}
