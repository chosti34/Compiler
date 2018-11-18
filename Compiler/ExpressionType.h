#ifndef COMPILER_60MIN_EXPRESSIONTYPE_H
#define COMPILER_60MIN_EXPRESSIONTYPE_H

#include <string>

enum class ExpressionType
{
    Int,
    Float,
    Bool,
    String
};

inline bool ConvertibleToBool(ExpressionType type)
{
    switch (type)
    {
        case ExpressionType::Int:
        case ExpressionType::Bool:
        case ExpressionType::Float:
            return true;
        case ExpressionType::String:
            return false;
        default:
            throw std::logic_error("ConvertibleToBool() - undefined expression type");
    }
}

inline std::string ToString(ExpressionType type)
{
    switch (type)
    {
        case ExpressionType::Int:
            return "Int";
        case ExpressionType::Float:
            return "Float";
        case ExpressionType::Bool:
            return "Bool";
        case ExpressionType::String:
            return "String";
        default:
            throw std::logic_error("can't get string representation of undefined expression type");
    }
}

#endif //COMPILER_60MIN_EXPRESSIONTYPE_H
