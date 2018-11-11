#pragma once
#include <string>
#include "ASTNode.h"

class IParser
{
public:
	virtual ~IParser() = default;
	virtual std::unique_ptr<ASTNode> Parse(const std::string& text) = 0;
};
