// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterAnchor.h"

#include <stdexcept>

namespace ThermoFun::model {

auto isIapwsWaterMethod(const std::string& method) -> bool
{
    return method.rfind("WaterIAPWS95", 0) == 0;
}

auto waterTripleSet(WaterReference anchor, const std::string& method) -> std::string
{
    switch (anchor)
    {
    case WaterReference::HelgesonKirkham1974: return "HKF_HGK_1974";
    case WaterReference::Codata1989: return isIapwsWaterMethod(method) ? "NEA_IAPWS" : "NEA_HGK";
    case WaterReference::SteamTables: break;
    }
    throw std::invalid_argument("steam_tables is a scale of water, not an anchor: the triple-point data come from helgeson_kirkham_1974 or codata_1989.");
}

auto waterTripleOf(const std::string& anchor_or_set, const std::string& method) -> const WaterTripleProperties&
{
    const auto& data = waterTripleData();
    if (const auto it = data.find(anchor_or_set); it != data.end())
        return it->second;
    const auto anchor = fromString<WaterReference>(anchor_or_set);
    if (!anchor || *anchor == WaterReference::SteamTables)
        throw std::invalid_argument("\"" + anchor_or_set + "\" is not a water anchor (helgeson_kirkham_1974, codata_1989) or a set of WaterTriplePoint "
                                    "(HKF_HGK_1974, NEA_HGK, NEA_IAPWS).");
    return data.at(waterTripleSet(*anchor, method));
}

auto waterAnchorOfSet(const std::string& set) -> std::optional<WaterReference>
{
    if (set == "HKF_HGK_1974")
        return WaterReference::HelgesonKirkham1974;
    if (set == "NEA_HGK" || set == "NEA_IAPWS")
        return WaterReference::Codata1989;
    return std::nullopt;
}

auto waterEquationOf(const SubstanceRecord& record) -> std::string
{
    for (const auto& a : record.methods)
        for (const char* prefix : {"WaterIAPWS95", "WaterHGK84", "WaterLVS83"})
            if (a.method.rfind(prefix, 0) == 0)
                return a.method;
    return "WaterHGK84";
}

auto waterSolventOf(const std::vector<SubstanceRecord>& substances, const std::string& symbol) -> const SubstanceRecord*
{
    const SubstanceRecord* found = nullptr;
    for (const auto& r : substances)
        if (r.class_ == SubstanceClass::WaterSolvent && r.aggregate_state != AggregateState::Gas)
        {
            if (r.symbol == symbol)
                return &r;
            if (!found)
                found = &r;
        }
    return found;
}

} // namespace ThermoFun::model
