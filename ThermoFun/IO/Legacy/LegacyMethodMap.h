// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_LEGACY_LEGACYMETHODMAP_H
#define THERMOFUN_IO_LEGACY_LEGACYMETHODMAP_H

#include <set>
#include <string>
#include <vector>

// The only place that knows the legacy method numbers, the old codes and the positions in coefficient arrays.

namespace ThermoFun::v2::legacy {

/// One value of a legacy coefficient array: the array, the position, the parameter of the method, the unit in the array.
struct LegacyParameter
{
    const char* key;
    int index;
    const char* name;
    const char* unit;
};

/// One legacy method.
struct LegacyMethod
{
    int id;                       ///< the number in TPMethods
    const char* name;             ///< the name in TPMethods
    const char* method;           ///< id of the method in the catalog; "" if the legacy method has no method of its own
    const char* implementation;   ///< "" if the method has one implementation
    int thrift_code;              ///< the old code of the method; 0 if it has none
    std::vector<LegacyParameter> parameters;
};

/// All legacy methods of substances and of reactions.
auto legacySubstanceMethods() -> const std::vector<LegacyMethod>&;
auto legacyReactionMethods() -> const std::vector<LegacyMethod>&;

/// The legacy method with this number, or nullptr.
auto legacySubstanceMethod(int id) -> const LegacyMethod*;
auto legacyReactionMethod(int id) -> const LegacyMethod*;

/// All keys of coefficient arrays of the substance and of the reaction methods.
auto legacyCoefficientKeys() -> const std::set<std::string>&;

} // namespace ThermoFun::v2::legacy

#endif
