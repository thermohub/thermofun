// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The keys of the legacy ThermoHub JSON format (docs/redesign/05-legacy-compatibility.md section 2).
#ifndef THERMOFUN_IO_LEGACY_LEGACYKEYS_H
#define THERMOFUN_IO_LEGACY_LEGACYKEYS_H

namespace ThermoFun::v2::legacy {

// file
inline constexpr const char* ThermoDataset = "thermodataset";
inline constexpr const char* Datasources = "datasources";
inline constexpr const char* Date = "date";
inline constexpr const char* Conventions = "conventions";

// records
inline constexpr const char* Properties = "properties";  // wrapper of an export of ArangoDB
inline constexpr const char* Label = "_label";
inline constexpr const char* Symbol = "symbol";
inline constexpr const char* Name = "name";
inline constexpr const char* Formula = "formula";
inline constexpr const char* FormulaCharge = "formula_charge";
inline constexpr const char* AggregateState = "aggregate_state";
inline constexpr const char* Class = "class_";
inline constexpr const char* Tst = "Tst";
inline constexpr const char* Pst = "Pst";
inline constexpr const char* LimitsTP = "limitsTP";
inline constexpr const char* MassPerMole = "mass_per_mole";
inline constexpr const char* Reaction = "reaction";
inline constexpr const char* References = "references";
inline constexpr const char* TPMethods = "TPMethods";
inline constexpr const char* Method = "method";
inline constexpr const char* MExpansivity = "m_expansivity";
inline constexpr const char* MCompressibility = "m_compressibility";

// properties of a substance
inline constexpr const char* SmGibbsEnergy = "sm_gibbs_energy";
inline constexpr const char* SmEnthalpy = "sm_enthalpy";
inline constexpr const char* SmEntropyAbs = "sm_entropy_abs";
inline constexpr const char* SmHeatCapacityP = "sm_heat_capacity_p";
inline constexpr const char* SmVolume = "sm_volume";

// reactions
inline constexpr const char* Equation = "equation";
inline constexpr const char* Reactants = "reactants";
inline constexpr const char* LogKr = "logKr";
inline constexpr const char* DrsmGibbsEnergy = "drsm_gibbs_energy";
inline constexpr const char* DrsmEnthalpy = "drsm_enthalpy";
inline constexpr const char* DrsmEntropy = "drsm_entropy";
inline constexpr const char* DrsmHeatCapacityP = "drsm_heat_capacity_p";
inline constexpr const char* DrsmVolume = "drsm_volume";

// elements
inline constexpr const char* Entropy = "entropy";
inline constexpr const char* HeatCapacity = "heat_capacity";
inline constexpr const char* AtomicMass = "atomic_mass";
inline constexpr const char* Volume = "volume";
inline constexpr const char* IsotopeMass = "isotope_mass";
inline constexpr const char* Number = "number";

// entries of TPMethods
inline constexpr const char* MPhaseTransProps = "m_phase_trans_props";
inline constexpr const char* TpTable = "tp_table";

} // namespace ThermoFun::v2::legacy

#endif
