#ifndef COMPILER_60MIN_GRAMMARPRODUCTIONFACTORY_H
#define COMPILER_60MIN_GRAMMARPRODUCTIONFACTORY_H

#include "GrammarProduction.h"

class GrammarProductionFactory
{
public:
    std::shared_ptr<GrammarProduction> CreateProduction(const std::string& line);
};

#endif //COMPILER_60MIN_GRAMMARPRODUCTIONFACTORY_H
