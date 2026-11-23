// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_PHYSICALCONSTANTS_H
#define THERMOFUN_COMMON_PHYSICALCONSTANTS_H

// The constants of physics (step 3b of the vocabulary plan): a set is chosen at run time with the engine option
// physical_constants, from the PhysicalConstants parameter sets (Methods/ParameterSets/PhysicalConstants*.yaml).

#include <string>
#include <vector>

namespace ThermoFun {

/// A set of constants of physics. R in J/(mol K), NA in 1/mol, F in C/mol, e in C, k in J/K.
struct PhysicalConstants
{
    std::string name;   ///< codata1986, codata2018
    double R = 0.0, NA = 0.0, F = 0.0, e = 0.0, k = 0.0;
};

/// The set of this name, read from Methods/ParameterSets at the first call. Throws std::invalid_argument for an unknown name.
auto physicalConstants(const std::string& name) -> const PhysicalConstants&;

/// The default set, codata1986: most stored G and log K values were derived with its R.
auto defaultPhysicalConstants() -> const PhysicalConstants&;

/// The names of the sets.
auto physicalConstantsNames() -> std::vector<std::string>;

/// The values of CODATA 1986, the default set (0.x names). They are also the R with which the fitted equations of the library
/// were made (NASA9, the fluid equations of state, Akinfiev-Diamond, Zhang-Duan, the ideal gas of Woolley): there R is a model
/// parameter and does not follow the option physical_constants. A calculation reads its R from the chosen set.
inline constexpr double R_CONSTANT = 8.31451, NA_CONSTANT = 6.0221367e23, F_CONSTANT = 96485.309, e_CONSTANT = 1.60217733e-19,
                        k_CONSTANT = 1.380658e-23;

/// The R of a fitted equation: CODATA 1986, a model parameter (see R_CONSTANT)
inline constexpr double R_CODATA1986 = R_CONSTANT;

} // namespace ThermoFun

#endif
