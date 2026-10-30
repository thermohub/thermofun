// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The fixes for bad legacy data. Each fix is a named function. It is used in one place and it reports an
// info diagnostic LEG-QRK-nnn (docs/redesign/05-legacy-compatibility.md section 4).
#ifndef THERMOFUN_IO_LEGACY_LEGACYQUIRKS_H
#define THERMOFUN_IO_LEGACY_LEGACYQUIRKS_H

#include <string>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>

namespace ThermoFun::v2::legacy {

/// Where a quirk was found: used in the diagnostic.
struct Where
{
    std::string record;
    std::string path;
};

/// Add an info diagnostic of a quirk. Does nothing if diagnostics is null.
auto reportQuirk(Diagnostics* diagnostics, const std::string& code, const Where& where, const std::string& message) -> void;

/// LEG-QRK-002: the name of a method without spaces at its ends
auto trimmedMethodName(const std::string& name, Diagnostics* diagnostics, const Where& where) -> std::string;

/// LEG-QRK-001: true if the name does not belong to the id. The id decides.
auto methodNameDisagrees(const std::string& trimmedName, const std::string& nameOfId, Diagnostics* diagnostics,
                         const Where& where) -> bool;

/// LEG-QRK-003: true if the units of eos_hkf_coeffs are wrong. The units of the model are used instead.
auto hkfUnitsAreWrong(const std::string& key, const std::vector<std::string>& units, Diagnostics* diagnostics,
                      const Where& where) -> bool;

/// LEG-QRK-004: true if m_expansivity has the unit label "kbar". The value is a thermal expansion in 1/K.
auto expansivityLabelIsSwapped(const std::string& unit, Diagnostics* diagnostics, const Where& where) -> bool;

/// LEG-QRK-005: true if m_compressibility has the unit label "1e-05/K". The value is a bulk modulus in kbar.
auto compressibilityLabelIsSwapped(const std::string& unit, Diagnostics* diagnostics, const Where& where) -> bool;

} // namespace ThermoFun::v2::legacy

#endif
