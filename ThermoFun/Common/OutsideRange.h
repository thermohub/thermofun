// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_OUTSIDERANGE_H
#define THERMOFUN_COMMON_OUTSIDERANGE_H

#include <string>

#include <ThermoFun/Common/Notes.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/ThermoScalar.hpp>

namespace ThermoFun {

/// The words that start the message of a value that a model calculated outside its range (of T, P, the density of water,
/// its T intervals or table). The status of such a value stays `calculated`: the value is there, and the message says why it
/// can be wrong. All methods write these messages with one function, so outsideRange() finds them all.
inline constexpr const char* outsideRangeMark = rangeNoteMark;

/// The message of a point outside the range of a model, as the methods write it.
inline auto outsideRangeMessage(const std::string& what) -> std::string
{
    return outsideRangeMark + what;
}

/// True if the value was calculated outside the range of its model. Its status message (sta.second) says which range.
inline auto outsideRange(const Reaktoro_::ThermoScalar& x) -> bool
{
    return x.sta.second.find(outsideRangeMark) != std::string::npos;
}

/// Add a note to a result and to the message of each of its values that is calculated (decision 0029).
template <class Result>
auto markOutsideRange(Result& result, const Note& note) -> void;

namespace detail {

inline auto markValue(Reaktoro_::ThermoScalar& x, const std::string& text) -> void
{
    if (x.sta.first == Reaktoro_::Status::calculated)
        x.sta.second += text;
}

} // namespace detail

template <>
inline auto markOutsideRange(ElectroPropertiesSolvent& r, const Note& note) -> void
{
    addNote(r.notes, note);
    const auto text = render(note);
    for (auto* x : {&r.epsilon, &r.epsilonT, &r.epsilonP, &r.epsilonTT, &r.epsilonTP, &r.epsilonPP, &r.bornZ, &r.bornY, &r.bornQ, &r.bornN, &r.bornU, &r.bornX})
        detail::markValue(*x, text);
}

template <>
inline auto markOutsideRange(PropertiesSolvent& r, const Note& note) -> void
{
    addNote(r.notes, note);
    const auto text = render(note);
    for (auto* x : {&r.Alpha, &r.dAldT, &r.Beta, &r.Albe, &r.density, &r.densityT, &r.densityP, &r.densityTT, &r.densityTP, &r.densityPP,
                    &r.pressure, &r.pressureT, &r.pressureD, &r.pressureTT, &r.pressureTD, &r.pressureDD})
        detail::markValue(*x, text);
}

template <>
inline auto markOutsideRange(ThermoPropertiesSubstance& r, const Note& note) -> void
{
    addNote(r.notes, note);
    const auto text = render(note);
    for (auto* x : {&r.gibbs_energy, &r.helmholtz_energy, &r.internal_energy, &r.enthalpy, &r.entropy, &r.volume, &r.heat_capacity_cp, &r.heat_capacity_cv})
        detail::markValue(*x, text);
}

} // namespace ThermoFun

#endif
