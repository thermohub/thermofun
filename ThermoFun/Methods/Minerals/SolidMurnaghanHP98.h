// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLIDMURNAGHANHP98_H
#define SOLIDMURNAGHANHP98_H
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Minerals/MineralInputs.h>


namespace ThermoFun {


/// Returns the  correcected themrodynamic properties of a substance (mineral) uisng the Murnaghan EoS (Holland and Powell, 1998)
/// @ref Holland T.J.B., Powell R. (1998) An internally consistent thermodynamic data set for phases of
/// petrological interest. Journal of Metamorphic Geology, 16, 309-343.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
/// The terms of the Murnaghan volume of Holland and Powell (1998), V = V0 (1 + a0 (T - Tr) - 20 a0 (sqrt T - sqrt Tr)) (1 + 4 P/k)^-1/4
/// with k = k0 (1 - 1.5e-4 (T - Tr)), integrated from P = 0: G = int V dP (J/mol), S = -dG/dT, Cp = -T d2G/dT2 (J/(mol K)), and V (J/bar).
/// P in bar, k0 in kbar. LandauHP98 uses it too, with V0 = Vmax Q298^2.
struct MurnaghanTermsHP98
{
    real g, s, v, cp;
};

auto murnaghanTermsHP98(const real& V0, const real& k0, const real& a0, const real& TK, double Tr, const real& Pbar) -> MurnaghanTermsHP98;

auto thermoPropertiesMinMurnaghanEOSHP98(real t, real p, const MineralInputs& subst, ThermoPropertiesSubstanceAD tps, bool* variableVolume = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLIDMURNAGHANHP98_H
