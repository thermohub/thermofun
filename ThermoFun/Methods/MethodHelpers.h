// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODHELPERS_H
#define THERMOFUN_METHODS_METHODHELPERS_H

#include <string>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/Applicability.h>

// What the methods with formulas on the property structures have in common.

namespace ThermoFun::v2 {

inline constexpr double jPerBarToM3 = 1e-5;

/// These methods have formulas on autodiff numbers only, so they have no pass with plain numbers.
[[noreturn]] inline auto needsAPass(const std::string& id) -> void
{
    funError("Cannot run the method " + id, "It has no calculation with plain numbers, only one with derivatives.", __LINE__, __FILE__);
}

inline auto infoOf(const Reaktoro_::ThermoScalar& x) -> ValueInfo
{
    ValueInfo out;
    out.status = x.sta.first == Reaktoro_::Status::notdefined ? ValueInfo::Status::NotDefined : ValueInfo::Status::Calculated;
    out.err = x.err;
    return out;
}

/// The properties of the model, with V in J/bar, as the state of the executor.
template <typename Scalar>
auto setFromModel(SubstanceState<Scalar>& state, const ThermoPropertiesSubstanceAD& tps) -> void
{
    state.G = tps.gibbs_energy;
    state.H = tps.enthalpy;
    state.S = tps.entropy;
    state.Cp = tps.heat_capacity_cp;
    state.V = tps.volume * jPerBarToM3;
}

/// The state of the executor as the properties of the model, with V in J/bar.
template <typename Scalar>
auto toModel(const SubstanceState<Scalar>& state) -> ThermoPropertiesSubstanceAD
{
    ThermoPropertiesSubstanceAD tps;
    tps.gibbs_energy = state.G;
    tps.enthalpy = state.H;
    tps.entropy = state.S;
    tps.heat_capacity_cp = state.Cp;
    tps.volume = state.V / jPerBarToM3;
    return tps;
}

/// The message of a point outside a range, on the properties that are calculated. The status does not change.
inline auto setMessage(PropertiesInfo& out, const std::string& message) -> void
{
    for (auto* x : {&out.G, &out.H, &out.S, &out.Cp, &out.V})
        if (x->status == ValueInfo::Status::Calculated)
            x->message += message + "; ";
}

/// The message of a point outside the range of the record; empty if it is inside.
inline auto boundsMessage(const std::string& model, const Applicability& a, double T, double P) -> std::string
{
    const double tMin = a.t_min.value_or(-1e300), tMax = a.t_max.value_or(1e300), pMin = a.p_min.value_or(-1e300), pMax = a.p_max.value_or(1e300);
    if (pMax >= P && tMax >= T && pMin <= P && tMin <= T)
        return "";
    return model + ": out of T(" + std::to_string(tMin) + "-" + std::to_string(tMax) + " K) and P(" + std::to_string(pMin) + "-" + std::to_string(pMax) + " Pa) bounds";
}

/// The properties of a method that calculates all of them: calculated, with no error.
inline auto allCalculated(PropertiesInfo& out) -> void
{
    for (auto* x : {&out.G, &out.H, &out.S, &out.Cp, &out.V})
    {
        x->status = ValueInfo::Status::Calculated;
        x->err = 0.0;
    }
}

/// The information of the reference volume, with its error in J/bar, as the old engine adds it to G and H.
inline auto volumeInJPerBar(const ValueInfo& V) -> ValueInfo
{
    ValueInfo out = V;
    out.err /= jPerBarToM3;
    return out;
}

} // namespace ThermoFun::v2

#endif
