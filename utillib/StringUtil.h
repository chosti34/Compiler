#pragma once
#include <string>

namespace StringUtil
{
template <typename Container>
std::string Join(
	const Container& strings,
	const std::string& separator = ",",
	const std::string& prefix = "",
	const std::string& suffix = "")
{
	std::string result(prefix);
	for (auto it = strings.cbegin(); it != strings.cend(); ++it)
	{
		result.append(*it);
		if (std::next(it) != strings.cend())
		{
			result.append(separator);
		}
	}
	return result + suffix;
}
}
