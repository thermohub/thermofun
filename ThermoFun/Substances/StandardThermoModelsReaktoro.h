// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef STANDARDTHERMOMODELSREAKTORO_H
#define STANDARDTHERMOMODELSREAKTORO_H
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

class Substance;

/// Standard properties from the Extended UNIQUAC model (Thomsen and Rasmussen, 1999; as in Reaktoro):
/// Cp(T) = a + b T + c/(T - Theta); G and H follow from the reference G and H of the record; the pressure
/// correction of G is R T (alpha + beta (P - Pr)) (P - Pr), from which the volume and the entropy follow.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param subst substance with eos_extended_uniquac_coeffs: a (J/(mol K)), b (J/(mol K^2)), c (J/mol), alpha (1/bar),
/// beta (1/bar^2), Theta (K, default 200)
auto thermoPropertiesExtendedUNIQUAC(real t, real p, Substance subst) -> ThermoPropertiesSubstanceAD;

/// Standard properties from NASA polynomials (McBride et al., NASA-9 form, as in Reaktoro):
/// Cp/R = a1/T^2 + a2/T + a3 + a4 T + a5 T^2 + a6 T^3 + a7 T^4, with the integration constants b1 (H) and b2 (S).
/// The values are those of the NASA convention (G = H - T S). Outside of all intervals the nearest one is used.
/// @param t temperature (K)
/// @param p pressure (bar), not used
/// @param subst substance with one record per temperature interval: limitsTP and nasa_polynomial_coeffs
/// (a1..a7, b1, b2); the volume is the reference volume
/// @param outsideBounds set true if t is outside all intervals
auto thermoPropertiesNasaPolynomials(real t, real p, Substance subst, bool* outsideBounds = nullptr) -> ThermoPropertiesSubstanceAD;

/// Standard properties by bilinear interpolation in temperature and pressure over a table (as in Reaktoro).
/// Outside of the table the nearest value of the table is used. The entropy is (H - G)/T.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param subst substance with the table tp_table
/// @param outsideBounds set true if t or p is outside the table
auto thermoPropertiesTPTableInterpolation(real t, real p, Substance subst, bool* outsideBounds = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // STANDARDTHERMOMODELSREAKTORO_H
