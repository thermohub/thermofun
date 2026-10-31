// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_BINDING_H
#define THERMOFUN_ENGINE_BINDING_H

#include <functional>
#include <memory>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Conventions/ConventionConverter.h>
#include <ThermoFun/Methods/Evaluate.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// The methods of one record, ready to run. Immutable. Nothing of it is made at a call of T and P.
struct BoundSubstance
{
    std::shared_ptr<const SubstanceRecord> record;
    std::vector<std::shared_ptr<const SubstanceMethod>> order;  ///< in the order of execution
    std::vector<ConventionShift> to_canonical;                  ///< for each method of order: the change of its result to the canonical convention
    ReferenceValues reference;                                  ///< the SI values of the record
    PropertiesInfo reference_info;                              ///< their status and errors
    MethodNeeds needs;                                          ///< what the methods need from outside the record
};

/// What a binding needs to find the conventions of the methods of a record.
struct BindConventions
{
    Convention dataset;                          ///< the convention of the dataset
    std::function<double()> element_entropy;     ///< the sum of the entropies of the elements of the record. Called only if needed.
};

/// True if every method of the record has code, and exactly one of them is a base method.
auto canBind(const SubstanceRecord& record) -> bool;

/// The conflicts between the methods of a record, each with what to do. Not used for legacy records yet: they keep
/// the behaviour of the old engine until the v2 reader (R3) reports these problems at load.
auto checkMethods(const SubstanceRecord& record) -> Diagnostics;

/// Make the methods of a record and put them in the order of execution: the base method, a transformation, the
/// corrections that set a property, the corrections that only add, each group by rank.
/// The result of the base method is in the convention of the method, else of the record, else of the dataset. Another method that sets G is in
/// its own convention, else the canonical one: it does not use the reference values. An increment has no energy offset, so it is not changed.
auto bindSubstance(std::shared_ptr<const SubstanceRecord> record, const BindConventions& conventions = {}) -> std::shared_ptr<const BoundSubstance>;

} // namespace ThermoFun::v2

#endif
