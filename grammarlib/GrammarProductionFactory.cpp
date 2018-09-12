#include "stdafx.h"
#include "GrammarProductionFactory.h"
#include <functional>
#include <cctype>

namespace
{
const std::string ARROW_SYMBOL = "->";
const std::string EPSILON_SYMBOL = "#Eps#";

bool MatchSafely(const std::string& str, size_t& offset, const std::string& match)
{
	size_t offsetCopy = offset;
	for (auto ch : match)
	{
		if (offsetCopy >= str.length() || str[offsetCopy++] != ch)
		{
			return false;
		}
	}
	offset = offsetCopy;
	return true;
}

void SkipSpaces(const std::string& str, size_t& offset)
{
	while (offset < str.length() && std::isspace(str[offset]))
	{
		++offset;
	}
}

bool ReadAs(const std::string& text, size_t& offset, const std::string& what)
{
	size_t offsetCopy = offset;
	SkipSpaces(text, offsetCopy);
	if (MatchSafely(text, offsetCopy, what))
	{
		offset = offsetCopy;
		return true;
	}
	return false;
}

void ReadCharacters(
	const std::string& text,
	std::string& characters,
	size_t& offset,
	std::function<bool(char ch)> && predicate)
{
	while (offset < text.length() && predicate(text[offset]))
	{
		characters += text[offset++];
	}
}

bool ReadAsArrow(const std::string& text, size_t& offset)
{
	return ReadAs(text, offset, ARROW_SYMBOL);
}

bool ReadAsEpsilon(const std::string& text, size_t& offset)
{
	return ReadAs(text, offset, EPSILON_SYMBOL);
}

bool ReadAsTerminal(const std::string& text, size_t& offset, std::string& terminal)
{
	size_t offsetCopy = offset;
	SkipSpaces(text, offsetCopy);

	std::string characters;
	ReadCharacters(text, characters, offsetCopy, [](char ch) {
		return std::isalnum(ch) || (std::ispunct(ch) && ch != '<' && ch != '>' && ch != '#');
	});

	if (!characters.empty())
	{
		offset = offsetCopy;
		terminal = std::move(characters);
		return true;
	}
	return false;
}

bool ReadAsNonterminal(const std::string& text, size_t& offset, std::string& nonterminal)
{
	size_t offsetCopy = offset;
	SkipSpaces(text, offsetCopy);

	if (!MatchSafely(text, offsetCopy, "<"))
	{
		return false;
	}

	std::string characters;
	ReadCharacters(text, characters, offsetCopy, [](char ch) {
		return std::isalnum(ch) || (std::ispunct(ch) && ch != '<' && ch != '>' && ch != '#');
	});

	if (!characters.empty() && MatchSafely(text, offsetCopy, ">"))
	{
		offset = offsetCopy;
		nonterminal = std::move(characters);
		return true;
	}
	return false;
}
}

std::shared_ptr<GrammarProduction> GrammarProductionFactory::CreateProduction(const std::string& line)
{
	size_t offset = 0;

	std::string nonterminal;
	if (!ReadAsNonterminal(line, offset, nonterminal))
	{
		throw std::invalid_argument("can't read left nonterminal of production: " + line);
	}

	if (!ReadAsArrow(line, offset))
	{
		throw std::invalid_argument("production's right and left part must be delimited with arrow");
	}

	std::vector<GrammarSymbol> right;
	while (true)
	{
		std::string symbol;
		if (ReadAsNonterminal(line, offset, symbol))
		{
			right.emplace_back(symbol, GrammarSymbolType::Nonterminal);
		}
		else if (ReadAsTerminal(line, offset, symbol))
		{
			right.emplace_back(symbol, GrammarSymbolType::Terminal);
		}
		else if (ReadAsEpsilon(line, offset))
		{
			right.emplace_back(EPSILON_SYMBOL, GrammarSymbolType::Epsilon);
		}
		else
		{
			break;
		}
	}

	// check on string end
	if (offset < line.length())
	{
		throw std::invalid_argument("unexpected character at position " + std::to_string(offset) + ": " + line);
	}

	if (!right.empty())
	{
		return std::make_shared<GrammarProduction>(nonterminal, right);
	}
	throw std::invalid_argument("production's left part is empty: " + line);
}
