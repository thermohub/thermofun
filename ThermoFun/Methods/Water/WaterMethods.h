// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_WATERMETHODS_H
#define THERMOFUN_METHODS_WATER_WATERMETHODS_H

#include <memory>
#include <string>

#include <ThermoFun/Methods/Water/SolventMethod.h>
#include <ThermoFun/Model/MethodApplication.h>
#include <ThermoFun/Methods/Water/WaterZhangDuan2005.h>

namespace ThermoFun {
struct WaterHGKgemsModelParameters;
}

namespace ThermoFun::model {

/// WaterHGK84: implementation gems (the HGK code of SUPCRT92 and GEMS; near the critical point they used WaterLVS83), or reaktoro.
auto waterHGK84GemsMethod(std::shared_ptr<const WaterHGKgemsModelParameters> constants) -> std::shared_ptr<const SolventMethod>;
auto waterHGK84ReaktoroMethod(const MethodApplication& application) -> std::shared_ptr<const SolventMethod>;

/// The properties of water as a solvent from the code of GEMS, tied to the formation values by `triple` (a water anchor
/// or the name of a triple-point set, as SolventMethod::substanceProperties).
auto waterHGK84GemsProperties(double T, double& P, int state, const std::string& triple) -> PropertiesSolvent;

/// WaterLVS83: the equation of Levelt Sengers et al. (1983) near the critical point, in its region (SUPCRT92). values: its
/// parameters by name; transport: the constants of viscosity, conductivity and surface tension (GEMS water parameter set).
auto waterLVS83Method(const std::map<std::string, Quantity>& values, const std::map<std::string, Quantity>& transport) -> std::shared_ptr<const SolventMethod>;

/// WaterIAPWS95Gas: the gas equation of Wagner and Pruss (2002), used in its region (metastable vapour below 10 MPa)
auto waterIAPWS95GasMethod(const MethodApplication& application) -> std::shared_ptr<const SolventMethod>;

/// WaterIAPWS95, implementation reaktoro
auto waterIAPWS95ReaktoroMethod(const MethodApplication& application) -> std::shared_ptr<const SolventMethod>;

/// WaterZhangDuan05: the volume, the density and its derivatives; with the option of DEW, also G (P >= 1 kbar).
auto waterZhangDuan05Method(bool dew_gibbs, ZhangDuanModelParameters constants) -> std::shared_ptr<const SolventMethod>;

/// JohnsonNorton91: implementation reaktoro, from the density of the solvent; or gems, from its own HGK calculation, without the solvent.
auto johnsonNorton91ReaktoroMethod(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>;
auto johnsonNorton91GemsMethod(std::shared_ptr<const WaterHGKgemsModelParameters> constants) -> std::shared_ptr<const DielectricMethod>;

/// BradleyPitzer79. constants: in the order of the catalog; it does not read the water
auto bradleyPitzer79Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>;

/// HelgesonKirkham74. constants: in the order of the catalog
auto helgesonKirkham74Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>;

/// SverjenskyEtAl14. constants: in the order of the catalog, as for the two others of this form
auto sverjenskyEtAl14Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>;

/// FernandezEtAl97
auto fernandezEtAl97Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>;

} // namespace ThermoFun::model

#endif
