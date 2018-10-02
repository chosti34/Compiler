#pragma once
#include <string>
#include <vector>
#include <optional>

// TODO: algorithm complexity should be O(1)
class TokenKeeper
{
public:
	void Add(const std::string& name, int kind);

	std::optional<int> GetKind(const std::string& name)const;
	std::optional<std::string> GetName(int kind)const;

private:
	std::vector<std::pair<std::string, int>> m_tokens;
};
