// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_REFERENCECONDITIONS_H
#define THERMOFUN_MODEL_REFERENCECONDITIONS_H

namespace ThermoFun::v2 {

/// The temperature and the pressure of the reference values of a record.
struct ReferenceConditions
{
    double T = 298.15;  ///< K
    double P = 1e5;     ///< Pa

    auto operator==(const ReferenceConditions&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
