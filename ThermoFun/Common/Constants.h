// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_CONSTANTS_H
#define THERMOFUN_COMMON_CONSTANTS_H

#include <string>
#include <unordered_map>

#include <ThermoFun/Common/PhysicalConstants.h>

// Conversion factors and the data of water at its triple point. The constants of physics are in the generated PhysicalConstants.h.

namespace ThermoFun {

// Conversion factors
static const double cal_to_J = 4.184,
                    C_to_K = 273.15,
                    K_to_C =-273.15,
                    lg_to_ln = 2.302585092994046,
                    ln_to_lg = 0.4342944819032518,
                    H2O_mol_to_kg = 55.50837344,
                    Min_phys_amount = 1.66e-24,
                    cm3_mol_to_J_bar = 0.1,
                    bar_to_Pa = 1e05,

                    TEMPER_PREC     = 0.5,
                    PRESSURE_PREC   = 0.1;

/// The molar mass of water in units of g/mol (IAPWS-95, the water methods)
static const double H2OMolarMass = 18.015268;

/// The molar mass of water in units of g/mol from the standard atomic weights, as GEMS and SUPCRT92 use it to convert
/// molality to mole fraction (ln(1000/Mw) = 4.0165)
static const double H2OMolarMassAtomicWeights = 18.01528;

struct WaterTripleProperties {
    double Str; // J/(mol*K)
    double Gtr; // J/mol
    double Htr; // J/mol
    double Utr; // J/mol
    double Atr; // J/mol
};

/// The data of water at its triple point, by source. From Methods/ParameterSets/WaterTriplePoint.yaml.
auto waterTripleData() -> const std::unordered_map<std::string, WaterTripleProperties>&;

} // namespace ThermoFun

#endif
