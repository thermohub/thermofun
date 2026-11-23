// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_APPLICABILITY_H
#define THERMOFUN_MODEL_APPLICABILITY_H

#include <map>
#include <optional>
#include <string>

#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun::model {

/// The T-P range in which a method application is valid, in SI units. A missing bound is no limit.
struct Applicability
{
    std::optional<double> t_min, t_max;  ///< K
    std::optional<double> p_min, p_max;  ///< Pa
    bool is_hard = false;                ///< true: a point outside the range is an error

    auto operator==(const Applicability&) const -> bool = default;
};

/// The T-P region in which a method application is used (decision 0028), in SI units; a missing bound is no limit. Methods
/// of the same kind in a record are tried in their order, and the first whose region holds T and P is used. Unlike
/// Applicability (where a method is valid, a warning outside), a region selects.
struct Region
{
    std::optional<double> t_min, t_max;  ///< K
    std::optional<double> p_min, p_max;  ///< Pa

    auto empty() const -> bool { return !t_min && !t_max && !p_min && !p_max; }
    auto holdsT(double T) const -> bool { return (!t_min || T >= *t_min) && (!t_max || T <= *t_max); }
    auto holds(double T, double P) const -> bool { return holdsT(T) && (!p_min || P >= *p_min) && (!p_max || P <= *p_max); }
    auto operator==(const Region&) const -> bool = default;
};

/// One piece of a method that is piecewise in T (several Cp intervals).
struct Segment
{
    double t_min = 0.0;                           ///< K
    double t_max = 0.0;                           ///< K
    std::map<std::string, Quantity> parameters;   ///< named parameters of this piece (SI)

    auto operator==(const Segment&) const -> bool = default;
};

} // namespace ThermoFun::model

#endif
