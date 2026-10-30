// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_WATERMETHODS_H
#define THERMOFUN_METHODS_WATER_WATERMETHODS_H

#include <ThermoFun/Methods/Water/SolventMethod.h>
#include <ThermoFun/Methods/Water/WaterZhangDuan2005.h>

namespace ThermoFun::v2 {

/// WaterHGK84, implementation gems (the code of SUPCRT and GEMS, with the LVS equation near the critical point)
class WaterHGK84Gems final : public SolventMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override;
    auto properties(double T, double& P, int state, const std::string& triple) const -> PropertiesSolvent;
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override;
    auto gibbsEnergyContainsRTlnP() const -> bool override { return true; }
};

/// WaterHGK84, implementation reaktoro
class WaterHGK84Reaktoro final : public SolventMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override;
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override;
    auto gibbsEnergyContainsRTlnP() const -> bool override { return true; }
};

/// WaterIAPWS95, implementation reaktoro
class WaterIAPWS95Reaktoro final : public SolventMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override;
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override;
};

/// WaterZhangDuan05: the volume, the density and its derivatives; with the option of DEW, also G (P >= 1 kbar)
class WaterZhangDuan05 final : public SolventMethod
{
public:
    explicit WaterZhangDuan05(bool dew_gibbs = false, DewGibbsCoefficients coefficients = {})
    : dew_gibbs_(dew_gibbs), coefficients_(coefficients) {}

    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override;
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override;

private:
    bool dew_gibbs_;
    DewGibbsCoefficients coefficients_;
};

/// JohnsonNorton91, implementation reaktoro: from the density of the solvent
class JohnsonNorton91Reaktoro final : public DielectricMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override;
};

/// JohnsonNorton91, implementation gems: from its own HGK calculation; it does not use the solvent
class JohnsonNorton91Gems final : public DielectricMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override;
};

/// SverjenskyEtAl14
class SverjenskyEtAl14 final : public DielectricMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override;
};

/// FernandezEtAl97
class FernandezEtAl97 final : public DielectricMethod
{
public:
    auto spec() const -> const MethodSpec& override;
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override;
};

} // namespace ThermoFun::v2

#endif
