#include "GlobalVariables.h"

namespace ThermoFun {

auto availableSubstanceTPMethods() -> const std::string
{
    std::string r;
    for (auto a : enum_method_substance)
    {
        r += "{\"method\": {\"" + to_string(a.first) + "\": \"" + a.second + "\"}";
        auto type = (SubstanceTPMethodType) a.first;
        if (substance_method_parameters.at(type).size() == 1  &&  substance_method_parameters.at(type)[0] !="")
        {
            r +=  ", \"" + substance_method_parameters.at(type)[0] +"\" : { \"values\": []}}\n";
        } else
            r += "}\n";
    }
    return r;
}

auto availableReactionTPMethods() -> const std::string
{
    std::string r;
    for (auto a : enum_method_reaction)
    {
        r += "{\"method\": {\"" + to_string(a.first) + "\": \"" + a.second + "\"}";
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
