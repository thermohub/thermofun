// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Quantity.h"

namespace ThermoFun {

auto Quantity::isBare() const -> bool
{
    return unit_as_written.empty() && !uncertainty.has_value() && meta.quality.empty() && meta.origin.empty() &&
           meta.calculation_path.empty() && meta.source.empty();
}

}
