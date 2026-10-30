// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Quantity.h"

namespace ThermoFun::v2 {

auto Quantity::isBare() const -> bool
{
    return unit_as_written.empty() && !uncertainty.has_value() && !meta.quality && !meta.origin &&
           meta.calculation_path.empty() && meta.source.empty();
}

} // namespace ThermoFun::v2
