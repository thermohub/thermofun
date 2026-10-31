// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_VALIDATE_H
#define THERMOFUN_IO_VALIDATE_H

#include <functional>
#include <map>
#include <string>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/IO/V2/V2Writer.h>

namespace ThermoFun::v2 {

/// The elements of a formula and their counts. It raises an error if the formula cannot be read.
using FormulaParser = std::function<std::map<std::string, double>(const std::string&)>;

/// All problems between the records, in the order of the records: what a load does not refuse, because a record can come
/// from another file. How the methods of one substance combine is checked by checkMethods of Engine/Binding.
auto validateRecords(const V2Records& records, const FormulaParser& parseFormula) -> Diagnostics;

} // namespace ThermoFun::v2

#endif
