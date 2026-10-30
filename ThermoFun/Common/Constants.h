// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_CONSTANTS_H
#define THERMOFUN_COMMON_CONSTANTS_H

#include <string>
#include <unordered_map>

// Physical constants, conversion factors and the data of water at its triple point.

namespace ThermoFun {

static const double R_CONSTANT = 8.31451,
                    NA_CONSTANT = 6.0221367e23,
                    F_CONSTANT = 96485.309,
                    e_CONSTANT = 1.60217733e-19,
                    k_CONSTANT = 1.380658e-23,
// Conversion factors
                    cal_to_J = 4.184,
                    C_to_K = 273.15,
                    K_to_C =-273.15,
                    lg_to_ln = 2.302585093,
                    ln_to_lg = 0.434294481,
                    H2O_mol_to_kg = 55.50837344,
                    Min_phys_amount = 1.66e-24,
                    cm3_mol_to_J_bar = 0.1,
                    bar_to_Pa = 1e05,

                    TEMPER_PREC     = 0.5,
                    PRESSURE_PREC   = 0.1;

/// The molar mass of water in units of g/mol
static const double H2OMolarMass = 18.015268;

struct WaterTripleProperties {
    double Str; // J/(mol*K)
    double Gtr; // J/mol
    double Htr; // J/mol
    double Utr; // J/mol
    double Atr; // J/mol
};

// Create the map with source labels
static const std::unordered_map<std::string, WaterTripleProperties> waterTripleData = {
    // Auxiliary data from Helgeson and Kirkham (1974), on page 1098
    { "HKF_HGK_1974", {
        15.1320 * cal_to_J,
       -56290.0 * cal_to_J,
       -68767.0 * cal_to_J,
       -67887.0 * cal_to_J,
       -55415.0 * cal_to_J
    }},
    { "NEA_HGK", {
        15.1386 * cal_to_J,
       -56279.9 * cal_to_J,
       -68766.44 * cal_to_J,
       -67921.9 * cal_to_J,
       -55434.49 * cal_to_J
    }},
    { "NEA_IAPWS", {
        15.1374 * cal_to_J,
       -56279.9 * cal_to_J,
       -68766.76 * cal_to_J,
       -67922.22 * cal_to_J,
       -55434.49 * cal_to_J
    }}
};

} // namespace ThermoFun

#endif
