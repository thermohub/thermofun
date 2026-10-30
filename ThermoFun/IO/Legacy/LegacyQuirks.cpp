// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyQuirks.h"

namespace ThermoFun::v2::legacy {

auto reportQuirk(Diagnostics* diagnostics, const std::string& code, const Where& where, const std::string& message) -> void
{
    if (!diagnostics)
        return;
    Diagnostic d;
    d.severity = Severity::Info;
    d.code = code;
    d.record = where.record;
    d.path = where.path;
    d.message = message;
    diagnostics->add(std::move(d));
}

auto trimmedMethodName(const std::string& name, Diagnostics* diagnostics, const Where& where) -> std::string
{
    const auto first = name.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    const auto last = name.find_last_not_of(" \t\r\n");
    std::string trimmed = name.substr(first, last - first + 1);
    if (trimmed != name)
        reportQuirk(diagnostics, "LEG-QRK-002", where, "the name of the method has spaces at its ends: \"" + name + "\"");
    return trimmed;
}

auto methodNameDisagrees(const std::string& trimmedName, const std::string& nameOfId, Diagnostics* diagnostics,
                         const Where& where) -> bool
{
    if (trimmedName == nameOfId)
        return false;
    reportQuirk(diagnostics, "LEG-QRK-001", where,
                "the method id means \"" + nameOfId + "\", the file says \"" + trimmedName + "\"; the id is used");
    return true;
}

auto hkfUnitsAreWrong(const std::string& key, const std::vector<std::string>& units, Diagnostics* diagnostics,
                      const Where& where) -> bool
{
    if (key != "eos_hkf_coeffs" || units.size() < 3 || units[2] != "(cal*K)/mol")
        return false;
    reportQuirk(diagnostics, "LEG-QRK-003", where, "the third unit of eos_hkf_coeffs is wrong; the units of the model are used");
    return true;
}

auto expansivityLabelIsSwapped(const std::string& unit, Diagnostics* diagnostics, const Where& where) -> bool
{
    if (unit != "kbar")
        return false;
    reportQuirk(diagnostics, "LEG-QRK-004", where, "m_expansivity has the unit label kbar; the value is in 1/K");
    return true;
}

auto compressibilityLabelIsSwapped(const std::string& unit, Diagnostics* diagnostics, const Where& where) -> bool
{
    if (unit != "1e-05/K")
        return false;
    reportQuirk(diagnostics, "LEG-QRK-005", where, "m_compressibility has the unit label 1e-05/K; the value is a bulk modulus in kbar");
    return true;
}

} // namespace ThermoFun::v2::legacy
