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
	{ "scan",   TokenType::Scan },
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
	while (m_ch)
	{
		if (std::isspace(*m_ch))
		{
			SkipWhitespaces();
			continue;
		}
		if (std::isdigit(*m_ch))
		{
			return OnDigit();
		}
		if (std::isalpha(*m_ch) || m_ch == '_')
		{
			return OnAlphaOrUnderscore();
		}
		if (std::ispunct(*m_ch))
		{
			if (m_text.compare(m_pos, 2, "//") == 0)
			{
				SkipUntil('\n');
				Advance();
				continue;
			}
			return OnPunct();
		}

		const auto fmt = boost::format("can't parse char '%1%' on line %2%, column %3%")
			% (std::isprint(*m_ch) ? std::to_string(*m_ch) : "#" + std::to_string(int(*m_ch)))
			% m_line
			% m_column;
		throw std::runtime_error(fmt.str());
	}
	return { TokenType::EndOfFile, boost::none, m_pos, m_line, m_column };
}

void Lexer::SetText(const std::string& text)
{
	m_text = text;
	m_pos = 0;
	m_column = 0;
	m_line = 0;
	UpdateCh();
}

void Lexer::SkipWhitespaces()
{
	while (m_ch && std::isspace(*m_ch))
	{
		Advance();
	}
}

void Lexer::SkipUntil(char ch)
{
	while (m_ch && m_ch != ch)
	{
		Advance();
	}
}

Token Lexer::OnDigit()
{
	assert(m_ch && std::isdigit(*m_ch));

	std::string value(1, *m_ch);
	Advance();

	while (m_ch && std::isdigit(*m_ch))
	{
		value += *m_ch;
		Advance();
	}

	if (m_ch != '.')
	{
		return { TokenType::IntegerConstant, value, m_pos, m_line, m_column };
	}

	value += *m_ch;
	Advance();

	while (m_ch && std::isdigit(*m_ch))
	{
		value += *m_ch;
		Advance();
	}
	return { TokenType::FloatConstant, value, m_pos, m_line, m_column };
}

Token Lexer::OnAlphaOrUnderscore()
{
	assert(m_ch && (std::isalpha(*m_ch) || m_ch == '_'));

	std::string value(1, *m_ch);
	Advance();

	while (m_ch && (std::isalnum(*m_ch) || m_ch == '_'))
	{
		value += *m_ch;
		Advance();
	}

	auto found = std::find_if(KEYWORDS.begin(), KEYWORDS.end(), [&value](const auto& pair) {
		const std::string& keyword = pair.first;
		return value == keyword;
	});

	if (found != KEYWORDS.end())
	{
		const TokenType& keyword = found->second;
		return { keyword, boost::none, m_pos, m_line, m_column };
	}
	return { TokenType::Identifier, value, m_pos, m_line, m_column };
}

Token Lexer::OnPunct()
{
	assert(m_ch && std::ispunct(*m_ch));

	if (m_text.compare(m_pos, 2, "==") == 0)
	{
		Advance();
		Advance();
		return { TokenType::Equals, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, "<=") == 0)
	{
		Advance();
		Advance();
		return { TokenType::LessOrEquals, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, ">=") == 0)
	{
		Advance();
		Advance();
		return { TokenType::MoreOrEquals, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, "!=") == 0)
	{
		Advance();
		Advance();
		return { TokenType::NotEquals, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, "->") == 0)
	{
		Advance();
		Advance();
		return { TokenType::Arrow, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, "||") == 0)
	{
		Advance();
		Advance();
		return { TokenType::Or, boost::none, m_pos, m_line, m_column };
	}
	if (m_text.compare(m_pos, 2, "&&") == 0)
	{
		Advance();
		Advance();
		return { TokenType::And, boost::none, m_pos, m_line, m_column };
	}

	if (m_ch == ';')
	{
		Advance();
		return { TokenType::Semicolon, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == ':')
	{
		Advance();
		return { TokenType::Colon, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '=')
	{
		Advance();
		return { TokenType::Assign, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == ',')
	{
		Advance();
		return { TokenType::Comma, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '(')
	{
		Advance();
		return { TokenType::LeftParenthesis, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == ')')
	{
		Advance();
		return { TokenType::RightParenthesis, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '{')
	{
		Advance();
		return { TokenType::LeftCurlyBrace, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '}')
	{
		Advance();
		return { TokenType::RightCurlyBrace, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '<')
	{
		Advance();
		return { TokenType::LeftAngleBracket, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '>')
	{
		Advance();
		return { TokenType::RightAngleBracket, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '[')
	{
		Advance();
		return { TokenType::LeftSquareBracket, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == ']')
	{
		Advance();
		return { TokenType::RightSquareBracket, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '-')
	{
		Advance();
		return { TokenType::Minus, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '+')
	{
		Advance();
		return { TokenType::Plus, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '*')
	{
		Advance();
		return { TokenType::Mul, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '/')
	{
		Advance();
		return { TokenType::Div, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '%')
	{
		Advance();
		return { TokenType::Mod, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '!')
	{
		Advance();
		return { TokenType::Negation, boost::none, m_pos, m_line, m_column };
	}
	if (m_ch == '"')
	{
		const size_t line = m_line;
		const size_t column = m_column;

		Advance();
		std::string value;
		bool escaped = false;

		while (m_ch && m_ch != '\n')
		{
			if (m_ch == '"' && !escaped)
			{
				Advance();
				boost::replace_all(value, "\\n"s, "\n"s);
				boost::replace_all(value, "\\t"s, "\t"s);
				return { TokenType::StringConstant, value, m_pos, m_line, m_column };
			}
			escaped = !escaped && m_ch == '\\';
			value += *m_ch;
			Advance();
		}

		auto fmt = boost::format("string doesn't have closing quotes on line %1%, column %2%")
			% std::to_string(line)
			% std::to_string(column);
		throw std::runtime_error(fmt.str());
	}

	auto fmt = boost::format("can't parse punct '%1%' on line %2%, column %3%")
		% *m_ch
		% m_line
		% m_column;
	throw std::runtime_error(fmt.str());
}

void Lexer::Advance()
{
	if (!m_ch)
	{
		throw std::logic_error("lexer can't advance because end of input has been reached");
	}

	if (m_ch == '\n')
	{
		++m_line;
		m_column = 0;
	}

	++m_pos;
	++m_column;

	UpdateCh();
}

void Lexer::UpdateCh()
{
	m_ch = (m_pos < m_text.length()) ? boost::make_optional(m_text[m_pos]) : boost::none;
}
