// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERZHANGDUAN2005
#define WATERZHANGDUAN2005

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

//// Forward declarations

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterZhangDuan2005(real T, real P) -> ThermoPropertiesSubstanceAD;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
/// The properties of water with the exact derivatives of the density up to the third order (the pressure of the pass in Pa)
auto propertiesWaterZhangDuan2005(const Reaktoro_::Pass& pass) -> PropertiesSolventAD;

auto waterDensityZhangDuan2005(real T, real P) -> real;

/// The coefficients of G(T, 1 kbar) of the model DEW: a polynomial in T (degC) in cal/mol, the highest power first
struct DewGibbsCoefficients
{
    double g1kb[5] = {2.6880734e-09, 6.3163061e-07, -0.019372355, -16.945093, -55769.287};
};

/// The Gibbs energy of water (J/mol) as in the model DEW, for P >= 1 kbar: G(T, 1 kbar) + the integral of V dP
auto waterGibbsEnergyDEW(real T, real P, const DewGibbsCoefficients& c) -> real;

}

#endif // WATERZHANGDUAN2005

