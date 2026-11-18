// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Solvent.h"

#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodRegistry.h>

#include <vector>

namespace ThermoFun::model {

namespace {

/// Several equations of state of water in one record (decision 0028): at each T, P and state the first that covers the
/// point, in the order of the record; the saturation (P = 0) from the first that covers it at T. The last one is used
/// where none does.
class RegionalSolventMethod final : public SolventMethod
{
public:
    explicit RegionalSolventMethod(std::vector<std::shared_ptr<const SolventMethod>> methods) : methods_(std::move(methods)) {}

    // the method used everywhere else: the last one
    auto spec() const -> const MethodSpec& override { return methods_.back()->spec(); }
    auto implementation() const -> std::string override { return methods_.back()->implementation(); }
    auto gibbsEnergyContainsRTlnP() const -> bool override { return methods_.back()->gibbsEnergyContainsRTlnP(); }

    auto properties(double T, double& P, int state) const -> PropertiesSolvent override { return at(T, P, state).properties(T, P, state); }

    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override
    {
        return at(T, P, state).substanceProperties(T, P, state, triple);
    }

    auto pressureAtDensity(double T, double rho) const -> std::optional<double> override { return methods_.back()->pressureAtDensity(T, rho); }

    auto covers(double T, double P, int state) const -> bool override
    {
        for (const auto& m : methods_)
            if (m->covers(T, P, state))
                return true;
        return false;
    }

    auto coversSaturation(double T) const -> bool override
    {
        for (const auto& m : methods_)
            if (m->coversSaturation(T))
                return true;
        return false;
    }

private:
    std::vector<std::shared_ptr<const SolventMethod>> methods_;

    auto at(double T, double P, int state) const -> const SolventMethod&
    {
        for (const auto& m : methods_)
            if (P == 0.0 ? m->coversSaturation(T) : m->covers(T, P, state))
                return *m;
        return *methods_.back();
    }
};

} // namespace

auto bindSolvent(std::shared_ptr<const SubstanceRecord> record) -> std::shared_ptr<const BoundSolvent>
{
    auto bound = std::make_shared<BoundSolvent>();
    std::vector<std::shared_ptr<const SolventMethod>> equations;
    auto values = std::make_shared<SubstanceRecord>(*record);
    values->methods.clear();
    for (const auto& a : record->methods)
    {
        const auto* spec = MethodCatalog::instance().find(a.method, OperatesOn::Substance);
        if (!spec)
            continue;
        if (MethodRegistry::instance().hasSolventMethod(a.method))
        {
            if (spec->role == MethodRole::SolventProperties)
                equations.push_back(MethodRegistry::instance().createSolvent(a));
            else
                bound->dielectric = MethodRegistry::instance().createDielectric(a);
        }
        else if (a.method == "ConstantCp" || a.method == "CpPolynomial" || a.method == "ConstantProperties")
            values->methods.push_back(a);
    }
    if (equations.size() == 1)
        bound->equation_of_state = equations.front();
    else if (equations.size() > 1)
        bound->equation_of_state = std::make_shared<const RegionalSolventMethod>(std::move(equations));
    // water has its own reference (water_reference): the engine converts it, not the binding
    values->convention.reset();
    if (!bound->equation_of_state && canBind(*values))
        bound->reference_values = bindSubstance(values);
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

} // namespace ThermoFun::model
