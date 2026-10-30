// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Solvent.h"

#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodRegistry.h>

namespace ThermoFun::v2 {

auto canBindSolvent(const SubstanceRecord& record) -> bool
{
    int equations = 0;
    for (const auto& a : record.methods)
    {
        const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Substance);
        if (!spec || !MethodRegistry::instance().hasSolventMethod(a.method))
            return false;
        equations += spec->role == MethodRole::SolventProperties;
    }
    return equations == 1;
}

auto bindSolvent(std::shared_ptr<const SubstanceRecord> record) -> std::shared_ptr<const BoundSolvent>
{
    auto bound = std::make_shared<BoundSolvent>();
    for (const auto& a : record->methods)
    {
        const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Substance);
        if (spec->role == MethodRole::SolventProperties)
            bound->equation_of_state = MethodRegistry::instance().createSolvent(a);
        else
            bound->dielectric = MethodRegistry::instance().createDielectric(a);
    }
    bound->record = std::move(record);
    return bound;
}

auto solventStateOf(const SubstanceRecord& record) -> int
{
    return record.aggregate_state == AggregateState::Gas ? 1 : 0;
}

auto chooseSolvent(const std::string& preferred, const std::string& dataset_solvent, const std::map<std::string, SubstanceClass>& records) -> std::string
{
    if (records.count(preferred))
        return preferred;
    if (records.count(dataset_solvent))
        return dataset_solvent;
    std::string last;
    for (const auto& [symbol, class_] : records)
        if (class_ == SubstanceClass::WaterSolvent)
            last = symbol;
    return last;
}

} // namespace ThermoFun::v2
