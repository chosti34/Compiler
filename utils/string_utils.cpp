#include "string_utils.h"

bool string_utils::TrimString(std::string& str)
{
    int lpos = 0;
    while (lpos < str.length() && std::isspace(str[lpos]))
    {
        ++lpos;
    }

    int rpos = int(str.length()) - 1;
    while (rpos >= 0 && std::isspace(str[rpos]))
    {
        --rpos;
    }

    if (lpos <= rpos)
    {
        str = str.substr(size_t(lpos), rpos - lpos + 1);
        return true;
    }
    return false;
}
