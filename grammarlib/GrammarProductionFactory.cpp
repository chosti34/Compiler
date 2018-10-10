#include "stdafx.h"
#include "GrammarProductionFactory.h"
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <cctype>

namespace
{
// Reserved strings for special symbols
const std::string ARROW_SYMBOL = "->";
const std::string EPSILON_SYMBOL = "#Eps#";

// Terminals and nonterminals can't contain this characters
bool IsSpecialCharacter(char ch)
{
	static std::unordered_set<char> specials = { '<', '>', '#' };
	return specials.find(ch) != specials.end();
}

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

void SkipWhile(const std::string& str, size_t& offset, std::function<bool(char ch)> && predicate)
{
	while (offset < str.length() && predicate(str[offset]))
	{
		++offset;
	}
}

void ReadWhile(
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

bool ReadAs(const std::string& text, size_t& offset, const std::string& what)
{
	size_t offsetCopy = offset;
	SkipWhile(text, offsetCopy, std::isspace);
	if (MatchSafely(text, offsetCopy, what))
	{
		offset = offsetCopy;
		return true;
	}
	return false;
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
	SkipWhile(text, offsetCopy, std::isspace);

	std::string characters;
	ReadWhile(text, characters, offsetCopy, [](char ch) {
		return std::isalnum(ch) || (std::ispunct(ch) && !IsSpecialCharacter(ch));
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
	SkipWhile(text, offsetCopy, std::isspace);

	if (!MatchSafely(text, offsetCopy, "<"))
	{
		return false;
	}

	std::string characters;
	ReadWhile(text, characters, offsetCopy, [](char ch) {
		return std::isalnum(ch) || (std::ispunct(ch) && !IsSpecialCharacter(ch));
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

	std::string leftNonterminal;
	if (!ReadAsNonterminal(line, offset, leftNonterminal))
	{
		throw std::invalid_argument("can't read left nonterminal of production: " + line);
	}

	if (!ReadAsArrow(line, offset))
	{
		throw std::invalid_argument("production's right and left parts must be delimited by arrow");
	}

	std::vector<GrammarSymbol> rightPart;
	std::string buffer;

	while (offset < line.length())
	{
		SkipWhile(line, offset, std::isspace);
		if (ReadAsEpsilon(line, offset))
		{
			rightPart.emplace_back(EPSILON_SYMBOL, GrammarSymbolType::Epsilon);
		}
		else if (ReadAsNonterminal(line, offset, buffer))
		{
			rightPart.emplace_back(buffer, GrammarSymbolType::Nonterminal);
		}
		else if (ReadAsTerminal(line, offset, buffer))
		{
			rightPart.emplace_back(buffer, GrammarSymbolType::Terminal);
		}
	}

	if (!rightPart.empty())
	{
		return std::make_shared<GrammarProduction>(leftNonterminal, rightPart);
	}
	throw std::invalid_argument("production's left part can't be empty: " + line);
}
