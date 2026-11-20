// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_WATERHELMHOLTZCONSTANTS_HPP
#define THERMOFUN_WATERHELMHOLTZCONSTANTS_HPP

#include <vector>

namespace ThermoFun {

/// The constants of the Helmholtz function of Haar, Gallagher and Kell (1984): Methods/ParameterSets/WaterHGK84.yaml
/// What the two equations of state share: Methods/ParameterSets/WaterSaturation.yaml
struct WaterSaturationConstants
{
    std::vector<double> a, b, c;  ///< of the saturation pressure, and of the densities of the liquid and the vapour
    double Tcr = 0.0, Pcr = 0.0, Dcr = 0.0, M = 0.0;
};

struct HGKConstants
{
    WaterSaturationConstants saturation;
    double Ttr = 0.0;
    std::vector<double> A0, A1, yc, A3, alpha, beta, ri, ti, A4;
    std::vector<int> ki, li, mi, ni;
    double referenceTemperature = 0.0, referenceDensity = 0.0, referenceVolume = 0.0, referenceHelmholtz = 0.0, A20 = 0.0, z0 = 0.0;
};

/// The constants of the Helmholtz function of Wagner and Pruss (2002): Methods/ParameterSets/WaterIAPWS95.yaml
struct WagnerPrussConstants
{
    WaterSaturationConstants saturation;
    double Ttr = 0.0;
    std::vector<double> no, gammao, c, d, t, n, alpha, beta, gamma, epsilon, a, b, A, B, C, F, E;
};

} // namespace ThermoFun

#endif
