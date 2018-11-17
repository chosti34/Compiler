#pragma once
#include <string>
#include <boost/optional.hpp>

enum class GrammarSymbolType
{
	Terminal,
	Nonterminal,
	Epsilon
};

class GrammarSymbol
{
public:
	GrammarSymbol(std::string text, GrammarSymbolType type, boost::optional<std::string> attribute = boost::none);

	const std::string& GetText()const;
	GrammarSymbolType GetType()const;

	void SetAttribute(std::string attribute);
	boost::optional<std::string> GetAttribute()const;

private:
	std::string m_text;
	GrammarSymbolType m_type;
	boost::optional<std::string> m_attribute;
};

std::string ToString(GrammarSymbolType type);
