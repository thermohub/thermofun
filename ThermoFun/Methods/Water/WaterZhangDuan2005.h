// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef WATERZHANGDUAN2005
#define WATERZHANGDUAN2005

#include <vector>

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

/// The constants of the model, as in Methods/ParameterSets/WaterZhangDuan05.yaml. The order is that of the catalog; a[0] is not used.
struct ZhangDuanConstants
{
    double a[15];
    double gamma, Tc, Vc, R;
    double g1kb[5];  ///< G(T, 1 kbar) of the model DEW: a polynomial in T (degC) in cal/mol, the highest power first
    double Tmax, Pmax, Pmin;  ///< the range of the model, K and Pa

    explicit ZhangDuanConstants(const std::vector<double>& v)
    : a{0.0, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13]}, gamma(v[14]), Tc(v[15]), Vc(v[16]), R(v[17]), g1kb{v[18], v[19], v[20], v[21], v[22]}, Tmax(v[23]), Pmax(v[24]), Pmin(v[25]) {}
};

//// Forward declarations

/// Return the thermodynamic properties of water
/// @param T temparature (K)
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
auto thermoPropertiesWaterZhangDuan2005(real T, real P, const ZhangDuanConstants& c) -> ThermoPropertiesSubstanceAD;

/// Return the physical properties of water
/// @param wt instance of the strcuture holding the calculated themrmodynamic properties of water
/// The properties of water with the exact derivatives of the density up to the third order (the pressure of the pass in Pa)
auto propertiesWaterZhangDuan2005(const Reaktoro_::Pass& pass, const ZhangDuanConstants& c) -> PropertiesSolventAD;


/// The Gibbs energy of water (J/mol) as in the model DEW, for P >= 1 kbar: G(T, 1 kbar) + the integral of V dP
auto waterGibbsEnergyDEW(real T, real P, const ZhangDuanConstants& c) -> real;

}

#endif // WATERZHANGDUAN2005

