// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_GASES_GASMETHODS_H
#define THERMOFUN_METHODS_GASES_GASMETHODS_H

#include <memory>
#include <string>
#include <vector>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/Applicability.h>

namespace ThermoFun::v2 {

/// What a fluid method knows of its record, beside the parameters.
struct GasRecord
{
    double lower_T = 298.15;     ///< K, the lower T of the heat capacity of the record
    std::string formula;         ///< CORK and Sterner-Pitzer have special forms for CO2 and H2O
    Applicability applicability;
};

/// A fluid equation of state: PRSV86, ChurakovGottschalk03, SoaveRedlichKwong72, SternerPitzer94, PengRobinson78 or CorkHP91. It adds its terms to G, H
/// and S of the ideal gas, and sets V. Parameters in SI, in the order of the catalog. The options of the engine say if G keeps RT ln(phi), and if the
/// pressure term is added.
auto gasMethod(const std::string& id, std::vector<double> parameters, GasRecord record) -> std::shared_ptr<const SubstanceMethod>;

} // namespace ThermoFun::v2

#endif
