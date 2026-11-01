// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_SOLUTES_SOLUTEMETHODS_H
#define THERMOFUN_METHODS_SOLUTES_SOLUTEMETHODS_H

#include <string>
#include <memory>
#include <string>
#include <vector>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Methods/Solutes/SoluteInputs.h>
#include <ThermoFun/Model/Applicability.h>

namespace ThermoFun::v2 {

/// What the HKF method knows of its species, beside the parameters.
struct HKFSpecies
{
    std::string symbol;
    double charge = 0.0;
    bool hydrogen_ion = false;
    Applicability applicability;
    HKFConstants constants{};
};

/// HKF88, in the implementation reaktoro or gems. The parameters in SI, in the order of the catalog: a1, a2, a3, a4, c1, c2, omega.
/// No parameters: the method raises the error of a record without them.
auto hkf88ReaktoroMethod(std::vector<double> parameters, HKFSpecies species, HKFConstants constants) -> std::shared_ptr<const SubstanceMethod>;
auto hkf88GemsMethod(std::vector<double> parameters, HKFSpecies species, HKFConstants constants) -> std::shared_ptr<const SubstanceMethod>;

/// SoluteHP98: the density model of Holland and Powell (1998). The parameter: b, J/(mol K2).
auto soluteHP98Method(std::vector<double> parameters, std::string symbol) -> std::shared_ptr<const SubstanceMethod>;

/// AndersonEtAl91: the density model of Anderson et al. (1991).
auto andersonEtAl91Method() -> std::shared_ptr<const SubstanceMethod>;

/// AkinfievDiamond03: from the state of the base method and of water. The parameters in SI: xi, a, b.
auto akinfievDiamond03Method(std::vector<double> parameters, std::string symbol, double water_molar_mass) -> std::shared_ptr<const SubstanceMethod>;

} // namespace ThermoFun::v2

#endif
