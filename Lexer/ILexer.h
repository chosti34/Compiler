#ifndef COMPILER_60MIN_ILEXER_H
#define COMPILER_60MIN_ILEXER_H

#include "Token.h"

class ILexer
{
public:
    virtual ~ILexer() = default;
    virtual Token GetNextToken() = 0;
    virtual void SetText(const std::string& text) = 0;
};

#endif //COMPILER_60MIN_ILEXER_H
