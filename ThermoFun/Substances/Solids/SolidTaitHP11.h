// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef SOLIDTAITHP11_H
#define SOLIDTAITHP11_H
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

class Substance;

/// Returns the thermodynamic properties of a substance (mineral) corrected for pressure with the Tait equation of state
/// (Holland and Powell, 2011): the volume, the integral of V dP from the reference pressure, and the entropy, enthalpy and
/// heat capacity that follow from the temperature dependence of the thermal pressure (Einstein model).
/// @ref Holland T.J.B., Powell R. (2011) An improved and extended internally consistent thermodynamic dataset for phases
/// of petrological interest, involving a new equation of state for solids. Journal of Metamorphic Geology, 29, 333-383.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param subst substance instance; uses the expansivity alpha0 and the bulk modulus kappa0 (kbar) of the record and
/// the coefficients eos_tait_hp11_coeffs: kappa0', kappa0'' (1/kbar), number of atoms in the formula
/// @param tps properties of the substance at the reference pressure (previously calculated with the heat capacity equation)
auto thermoPropertiesMinTaitHP11(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps, bool* variableVolume = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLIDTAITHP11_H
