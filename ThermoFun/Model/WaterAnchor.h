// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_WATER_ANCHOR_H
#define THERMOFUN_MODEL_WATER_ANCHOR_H

// The water anchors (decision 0031, "Correction of row 8"): helgeson_kirkham_1974 and codata_1989 are two data values of
// liquid water, declared on the dataset or the water record and used inside the calculation. An anchor is applied through a
// set of triple-point data of WaterTriplePoint.yaml, which for codata_1989 depends on the equation of state of water.

#include <optional>
#include <string>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Model/SubstanceRecord.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::model {

/// True for the equations of state of water whose energies follow IAPWS-95 (WaterIAPWS95, WaterIAPWS95Gas); the others
/// (WaterHGK84, WaterLVS83, ...) follow HGK.
auto isIapwsWaterMethod(const std::string& method) -> bool;

/// The name of the triple-point set of an anchor for an equation of state of water: helgeson_kirkham_1974 -> HKF_HGK_1974,
/// codata_1989 -> NEA_IAPWS (IAPWS-95 methods) or NEA_HGK (the others). Throws for steam_tables, which is a scale.
auto waterTripleSet(WaterReference anchor, const std::string& method) -> std::string;

/// The triple-point data for an anchor word (helgeson_kirkham_1974, codata_1989) or the name of a set (HKF_HGK_1974,
/// NEA_HGK, NEA_IAPWS), for an equation of state of water. This is the `triple` argument of the water methods.
auto waterTripleOf(const std::string& anchor_or_set, const std::string& method) -> const WaterTripleProperties&;

/// The anchor of the name of a triple-point set (the 0.x and legacy `water_triple_point_reference`), none if not known.
auto waterAnchorOfSet(const std::string& set) -> std::optional<WaterReference>;

/// The equation of state of water of a record, for its triple-point set: the first of its methods that is one (WaterIAPWS95,
/// WaterIAPWS95Gas, WaterHGK84, WaterLVS83), else WaterHGK84.
auto waterEquationOf(const SubstanceRecord& record) -> std::string;

/// The water solvent among the records (not a gas) (the given symbol first), or none.
auto waterSolventOf(const std::vector<SubstanceRecord>& substances, const std::string& symbol = "") -> const SubstanceRecord*;

/// The anchor of a declared value of energy_reference.water: none for steam_tables (a scale) and for no value.
inline auto anchorOf(const std::optional<WaterReference>& declared) -> std::optional<WaterReference>
{
    return declared && *declared != WaterReference::SteamTables ? declared : std::nullopt;
}

} // namespace ThermoFun::model

#endif
