// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_SOLVENT_H
#define THERMOFUN_ENGINE_SOLVENT_H

#include <map>
#include <memory>
#include <string>

#include <ThermoFun/Methods/Water/SolventMethod.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// The methods of a water record, ready to run. Immutable.
struct BoundSolvent
{
    std::shared_ptr<const SubstanceRecord> record;
    std::shared_ptr<const SolventMethod> equation_of_state;
    std::shared_ptr<const DielectricMethod> dielectric;  ///< null if the record has no dielectric model
};

/// True if the record has one equation of state of water with code, no other base method, and only methods of the solvent.
auto canBindSolvent(const SubstanceRecord& record) -> bool;

/// Make the methods of a water record.
auto bindSolvent(std::shared_ptr<const SubstanceRecord> record) -> std::shared_ptr<const BoundSolvent>;

/// The state of water for a record: 1 (gas) if its aggregate state is gas, else 0 (liquid).
auto solventStateOf(const SubstanceRecord& record) -> int;

/// The symbol of the solvent. The first rule that gives a symbol in the records is used: the symbol of the preferences,
/// the solvent of the dataset, the last record of the class water_solvent in the order of the map. Empty: no solvent.
auto chooseSolvent(const std::string& preferred, const std::string& dataset_solvent, const std::map<std::string, SubstanceClass>& records) -> std::string;

} // namespace ThermoFun::v2

#endif
