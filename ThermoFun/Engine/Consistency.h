// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_CONSISTENCY_H
#define THERMOFUN_ENGINE_CONSISTENCY_H

#include <string>

namespace ThermoFun {

struct ConsistencyOptions
{
    double delta_G_threshold = 100.0;   ///< J/mol; a larger difference is "inconsistent"
};

/// One record at one T and P that has two ways to its properties.
struct ConsistencyItem
{
    std::string symbol;
    std::string kind;         ///< "substance_vs_reaction" or "reaction_vs_reactants"
    double T = 0.0, P = 0.0;  ///< K, Pa
    double delta_G = 0.0;     ///< J/mol, direct minus derived
    double delta_logK = 0.0;  ///< for a reaction
    std::string flag;         ///< "consistent", "inconsistent", or "not calculated" with the reason in message
    std::string message;
};

} // namespace ThermoFun

#endif
