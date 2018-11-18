#ifndef COMPILER_60MIN_IPARSER_H
#define COMPILER_60MIN_IPARSER_H

#include <string>

template <typename Result>
class IParser
{
public:
    virtual ~IParser() = default;
    virtual Result Parse(const std::string& text) = 0;
};

#endif //COMPILER_60MIN_IPARSER_H
