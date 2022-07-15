#include "GlobalVariables.h"
#include "Batch/ThermoBatch.h"

namespace ThermoFun {

auto availableSubstanceTPMethods() -> const std::string
{
    std::string r;
    for (auto a : enum_method_substance)
    {
        r += "{\"method\": {\"" + std::to_string(a.first) + "\": \"" + a.second + "\"}";
        auto type = (SubstanceTPMethodType) a.first;
        if (substance_method_parameters.at(type).size() == 1  &&  substance_method_parameters.at(type)[0] !="")
        {
            r +=  ", \"" + substance_method_parameters.at(type)[0] +"\" : { \"values\": []}}\n";
        } else
            r += "}\n";
    }
    return r;
}

auto scanProperties(const std::string& what) -> const std::string
{
    std::string r = "property (unit)\n---------------\n";
    for (auto a: defaultPropertyNames)
    {
        if (a.second == what)
        {
            r += a.first + " (";
            r += defaultPropertyUnits.at(a.first)+ ")\n";
        }
    }
    return r;
}

auto availablePropertiesSubstance() -> const std::string
{
    return scanProperties("substance");
}

auto availablePropertiesReaction() -> const std::string
{
    return scanProperties("reaction");
}

auto availableReactionTPMethods() -> const std::string
{
    std::string r;
    for (auto a : enum_method_reaction)
    {
        r += "{\"method\": {\"" + std::to_string(a.first) + "\": \"" + a.second + "\"}";
        auto type = (ReactionTPMethodType) a.first;
        if (reaction_method_parameters.at(type).size() == 1  &&  reaction_method_parameters.at(type)[0] !="")
        {
            r +=  ", \"" + reaction_method_parameters.at(type)[0] +"\" : { \"values\": []}}\n";
        } else
            r += "}\n";
    }
    return r;
}
} // namespace ThermoFun
