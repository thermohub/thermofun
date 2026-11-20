// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_WATERLVS83_H
#define THERMOFUN_METHODS_WATER_WATERLVS83_H

// The equation of state of water near the critical point of Levelt Sengers, Kamgar-Parsi, Balfour and Sengers (1983),
// J. Phys. Chem. Ref. Data 12, 1-28, as in SUPCRT92 (H2O92) and GEMS. A revised and extended scaling model in the parametric
// variables r and theta; |theta| = 1 is the coexistence curve. Units of the code: T in K, P in MPa, density in kg/m3,
// energies in kJ/kg (J/g).
//
// It is used in a region near the critical point (SUPCRT92): Tmin1 <= T <= Tmax and Pmin(T) <= P <= Pmax(T), two
// isochores; below Tmin2 also Pmin(T) <= P <= Psat(T), the vapour side. Its saturation is used for Tmin1 <= T < Tc.

#include <array>
#include <map>
#include <memory>
#include <string>

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Model/Quantity.h>
#include "WaterGemsTypes.h"

namespace ThermoFun {

/// The coefficients of the equation (co_a, co_q, cr_ of SUPCRT92) and its region
struct WaterLVS83Constants
{
    COEFS co{};
    CRITS cr{};
    double Tmin1 = 0.0, Tmin2 = 0.0, Tmax = 0.0;  ///< K
    double Dmin = 0.0, Dmax = 0.0;               ///< kg/m3: the isochores of the region
    double Pbase1 = 0.0, Pbase2 = 0.0;           ///< MPa: Pmin(T) = Pbase1 + PTmins (T - Tmin1), Pmax(T) = Pbase2 + PTmaxs (T - Tmin2)
    double PTmins = 0.0, PTmaxs = 0.0;           ///< MPa/K
};

/// The constants from the values of the method (its parameter set and the record), by name
auto makeWaterLVS83Constants(const std::map<std::string, model::Quantity>& values) -> std::shared_ptr<const WaterLVS83Constants>;

/// One phase of water from the equation: T in K, P in MPa, D in kg/m3; A, G, S, U, H, Cv, Cp in kJ/kg (S, Cv, Cp per K) in
/// the reference of the code (before the triple-point convention); alpha 1/K, beta 1/MPa, dAldT 1/K2, gamma = rho
/// (d2P/drho2)/(dP/drho), speed of sound in m/s.
struct WaterLVS83Phase
{
    real T, P, D;
    real A, G, S, U, H, Cv, Cp;
    real alpha, beta, dAldT, gamma, speed;
};

/// The equation of Levelt Sengers et al. (1983). One object for one calculation (it keeps intermediate values).
class WaterLVS83
{
public:
    /// Keeps the constants alive as long as the object lives
    explicit WaterLVS83(std::shared_ptr<const WaterLVS83Constants> constants);
    WaterLVS83(const WaterLVS83Constants&&) = delete;

    /// The critical temperature of the equation, K
    auto criticalTemperature() const -> double { return k_->cr.Tc.val(); }

    /// True if the equation gives the saturation at T (K): Tmin1 <= T < Tc
    auto coversSaturation(double T) const -> bool;

    /// True if (T K, P MPa) is in the region of the equation
    auto inRegion(double T, double P) -> bool;

    /// The saturation pressure at T (K), MPa. Tmin1 <= T < Tc.
    auto saturationPressure(real T) -> real;

    /// The two phases at saturation at T (K): {liquid, vapour}. Tmin1 <= T < Tc.
    auto saturation(real T) -> std::array<WaterLVS83Phase, 2>;

    /// The fluid at T (K) and P (MPa) off the saturation curve: below Tc the stable phase, which is liquid above the
    /// saturation pressure and vapour below; above Tc the one fluid. The equation has no metastable states.
    auto fluid(real T, real P) -> WaterLVS83Phase;

private:
    std::shared_ptr<const WaterLVS83Constants> keep_;
    const WaterLVS83Constants* k_;
    const COEFS* co;
    const CRITS* cr;
    SATUR sa{};
    PARAM par{};
    DERI2 d2{};
    DERIV dv{};
    ABC1 a1{};
    ABC2 a2{};
    THERM th{};

    auto phase(real T, real P, real theta) -> WaterLVS83Phase;
    auto pressure(real T, real D) -> real;
    auto Pfind(int isat, real T, real DD) -> real;
    auto aux(real r1, real th1, real *d2PdT2, real *d2PdMT, real *d2PdM2, real aa, real *xk, real *sd, real Cvcoex) -> void;
    auto dFdMLVS(real r1, real th1) -> real;
    auto saturatedDensities(real T) -> void;
    auto thmLVS(int isat, real T, real r1, real th1) -> void;
    auto dalLVS(real D, real T, real P, real alpha) -> real;
    auto conver(real *rho, real Tee, real rho1s, real *rhodi, real error1) -> void;
    auto ss(real r, real th, real *s, real *sd) -> void;
    auto rtheta(real *r, real *theta, real rho, real Tee) -> void;
};

} // namespace ThermoFun

#endif
