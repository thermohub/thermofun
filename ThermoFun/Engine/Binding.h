// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_BINDING_H
#define THERMOFUN_ENGINE_BINDING_H

#include <memory>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Methods/Evaluate.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// The methods of one record, ready to run. Immutable. Nothing of it is made at a call of T and P.
struct BoundSubstance
{
    std::shared_ptr<const SubstanceRecord> record;
    std::vector<std::shared_ptr<const SubstanceMethod>> order;  ///< in the order of execution
    ReferenceValues reference;                                  ///< the SI values of the record
    PropertiesInfo reference_info;                              ///< their status and errors
    MethodNeeds needs;                                          ///< what the methods need from outside the record
};

/// True if every method of the record has code, and exactly one of them is a base method.
auto canBind(const SubstanceRecord& record) -> bool;

/// The conflicts between the methods of a record, each with what to do. Not used for legacy records yet: they keep
/// the behaviour of the old engine until the v2 reader (R3) reports these problems at load.
auto checkMethods(const SubstanceRecord& record) -> Diagnostics;

/// Make the methods of a record and put them in the order of execution: the base method, a transformation, the
/// corrections that set a property, the corrections that only add, each group by rank.
auto bindSubstance(std::shared_ptr<const SubstanceRecord> record) -> std::shared_ptr<const BoundSubstance>;

} // namespace ThermoFun::v2

#endif
