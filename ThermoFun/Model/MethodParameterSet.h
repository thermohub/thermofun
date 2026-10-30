// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_METHODPARAMETERSET_H
#define THERMOFUN_MODEL_METHODPARAMETERSET_H

#include <map>
#include <string>

#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun::v2 {

/// A set of constants of a method, shared by many records.
struct MethodParameterSet
{
    std::string symbol;                            ///< for example "IAPWS95_constants_v1"
    std::string method;                            ///< id of the method it belongs to
    std::map<std::string, Quantity> parameters;
    std::map<std::string, Quantity> auxiliary;
    std::string reference;                         ///< symbol of a data source
    std::string extra_json;

    auto operator==(const MethodParameterSet&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
