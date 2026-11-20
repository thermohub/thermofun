// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_OUTSIDERANGE_H
#define THERMOFUN_COMMON_OUTSIDERANGE_H

#include <string>

#include <ThermoFun/Common/Notes.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/ThermoScalar.hpp>

namespace ThermoFun {

/// The words that start the message of a value that a model calculated outside its range (decision 0029).
inline constexpr const char* outsideRangeMark = rangeNoteMark;

/// The message of a point outside the range of a model, as the methods write it.
inline auto outsideRangeMessage(const std::string& what) -> std::string
{
    return outsideRangeMark + what;
}

/// How far a value can be used (decision 0029): inside every range it depends on, calculated beyond one of them, or no value.
enum class Validity
{
    within,
    extrapolated,
    undefined,
};

/// Whether the error (err) of a value can be trusted (decision 0029): known, the uncertainty of the data propagated; partial,
/// a contribution could not be propagated (err is NaN); unknown, the data give no uncertainty (err is 0, which does not mean
/// exact).
enum class ErrorState
{
    known,
    partial,
    unknown,
};

/// The validity of a value, from its status and its notes.
inline auto validity(const Reaktoro_::ThermoScalar& x) -> Validity
{
    if (x.sta.first == Reaktoro_::Status::notdefined)
        return Validity::undefined;
    for (const auto& note : x.info.notes)
        if (note.kind == Note::Kind::Range)
            return Validity::extrapolated;
    return Validity::within;
}

/// The state of the error of a value, from its notes and its error. The values of the hydrogen ion are exact by convention.
inline auto errorState(const Reaktoro_::ThermoScalar& x) -> ErrorState
{
    if (std::isnan(x.err))
        return ErrorState::partial;
    for (const auto& note : x.info.notes)
        if (note.kind == Note::Kind::Uncertainty)
            return ErrorState::partial;
    if (x.err == 0.0 && x.info.source != "hydrogen_ion")
        return ErrorState::unknown;
    return ErrorState::known;
}

/// True if the value was calculated outside the range of a model or of data that it depends on.
inline auto extrapolated(const Reaktoro_::ThermoScalar& x) -> bool
{
    return validity(x) == Validity::extrapolated;
}

/// Call f on each value of a result.
template <class F>
auto forEachValue(ElectroPropertiesSolvent& r, F f) -> void
{
    for (auto* x : {&r.epsilon, &r.epsilonT, &r.epsilonP, &r.epsilonTT, &r.epsilonTP, &r.epsilonPP, &r.bornZ, &r.bornY, &r.bornQ, &r.bornN, &r.bornU, &r.bornX})
        f(*x);
}
template <class F>
auto forEachValue(PropertiesSolvent& r, F f) -> void
{
    for (auto* x : {&r.Alpha, &r.dAldT, &r.Beta, &r.Albe, &r.density, &r.densityT, &r.densityP, &r.densityTT, &r.densityTP, &r.densityPP,
                    &r.pressure, &r.pressureT, &r.pressureD, &r.pressureTT, &r.pressureTD, &r.pressureDD})
        f(*x);
}
template <class F>
auto forEachValue(ThermoPropertiesReaction& r, F f) -> void
{
    for (auto* x : {&r.reaction_gibbs_energy, &r.reaction_helmholtz_energy, &r.reaction_internal_energy, &r.reaction_enthalpy, &r.reaction_entropy,
                    &r.reaction_volume, &r.reaction_heat_capacity_cp, &r.reaction_heat_capacity_cv, &r.ln_equilibrium_constant, &r.log_equilibrium_constant})
        f(*x);
}
template <class F>
auto forEachValue(ThermoPropertiesSubstance& r, F f) -> void
{
    for (auto* x : {&r.gibbs_energy, &r.helmholtz_energy, &r.internal_energy, &r.enthalpy, &r.entropy, &r.volume, &r.heat_capacity_cp, &r.heat_capacity_cv})
        f(*x);
}

/// Add a note to a result and to the message of each of its values that is calculated (decision 0029).
template <class Result>
auto markOutsideRange(Result& result, const Note& note) -> void;

namespace detail {

inline auto markValue(Reaktoro_::ThermoScalar& x, const Note& note) -> void
{
    if (x.sta.first == Reaktoro_::Status::calculated)
    {
        x.sta.second += render(note);
        addNote(x.info.notes, note);
    }
}

} // namespace detail

template <>
inline auto markOutsideRange(ElectroPropertiesSolvent& r, const Note& note) -> void
{
    addNote(r.info.notes, note);
    for (auto* x : {&r.epsilon, &r.epsilonT, &r.epsilonP, &r.epsilonTT, &r.epsilonTP, &r.epsilonPP, &r.bornZ, &r.bornY, &r.bornQ, &r.bornN, &r.bornU, &r.bornX})
        detail::markValue(*x, note);
}

template <>
inline auto markOutsideRange(PropertiesSolvent& r, const Note& note) -> void
{
    addNote(r.info.notes, note);
    for (auto* x : {&r.Alpha, &r.dAldT, &r.Beta, &r.Albe, &r.density, &r.densityT, &r.densityP, &r.densityTT, &r.densityTP, &r.densityPP,
                    &r.pressure, &r.pressureT, &r.pressureD, &r.pressureTT, &r.pressureTD, &r.pressureDD})
        detail::markValue(*x, note);
}

template <>
inline auto markOutsideRange(ThermoPropertiesReaction& r, const Note& note) -> void
{
    addNote(r.info.notes, note);
    for (auto* x : {&r.reaction_gibbs_energy, &r.reaction_helmholtz_energy, &r.reaction_internal_energy, &r.reaction_enthalpy, &r.reaction_entropy,
                    &r.reaction_volume, &r.reaction_heat_capacity_cp, &r.reaction_heat_capacity_cv, &r.ln_equilibrium_constant, &r.log_equilibrium_constant})
        detail::markValue(*x, note);
}

template <>
inline auto markOutsideRange(ThermoPropertiesSubstance& r, const Note& note) -> void
{
    addNote(r.info.notes, note);
    for (auto* x : {&r.gibbs_energy, &r.helmholtz_energy, &r.internal_energy, &r.enthalpy, &r.entropy, &r.volume, &r.heat_capacity_cp, &r.heat_capacity_cv})
        detail::markValue(*x, note);
}

/// Set where the values of a result came from (decision 0029), on the result and on each value; empty: left as it is.
template <class Result>
auto stampSource(Result& r, const std::string& source) -> void
{
    if (source.empty())
        return;
    r.info.source = source;
    forEachValue(r, [&](Reaktoro_::ThermoScalar& x) { x.info.source = source; });
}

/// Set the pressure that the calculation used, Pa (the saturation pressure at P = 0), on the result and on each value.
template <class Result>
auto stampPressure(Result& r, double P) -> void
{
    r.info.pressure = P;
    forEachValue(r, [&](Reaktoro_::ThermoScalar& x) { x.info.pressure = P; });
}

} // namespace ThermoFun

#endif
