#include "stdafx.h"
#include "Lexer.h"
#include <cctype>
#include <vector>
#include <algorithm>

namespace
{
const std::unordered_map<std::string, TokenKind> KEYWORDS = {
	{ "func", TokenKind::Func },
	{ "Int", TokenKind::Int },
	{ "Float", TokenKind::Float },
	{ "Bool", TokenKind::Bool },
	{ "Array", TokenKind::Array },
	{ "if", TokenKind::If },
	{ "else", TokenKind::Else },
	{ "while", TokenKind::While },
	{ "var", TokenKind::Var },
	{ "return", TokenKind::Return },
	{ "True", TokenKind::True },
	{ "False", TokenKind::False },
};
}

Lexer::Lexer(const std::string& text)
	: m_text(text)
{
}

Token Lexer::GetNextToken()
{
	while (m_pos < m_text.length())
	{
		char ch = m_text[m_pos];
		if (std::isspace(ch))
		{
			SkipWhitespaces();
			continue;
		}
		if (std::isdigit(ch))
		{
			std::string value(1, m_text[m_pos++]);
			while (m_pos < m_text.length() && std::isdigit(m_text[m_pos]))
			{
				value += m_text[m_pos++];
			}

			if (m_pos < m_text.length() && m_text[m_pos] == '.')
			{
				value += m_text[m_pos++];
				while (m_pos < m_text.length() && std::isdigit(m_text[m_pos]))
				{
					value += m_text[m_pos++];
				}
				return Token(TokenKind::FloatConstant, value);
			}

			return Token(TokenKind::IntegerConstant, value);
		}
		if (std::isalpha(ch))
		{
			std::string value(1, m_text[m_pos++]);
			while (std::isalnum(m_text[m_pos]))
			{
				value += m_text[m_pos++];
			}

			auto found = std::find_if(KEYWORDS.begin(), KEYWORDS.end(), [&value](const auto& pair) {
				return value == pair.first;
			});

			if (found != KEYWORDS.end())
			{
				return Token(found->second);
			}
			return Token(TokenKind::Identifier, value);
		}
		if (std::ispunct(ch))
		{
			if (ch == ';')
			{
				++m_pos;
				return Token(TokenKind::Semicolon);
			}
			if (ch == ':')
			{
				++m_pos;
				return Token(TokenKind::Colon);
			}
			if (ch == '=')
			{
				++m_pos;
				return Token(TokenKind::Assign);
			}
			if (ch == ',')
			{
				++m_pos;
				return Token(TokenKind::Comma);
			}
			if (ch == '(')
			{
				++m_pos;
				return Token(TokenKind::LeftParenthesis);
			}
			if (ch == ')')
			{
				++m_pos;
				return Token(TokenKind::RightParenthesis);
			}
			if (ch == '{')
			{
				++m_pos;
				return Token(TokenKind::LeftCurly);
			}
			if (ch == '}')
			{
				++m_pos;
				return Token(TokenKind::RightCurly);
			}
			if (ch == '<')
			{
				++m_pos;
				return Token(TokenKind::LeftBracket);
			}
			if (ch == '>')
			{
				++m_pos;
				return Token(TokenKind::RightBracket);
			}
			if (ch == '-')
			{
				++m_pos;
				if (m_pos < m_text.length() && m_text[m_pos] == '>')
				{
					++m_pos;
					return Token(TokenKind::Arrow);
				}
				return Token(TokenKind::Minus);
			}
			if (ch == '+')
			{
				++m_pos;
				return Token(TokenKind::Plus);
			}
			if (ch == '*')
			{
				++m_pos;
				return Token(TokenKind::Mul);
			}
			if (ch == '/')
			{
				++m_pos;
				return Token(TokenKind::Div);
			}
		}
		throw std::runtime_error("illegal character at pos " + std::to_string(m_pos) + ": " + m_text[m_pos]);
	}
	return Token(TokenKind::EndOfFile);
}

void Lexer::SetText(const std::string& text)
{
	m_text = text;
	m_pos = 0;
}

void Lexer::SkipWhitespaces()
{
	while (m_pos < m_text.length() && std::isspace(m_text[m_pos]))
	{
		++m_pos;
	}
}
