// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_SOLVENTMETHOD_H
#define THERMOFUN_METHODS_WATER_SOLVENTMETHOD_H

#include <string>

#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/MethodSpec.h>

namespace ThermoFun::v2 {

/// An equation of state of water. Immutable. Independent of the database, the engine and the record.
/// A pressure of 0 means the saturation pressure: the method then writes this pressure to P.
class SolventMethod
{
public:
    virtual ~SolventMethod() = default;

    /// The specification of the method.
    virtual auto spec() const -> const MethodSpec& = 0;

    /// The properties of water as a solvent at T (K) and P (Pa). State: 0 liquid, 1 gas.
    virtual auto properties(double T, double& P, int state) const -> PropertiesSolvent = 0;

    /// The properties of water as a substance, in the reference of the triple point given by its name.
    virtual auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance = 0;

    /// True if G of the gas state contains the term R T ln(P / 1 bar).
    virtual auto gibbsEnergyContainsRTlnP() const -> bool { return false; }
};

/// A model of the dielectric constant of water, with the Born functions.
class DielectricMethod
{
public:
    virtual ~DielectricMethod() = default;

    /// The specification of the method.
    virtual auto spec() const -> const MethodSpec& = 0;

    /// The electrochemical properties at T (K) and P (Pa), from the properties of the solvent at this T and P.
    virtual auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent = 0;
};

} // namespace ThermoFun::v2

#endif
