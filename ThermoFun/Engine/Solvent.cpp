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

/// A water equation of state used in a region given in the record (decision 0028), besides its own. The saturation is
/// covered if the region holds T (its P bounds are not checked at P = 0).
class BoxedSolventMethod final : public SolventMethod
{
public:
    BoxedSolventMethod(std::shared_ptr<const SolventMethod> inner, Region region) : inner_(std::move(inner)), region_(std::move(region)) {}

    auto spec() const -> const MethodSpec& override { return inner_->spec(); }
    auto implementation() const -> std::string override { return inner_->implementation(); }
    auto gibbsEnergyContainsRTlnP() const -> bool override { return inner_->gibbsEnergyContainsRTlnP(); }
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override { return inner_->properties(T, P, state); }
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override
    {
        return inner_->substanceProperties(T, P, state, triple);
    }
    auto pressureAtDensity(double T, double rho) const -> std::optional<double> override { return inner_->pressureAtDensity(T, rho); }
    auto covers(double T, double P, int state) const -> bool override { return region_.holds(T, P) && inner_->covers(T, P, state); }
    auto coversSaturation(double T) const -> bool override { return region_.holdsT(T) && inner_->coversSaturation(T); }

private:
    std::shared_ptr<const SolventMethod> inner_;
    Region region_;
};

/// A dielectric model used in a region given in the record
class BoxedDielectricMethod final : public DielectricMethod
{
public:
    BoxedDielectricMethod(std::shared_ptr<const DielectricMethod> inner, Region region) : inner_(std::move(inner)), region_(std::move(region)) {}

    auto spec() const -> const MethodSpec& override { return inner_->spec(); }
    auto implementation() const -> std::string override { return inner_->implementation(); }
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override
    {
        return inner_->properties(T, P, solvent, state);
    }
    auto covers(double T, double P) const -> bool override { return region_.holds(T, P) && inner_->covers(T, P); }

private:
    std::shared_ptr<const DielectricMethod> inner_;
    Region region_;
};

/// Several dielectric models in one record (decision 0028): at each T and P the first that covers it, in the order of the
/// record; the last one where none does. Tanger and Helgeson (1988): Bradley-Pitzer at low T above 1 kbar, else
/// Helgeson-Kirkham.
class RegionalDielectricMethod final : public DielectricMethod
{
public:
    explicit RegionalDielectricMethod(std::vector<std::shared_ptr<const DielectricMethod>> methods) : methods_(std::move(methods)) {}

    auto spec() const -> const MethodSpec& override { return methods_.back()->spec(); }
    auto implementation() const -> std::string override { return methods_.back()->implementation(); }
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override
    {
        for (const auto& m : methods_)
            if (m->covers(T, P))
                return m->properties(T, P, solvent, state);
        return methods_.back()->properties(T, P, solvent, state);
    }
    auto covers(double T, double P) const -> bool override
    {
        for (const auto& m : methods_)
            if (m->covers(T, P))
                return true;
        return false;
    }

private:
    std::vector<std::shared_ptr<const DielectricMethod>> methods_;
};

} // namespace

auto bindSolvent(std::shared_ptr<const SubstanceRecord> record) -> std::shared_ptr<const BoundSolvent>
{
    auto bound = std::make_shared<BoundSolvent>();
    std::vector<std::shared_ptr<const SolventMethod>> equations;
    std::vector<std::shared_ptr<const DielectricMethod>> dielectrics;
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
            {
                auto m = MethodRegistry::instance().createSolvent(a);
                equations.push_back(a.region.empty() ? m : std::make_shared<const BoxedSolventMethod>(m, a.region));
            }
            else
            {
                auto m = MethodRegistry::instance().createDielectric(a);
                dielectrics.push_back(a.region.empty() ? m : std::make_shared<const BoxedDielectricMethod>(m, a.region));
            }
        }
        else if (a.method == "ConstantCp" || a.method == "CpPolynomial" || a.method == "ConstantProperties")
            values->methods.push_back(a);
    }
    // one method of a kind is used everywhere, even with a region (V2-MTH-027 says that nothing follows it)
    if (dielectrics.size() == 1)
        bound->dielectric = dielectrics.front();
    else if (dielectrics.size() > 1)
        bound->dielectric = std::make_shared<const RegionalDielectricMethod>(std::move(dielectrics));
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
