// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The text of the problem V2-MTH-004, shared by the readers and the validation. Not installed.

#ifndef THERMOFUN_IO_MISSINGPARAMETER_H
#define THERMOFUN_IO_MISSINGPARAMETER_H

#include <string>

#include <ThermoFun/Methods/MethodSpec.h>

namespace ThermoFun::model {

/// "The method M needs the parameter p (what it is, in its unit). The method works on G, V and cannot calculate without it. The page of the method ... lists its parameters."
/// @param where for example " in a segment", put after the name of the parameter
inline auto missingParameterMessage(const MethodSpec& spec, const ParameterSpec& p, const std::string& where = "") -> std::string
{
    std::string text = "The method " + spec.id + " needs the parameter " + p.name;
    if (!p.description.empty() || (!p.source_unit.empty() && p.source_unit != "1"))
    {
        text += " (" + p.description;
        if (!p.source_unit.empty() && p.source_unit != "1")
            text += std::string(p.description.empty() ? "in " : ", in ") + p.source_unit;
        text += ")";
    }
    text += where;
    std::string properties;
    const auto add = [&](Contribution e, const char* name) {
        if (e != Contribution::None)
            properties += std::string(properties.empty() ? "" : ", ") + name;
    };
    add(spec.contributes_to.G, "G");
    add(spec.contributes_to.H, "H");
    add(spec.contributes_to.S, "S");
    add(spec.contributes_to.Cp, "Cp");
    add(spec.contributes_to.V, "V");
    if (!properties.empty())
        text += ". The method works on " + properties + " and cannot calculate without it";
    return text + ". The page of the method (reference/methods/" + spec.id + ") lists its parameters.";
}

} // namespace ThermoFun::model

#endif
