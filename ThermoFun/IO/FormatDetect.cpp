// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "FormatDetect.h"

#include "detail/JsonDetect.h"

namespace ThermoFun::v2 {

namespace {

// Keys that only a record of the v2 format has
const char* const v2Keys[] = {"standard_state_properties", "methods", "stoichiometry", "reaction_properties", "record_type"};

// The legacy format has the keys TPMethods, sm_gibbs_energy, sm_enthalpy, sm_entropy_abs, sm_heat_capacity_p, sm_volume,
// logKr, drsm_gibbs_energy, reactants, _label, properties, class_ and Tst. A record without a key of the v2 format is
// legacy, so these keys are not tested.

template <class Keys>
bool hasAny(const nlohmann::json& j, const Keys& keys)
{
    for (const char* key : keys)
        if (j.contains(key))
            return true;
    return false;
}

}

namespace detail {

auto detectDocumentFormat(const nlohmann::json& j) -> DataFormat
{
    if (j.is_object() && j.contains("thermohub"))
        return DataFormat::V2;
    return DataFormat::Legacy;
}

auto detectRecordFormat(const nlohmann::json& j) -> DataFormat
{
    if (!j.is_object())
        return DataFormat::Auto;
    if (hasAny(j, v2Keys))
        return DataFormat::V2;
    // a record with a legacy key, and a record with no key of either format, is read as legacy
    return DataFormat::Legacy;
}

}

auto detectDocumentFormat(const std::string& jsonText) -> DataFormat
{
    const auto j = nlohmann::json::parse(jsonText, nullptr, false);
    if (j.is_discarded())
        return DataFormat::Auto;
    return detail::detectDocumentFormat(j);
}

auto detectRecordFormat(const std::string& jsonText) -> DataFormat
{
    const auto j = nlohmann::json::parse(jsonText, nullptr, false);
    if (j.is_discarded())
        return DataFormat::Auto;
    return detail::detectRecordFormat(j);
}

}
