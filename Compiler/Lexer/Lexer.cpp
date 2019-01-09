#include "stdafx.h"
#include "Lexer.h"

using namespace std::string_literals;

namespace
{
const std::unordered_map<std::string, TokenType> KEYWORDS = {
	{ "func",   TokenType::Func },
	{ "Int",    TokenType::Int },
	{ "Float",  TokenType::Float },
	{ "Bool",   TokenType::Bool },
	{ "String", TokenType::String },
	{ "Array",  TokenType::Array },
	{ "if",     TokenType::If },
	{ "print",  TokenType::Print },
	{ "else",   TokenType::Else },
	{ "while",  TokenType::While },
	{ "var",    TokenType::Var },
	{ "return", TokenType::Return },
	{ "True",   TokenType::True },
	{ "False",  TokenType::False }
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
				return Token{ TokenType::FloatConstant, value };
			}

			return Token{ TokenType::IntegerConstant, value };
		}
		if (std::isalpha(ch) || ch == '_')
		{
			std::string value(1, m_text[m_pos++]);
			while (m_pos < m_text.length() && (std::isalnum(m_text[m_pos]) || m_text[m_pos] == '_'))
			{
				value += m_text[m_pos++];
			}

			auto found = std::find_if(KEYWORDS.begin(), KEYWORDS.end(), [&value](const auto& pair) {
				return value == pair.first;
			});

			if (found != KEYWORDS.end())
			{
				return { found->second };
			}
			return Token{ TokenType::Identifier, value };
		}
		if (std::ispunct(ch))
		{
			if (ch == ';')
			{
				++m_pos;
				return Token{ TokenType::Semicolon };
			}
			if (ch == ':')
			{
				++m_pos;
				return Token{ TokenType::Colon };
			}
			if (ch == '=')
			{
				++m_pos;
				if (m_pos < m_text.length() && m_text[m_pos] == '=')
				{
					++m_pos;
					return Token{ TokenType::Equals };
				}
				return Token{ TokenType::Assign };
			}
			if (ch == ',')
			{
				++m_pos;
				return Token{ TokenType::Comma };
			}
			if (ch == '(')
			{
				++m_pos;
				return Token{ TokenType::LeftParenthesis };
			}
			if (ch == ')')
			{
				++m_pos;
				return Token{ TokenType::RightParenthesis };
			}
			if (ch == '{')
			{
				++m_pos;
				return Token{ TokenType::LeftCurlyBrace };
			}
			if (ch == '}')
			{
				++m_pos;
				return Token{ TokenType::RightCurlyBrace };
			}
			if (ch == '<')
			{
				++m_pos;
				return Token{ TokenType::LeftAngleBracket };
			}
			if (ch == '>')
			{
				++m_pos;
				return Token{ TokenType::RightAngleBracket };
			}
			if (ch == '[')
			{
				++m_pos;
				return { TokenType::LeftSquareBracket };
			}
			if (ch == ']')
			{
				++m_pos;
				return { TokenType::RightSquareBracket };
			}
			if (ch == '-')
			{
				++m_pos;
				if (m_pos < m_text.length() && m_text[m_pos] == '>')
				{
					++m_pos;
					return Token{ TokenType::Arrow };
				}
				return Token{ TokenType::Minus };
			}
			if (ch == '+')
			{
				++m_pos;
				return Token{ TokenType::Plus };
			}
			if (ch == '*')
			{
				++m_pos;
				return Token{ TokenType::Mul };
			}
			if (ch == '/')
			{
				++m_pos;
				return Token{ TokenType::Div };
			}
			if (ch == '%')
			{
				++m_pos;
				return Token{ TokenType::Mod };
			}
			if (ch == '!')
			{
				++m_pos;
				return Token{ TokenType::Negation };
			}
			if (m_text.compare(m_pos, 2, "||") == 0)
			{
				m_pos += 2;
				return Token{ TokenType::Or };
			}
			if (m_text.compare(m_pos, 2, "&&") == 0)
			{
				m_pos += 2;
				return Token{ TokenType::And };
			}
			if (ch == '"')
			{
				std::string str;
				bool escaped = false;
				size_t offsetCopy = m_pos + 1;

				while (offsetCopy < m_text.length())
				{
					if (m_text[offsetCopy] == '"' && !escaped)
					{
						++offsetCopy;
						break;
					}
					escaped = !escaped && m_text[offsetCopy] == '\\';
					str += m_text[offsetCopy++];
				}

				if (offsetCopy < m_text.length())
				{
					m_pos = offsetCopy;
					boost::replace_all(str, "\\n"s, "\n"s);
					boost::replace_all(str, "\\t"s, "\t"s);
					return Token{ TokenType::StringConstant, str };
				}
			}
		}
		throw std::runtime_error("lexer can't parse token starting from character at pos " + std::to_string(m_pos) + ": '" + m_text[m_pos] + "'");
	}
	return Token{ TokenType::EndOfFile };
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
