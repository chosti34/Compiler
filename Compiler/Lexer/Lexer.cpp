#include "stdafx.h"
#include "Lexer.h"

using namespace std::string_literals;

namespace
{
const std::unordered_map<std::string, Token::Type> gcKeywords = {
	{ "func",   Token::Func },
	{ "Int",    Token::Int },
	{ "Float",  Token::Float },
	{ "Bool",   Token::Bool },
	{ "String", Token::String },
	{ "Array",  Token::Array },
	{ "if",     Token::If },
	{ "print",  Token::Print },
	{ "else",   Token::Else },
	{ "while",  Token::While },
	{ "var",    Token::Var },
	{ "return", Token::Return },
	{ "True",   Token::True },
	{ "False",  Token::False }
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
		if (std::isalpha(ch) || ch == '_')
		{
			std::string value(1, m_text[m_pos++]);
			while (m_pos < m_text.length() && (std::isalnum(m_text[m_pos]) || m_text[m_pos] == '_'))
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
				if (m_pos < m_text.length() && m_text[m_pos] == '=')
				{
					++m_pos;
					return Token{ Token::Equals };
				}
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
			if (ch == '[')
			{
				++m_pos;
				return { Token::LeftSquareBracket };
			}
			if (ch == ']')
			{
				++m_pos;
				return { Token::RightSquareBracket };
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
			if (ch == '%')
			{
				++m_pos;
				return Token{ Token::Mod };
			}
			if (ch == '!')
			{
				++m_pos;
				return Token{ Token::Negation };
			}
			if (m_text.compare(m_pos, 2, "||") == 0)
			{
				m_pos += 2;
				return { Token::Or };
			}
			if (m_text.compare(m_pos, 2, "&&") == 0)
			{
				m_pos += 2;
				return { Token::And };
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
					return { Token::StringConstant, str };
				}
			}
		}
		throw std::runtime_error("lexer can't parse token starting from character at pos " + std::to_string(m_pos) + ": '" + m_text[m_pos] + "'");
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
