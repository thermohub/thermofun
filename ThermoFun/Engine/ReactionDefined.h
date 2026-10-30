// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_REACTIONDEFINED_H
#define THERMOFUN_ENGINE_REACTIONDEFINED_H

#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun::v2 {

/// The properties of a substance from the properties of its defining reaction and of the other reactants, each with its
/// coefficient. coefficient: of the substance in the reaction.
auto substanceFromReaction(double T, double P, const ThermoPropertiesReaction& reaction, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& others, double coefficient) -> ThermoPropertiesSubstance;

/// The molar volume of an ideal gas, J/bar, with its derivatives.
auto setIdealGasVolume(ThermoPropertiesSubstance& tps, double T, double P) -> void;

/// While it lives, one substance is being calculated from its defining reaction. It raises an error that names the
/// cycle if the same substance is already being calculated on this thread, instead of a stack overflow.
class DefiningReactionGuard
{
public:
    explicit DefiningReactionGuard(const std::string& symbol);
    ~DefiningReactionGuard();
    DefiningReactionGuard(const DefiningReactionGuard&) = delete;
    auto operator=(const DefiningReactionGuard&) -> DefiningReactionGuard& = delete;
};

} // namespace ThermoFun::v2

#endif
