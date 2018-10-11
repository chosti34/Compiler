#pragma once
#include <string>
#include <optional>

enum class GrammarSymbolType
{
	Terminal,
	Nonterminal,
	Epsilon
};

class GrammarSymbol
{
public:
	GrammarSymbol(std::string text, GrammarSymbolType type, std::optional<std::string> attribute = std::nullopt);

	const std::string& GetText()const;
	GrammarSymbolType GetType()const;

	void SetAttribute(std::string attribute);
	std::optional<std::string> GetAttribute()const;

private:
	std::string m_text;
	GrammarSymbolType m_type;
	std::optional<std::string> m_attribute;
};

std::string ToString(GrammarSymbolType type);
