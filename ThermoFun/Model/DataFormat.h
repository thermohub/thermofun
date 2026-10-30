// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_DATAFORMAT_H
#define THERMOFUN_MODEL_DATAFORMAT_H

namespace ThermoFun::v2 {

/// The format of a data file or of a record. Auto: the reader finds it.
enum class DataFormat
{
    Auto,
    Legacy,
    V2
};

} // namespace ThermoFun::v2

#endif
