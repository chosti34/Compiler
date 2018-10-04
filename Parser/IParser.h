#pragma once
#include <string>

class IParser
{
public:
	virtual ~IParser() = default;
	virtual bool Parse(const std::string& text) = 0;
};
