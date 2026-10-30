// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_APPLICABILITY_H
#define THERMOFUN_MODEL_APPLICABILITY_H

#include <map>
#include <optional>
#include <string>

#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun::v2 {

/// The T-P range in which a method application is valid, in SI units. A missing bound is no limit.
struct Applicability
{
    std::optional<double> t_min, t_max;  ///< K
    std::optional<double> p_min, p_max;  ///< Pa
    bool is_hard = false;                ///< true: a point outside the range is an error

    auto operator==(const Applicability&) const -> bool = default;
};

/// One piece of a method that is piecewise in T (several Cp intervals).
struct Segment
{
    double t_min = 0.0;                           ///< K
    double t_max = 0.0;                           ///< K
    std::map<std::string, Quantity> parameters;   ///< named parameters of this piece (SI)

    auto operator==(const Segment&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
