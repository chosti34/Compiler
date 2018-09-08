#pragma once
#include <string>

class IParser
{
public:
	virtual bool Parse(const std::string& text) = 0;
};
