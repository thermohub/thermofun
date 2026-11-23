// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_METHODPARAMETERSET_H
#define THERMOFUN_MODEL_METHODPARAMETERSET_H

#include <map>
#include <string>

#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun::model {

/// A set of constants of a method, shared by many records.
struct MethodParameterSet
{
    std::string symbol;                            ///< for example "IAPWS95_constants_v1"
    std::string method;                            ///< id of the method it belongs to
    std::string implementation;                    ///< empty: all the implementations of the method
    std::map<std::string, Quantity> parameters;
    bool is_default = true;                        ///< false: only a record that names it uses it
    std::string formula_units;                     ///< the units of the formulas that the values are in, as text; values are not converted
    std::string description;                       ///< what the values are, and their units
    std::vector<std::string> data_sources;         ///< NEA reference codes (TDB-5 1.11), or free text
    std::string extra_json;

    auto operator==(const MethodParameterSet&) const -> bool = default;
};

} // namespace ThermoFun::model

#endif
