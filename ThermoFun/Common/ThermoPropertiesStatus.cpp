// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "ThermoPropertiesStatus.h"

namespace ThermoFun {

auto setMessage(Reaktoro_::Status sta, std::string propertyOfsymbol, std::string calcMessage, std::string &message_ ) -> void
{
    if (sta == Reaktoro_::Status::notdefined)
        message_ += propertyOfsymbol + " not defined; ";
    else
        message_ = calcMessage;
}

auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesSubstance &tps ) -> void
{
    if (tps.enthalpy.sta.first == sta)
        tps.enthalpy.sta.second += message + "; ";
    if (tps.entropy.sta.first == sta)
        tps.entropy.sta.second += message + "; ";
    if (tps.gibbs_energy.sta.first == sta)
        tps.gibbs_energy.sta.second += message + "; ";
    if (tps.heat_capacity_cp.sta.first == sta)
        tps.heat_capacity_cp.sta.second += message + "; ";
    if (tps.heat_capacity_cv.sta.first == sta)
        tps.heat_capacity_cv.sta.second += message + "; ";
    if (tps.helmholtz_energy.sta.first == sta)
        tps.helmholtz_energy.sta.second += message + "; ";
    if (tps.internal_energy.sta.first == sta)
        tps.internal_energy.sta.second += message + "; ";
    if (tps.volume.sta.first == sta)
        tps.volume.sta.second += message + "; ";
}

auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesReaction &tpr ) -> void
{
    if (tpr.reaction_enthalpy.sta.first == sta)
        tpr.reaction_enthalpy.sta.second += message + "; ";
    if (tpr.reaction_entropy.sta.first == sta)
        tpr.reaction_entropy.sta.second += message + "; ";
    if (tpr.reaction_gibbs_energy.sta.first == sta)
        tpr.reaction_gibbs_energy.sta.second += message + "; ";
    if (tpr.reaction_heat_capacity_cp.sta.first == sta)
        tpr.reaction_heat_capacity_cp.sta.second += message + "; ";
    if (tpr.reaction_heat_capacity_cv.sta.first == sta)
        tpr.reaction_heat_capacity_cv.sta.second += message + "; ";
    if (tpr.reaction_helmholtz_energy.sta.first == sta)
        tpr.reaction_helmholtz_energy.sta.second += message + "; ";
    if (tpr.reaction_internal_energy.sta.first == sta)
        tpr.reaction_internal_energy.sta.second += message + "; ";
    if (tpr.reaction_volume.sta.first == sta)
        tpr.reaction_volume.sta.second += message + "; ";
}

} // namespace ThermoFun
