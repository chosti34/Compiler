#include "stdafx.h"
#include "Lexer.h"

namespace
{
const std::unordered_map<std::string, Token::Type> gcKeywords = {
	{ "func", Token::Func },
	{ "Int", Token::Int },
	{ "Float", Token::Float },
	{ "Bool", Token::Bool },
	{ "Array", Token::Array },
	{ "if", Token::If },
	{ "else", Token::Else },
	{ "while", Token::While },
	{ "var", Token::Var },
	{ "return", Token::Return },
	{ "True", Token::True },
	{ "False", Token::False },
};
}

Lexer::Lexer(const std::string& text)
{
	SetText(text);
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
				return Token{ Token::FloatConstant, value };
			}

			return Token{ Token::IntegerConstant, value };
		}
		if (std::isalpha(ch))
		{
			std::string value(1, m_text[m_pos++]);
			while (std::isalnum(m_text[m_pos]))
			{
				value += m_text[m_pos++];
			}

			auto found = std::find_if(gcKeywords.begin(), gcKeywords.end(), [&value](const auto& pair) {
				return value == pair.first;
			});

			if (found != gcKeywords.end())
			{
				return { found->second };
			}
			return Token{ Token::Identifier, value };
		}
		if (std::ispunct(ch))
		{
			if (ch == ';')
			{
				++m_pos;
				return Token{ Token::Semicolon };
			}
			if (ch == ':')
			{
				++m_pos;
				return Token{ Token::Colon };
			}
			if (ch == '=')
			{
				++m_pos;
				return Token{ Token::Assign };
			}
			if (ch == ',')
			{
				++m_pos;
				return Token{ Token::Comma };
			}
			if (ch == '(')
			{
				++m_pos;
				return Token{ Token::LeftParenthesis };
			}
			if (ch == ')')
			{
				++m_pos;
				return Token{ Token::RightParenthesis };
			}
			if (ch == '{')
			{
				++m_pos;
				return Token{ Token::LeftCurly };
			}
			if (ch == '}')
			{
				++m_pos;
				return Token{ Token::RightCurly };
			}
			if (ch == '<')
			{
				++m_pos;
				return Token{ Token::LeftBracket };
			}
			if (ch == '>')
			{
				++m_pos;
				return Token{ Token::RightBracket };
			}
			if (ch == '-')
			{
				++m_pos;
				if (m_pos < m_text.length() && m_text[m_pos] == '>')
				{
					++m_pos;
					return Token{ Token::Arrow };
				}
				return Token{ Token::Minus };
			}
			if (ch == '+')
			{
				++m_pos;
				return Token{ Token::Plus };
			}
			if (ch == '*')
			{
				++m_pos;
				return Token{ Token::Mul };
			}
			if (ch == '/')
			{
				++m_pos;
				return Token{ Token::Div };
			}
		}
		throw std::runtime_error("lexer can't parse character at pos " + std::to_string(m_pos) + ": '" + m_text[m_pos] + "'");
	}
	return Token{ Token::EndOfFile };
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
