// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef STANDARDTHERMOMODELSREAKTORO_H
#define STANDARDTHERMOMODELSREAKTORO_H
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

class Substance;

/// Standard properties from the Extended UNIQUAC model (Thomsen and Rasmussen, 1999), as in Reaktoro.
/// The heat capacity is Cp(T) = a + b T + c/(T - Theta).
/// G and H follow from the reference G and H of the record.
/// The pressure term of G is R T (alpha + beta (P - Pr)) (P - Pr). The volume and the entropy follow from it.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param subst substance with the coefficients eos_extended_uniquac_coeffs: a (J/(mol K)), b (J/(mol K^2)), c (J/mol),
/// alpha (1/bar), beta (1/bar^2), Theta (K, default 200)
auto thermoPropertiesExtendedUNIQUAC(real t, real p, Substance subst) -> ThermoPropertiesSubstanceAD;

/// Standard properties from NASA polynomials (McBride et al., NASA-9 form), as in Reaktoro.
/// The heat capacity is Cp/R = a1/T^2 + a2/T + a3 + a4 T + a5 T^2 + a6 T^3 + a7 T^4.
/// The integration constants are b1 (H) and b2 (S).
/// G, H and S use the NASA convention (G = H - T S). Outside all intervals, the function uses the nearest interval.
/// @param t temperature (K)
/// @param p pressure (bar), not used
/// @param subst substance with one record per temperature interval. Each record has limitsTP and nasa_polynomial_coeffs
/// (a1..a7, b1, b2). The volume is the reference volume.
/// @param outsideBounds set to true if t is outside all intervals
auto thermoPropertiesNasaPolynomials(real t, real p, Substance subst, bool* outsideBounds = nullptr) -> ThermoPropertiesSubstanceAD;

/// Standard properties by bilinear interpolation in temperature and pressure over a table, as in Reaktoro.
/// Outside the table, the function uses the nearest table value. The entropy is (H - G)/T.
/// @param t temperature (K)
/// @param p pressure (bar)
/// @param subst substance with the table tp_table
/// @param outsideBounds set to true if t or p is outside the table
auto thermoPropertiesTPTableInterpolation(real t, real p, Substance subst, bool* outsideBounds = nullptr) -> ThermoPropertiesSubstanceAD;

}

#endif // STANDARDTHERMOMODELSREAKTORO_H
