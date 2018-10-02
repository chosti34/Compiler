#include "stdafx.h"
#include "TokenKeeper.h"

void TokenKeeper::Add(const std::string& name, int kind)
{
	if (!GetName(kind) && !GetKind(name))
	{
		m_tokens.emplace_back(name, kind);
	}
	else
	{
		throw std::logic_error("can't duplicate keys and values in token keeper");
	}
}

std::optional<int> TokenKeeper::GetKind(const std::string& name)const
{
	for (const auto& [key, value] : m_tokens)
	{
		if (name == key)
		{
			return value;
		}
	}
	return std::nullopt;
}

std::optional<std::string> TokenKeeper::GetName(int kind)const
{
	for (const auto& [key, value] : m_tokens)
	{
		if (kind == value)
		{
			return key;
		}
	}
	return std::nullopt;
}
