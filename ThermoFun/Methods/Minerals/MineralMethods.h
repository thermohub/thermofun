// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_MINERALMETHODS_H
#define THERMOFUN_METHODS_MINERALS_MINERALMETHODS_H

#include <memory>
#include <string>
#include <vector>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/MethodApplication.h>

namespace ThermoFun::model {

/// A method of a mineral with a formula of its own: LandauHP98, LandauHP11, MurnaghanHP98, TaitHP11, VolumeBerman88, VolumeDorogokupets88,
/// BirchMurnaghan47 or CpExtendedUNIQUAC. coefficients: in SI, in the order of the catalog; none if the record gives none, and the formula then
/// raises its error. alpha0 and kappa0: in SI, if the method has them.
auto mineralMethod(const std::string& id, std::vector<double> coefficients, double alpha0, double kappa0, Applicability applicability, std::vector<std::string> names = {}, bool gems = false) -> std::shared_ptr<const SubstanceMethod>;

/// NASA9: the coefficients by T segment.
auto nasa9Method(const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>;

/// TPTable: a table in T and P.
auto tpTableMethod(const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>;

} // namespace ThermoFun::model

#endif
