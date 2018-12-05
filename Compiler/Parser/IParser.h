#pragma once
#include <string>

template <typename Result>
class IParser
{
public:
	virtual ~IParser() = default;
	virtual Result Parse(const std::string& text) = 0;
};
