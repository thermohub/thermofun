// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_BINDING_H
#define THERMOFUN_ENGINE_BINDING_H

#include <map>
#include <ThermoFun/Model/MethodDefinition.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Model/BuiltinParameterSets.h>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Conventions/ConventionConverter.h>
#include <ThermoFun/Methods/Evaluate.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/MethodParameterSet.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::model {

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
    const std::map<std::string, MethodDefinition>* definitions = nullptr;  ///< the Method records of the database
    std::function<double()> element_entropy;     ///< the sum of the entropies of the elements of the record. Called only if needed.
};

/// A copy of a record whose methods hold the values of their parameter sets. A value that a method gives itself stays. The same record if no method names a set.
template <typename Record>
auto withParameterSets(std::shared_ptr<const Record> record, const std::map<std::string, MethodParameterSet>& sets) -> std::shared_ptr<const Record>
{
    if (std::none_of(record->methods.begin(), record->methods.end(), [](const auto& a) { return !a.parameter_set.empty(); }))
        return record;
    auto copy = std::make_shared<Record>(*record);
    for (auto& a : copy->methods)
    {
        if (a.parameter_set.empty())
            continue;
        const auto set = sets.find(a.parameter_set);
        const auto* found = set != sets.end() ? &set->second : parameterSetVariant(a.parameter_set);
        if (!found)
            funError("Cannot use the method " + a.method + " of " + record->symbol, "There is no parameter set " + a.parameter_set + ".", __LINE__, __FILE__);
        a.parameters.insert(found->parameters.begin(), found->parameters.end());
        if (a.implementation.empty())
            a.implementation = found->implementation;
    }
    return copy;
}

/// True if every method of the record has code, and exactly one of them is a base method.
auto canBind(const SubstanceRecord& record, const std::map<std::string, MethodDefinition>* definitions = nullptr) -> bool;

/// The conflicts between the methods of a record, each with what to do. A load uses it for v2 files, and validate for all records:
/// a legacy record is read as it is.
auto checkMethods(const SubstanceRecord& record) -> Diagnostics;

/// Make the methods of a record and put them in the order of execution: the base method, a transformation, the
/// corrections that set a property, the corrections that only add, each group by rank.
/// The result of the base method is in the convention of the method, else of the record, else of the dataset. Another method that sets G is in
/// its own convention, else the canonical one: it does not use the reference values. An increment has no energy offset, so it is not changed.
auto bindSubstance(std::shared_ptr<const SubstanceRecord> record, const BindConventions& conventions = {}) -> std::shared_ptr<const BoundSubstance>;

} // namespace ThermoFun::model

#endif
