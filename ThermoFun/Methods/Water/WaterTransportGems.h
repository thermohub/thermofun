// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_WATERTRANSPORTGEMS_H
#define THERMOFUN_METHODS_WATER_WATERTRANSPORTGEMS_H

// The transport properties of the water code of GEMS (SUPCRT92 H2O92): viscosity and thermal conductivity of Sengers and
// Kamgar-Parsi (1984), surface tension at saturation (IAPWS 1976). Functions of T, P, the density and the expansion and
// compressibility, so any equation of state of water gives them. Results in SI.

#include <map>
#include <string>

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun {

/// The constants of the transport properties (names visc_, cond_, trans_, surf_ of the GEMS water parameter set)
struct WaterTransportGemsConstants
{
    double visc_a[4] = {};
    double visc_b[7][6] = {};
    double cond_aL[4] = {};
    double cond_au[4] = {};
    double cond_bL[5][6] = {};
    double cond_bu[6][5] = {};
    double visc_Pmax = 0.0, visc_P2 = 0.0, visc_T2 = 0.0, visc_P3 = 0.0, visc_T3 = 0.0, visc_Tmax = 0.0;
    double cond_Pmax = 0.0, cond_P2 = 0.0, cond_T2 = 0.0, cond_P3 = 0.0, cond_T3 = 0.0, cond_Tmax = 0.0;
    double surf_Ttripl = 0.0, surf_Tcrit = 0.0, surf_Tstar = 0.0, surf_Tcstar = 0.0, surf_v = 0.0, surf_B = 0.0, surf_stref = 0.0;
    double trans_Tstar = 0.0, trans_Dstar = 0.0, trans_Pstar = 0.0, trans_ustar = 0.0, cond_C = 0.0;
};

/// The constants from the values of a parameter set, by name
auto readWaterTransportGemsConstants(const std::map<std::string, model::Quantity>& values) -> WaterTransportGemsConstants;

/// Dynamic viscosity in Pa s at T (K), P (bar), density (kg/m3), compressibility (1/Pa). 0 outside the range of the model.
auto waterViscosityGems(const WaterTransportGemsConstants& k, real Tk, real Pbars, real Dkgm3, real betaPa) -> real;

/// Thermal conductivity in W/(m K), arguments as waterViscosityGems and the expansion (1/K). 0 outside the range.
auto waterThermalConductivityGems(const WaterTransportGemsConstants& k, real Tk, real Pbars, real Dkgm3, real alpha, real betaPa) -> real;

/// Surface tension in N/m at the saturation temperature (K). 0 outside the triple and critical points.
auto waterSurfaceTensionGems(const WaterTransportGemsConstants& k, real Tsat) -> real;

} // namespace ThermoFun

#endif
