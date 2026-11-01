// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The handles on the parameters of the records: for a fit, a change clears only what depends on the record.

#include <algorithm>

#include "ThermoEngineImpl.h"
#include <ThermoFun/Methods/MethodRegistry.h>

namespace ThermoFun
{

auto ThermoEngine::Impl::isOfReaction(const ParameterRef& ref) const -> bool
{
    if (ref.method.empty())
        return ref.parameter == "logK" || ref.parameter == "dG" || ref.parameter == "dH" || ref.parameter == "dS" || ref.parameter == "dCp" || ref.parameter == "dV";
    const auto* spec = v2::MethodCatalog::instance().find(ref.method);
    return spec && spec->operates_on == v2::OperatesOn::Reaction;
}

auto ThermoEngine::Impl::isConstant(const ParameterRef& ref) const -> bool
{
    if (ref.method.empty())
        return false;
    const auto* spec = v2::MethodCatalog::instance().find(ref.method);
    return spec && std::any_of(spec->parameters.begin(), spec->parameters.end(), [&](const auto& p) { return p.constant && p.name == ref.parameter; });
}

auto ThermoEngine::Impl::valueOf(const ParameterRef& ref) const -> v2::Quantity
{
    const v2::Quantity* found = nullptr;
    v2::SubstanceRecord s;
    v2::ReactionRecord r;
    if (isOfReaction(ref))
    {
        r = *v2::withParameterSets(database.getReaction(ref.symbol).record(), database.parameterSets());
        found = v2::findValue(r, ref);
    }
    else
    {
        s = *v2::withParameterSets(database.getSubstance(ref.symbol).record(), database.parameterSets());
        found = v2::findValue(s, ref);
    }
    // a constant of the method that the record does not replace
    if (!found && isConstant(ref))
        for (const auto& a : isOfReaction(ref) ? r.methods : s.methods)
            if (a.method == ref.method)
            {
                const auto all = v2::withBuiltinConstants(a);
                if (const auto it = all.parameters.find(ref.parameter); it != all.parameters.end())
                    return it->second;
            }
    if (!found)
        funError("Cannot read the parameter " + ref.name(), "The record has no such value.", __LINE__, __FILE__);
    return *found;
}

auto ThermoEngine::Impl::dependentsOf(const std::string& symbol) const -> std::set<std::string>
{
    if (!users_are_current)
    {
        users.clear();
        for (const auto& [name, substance] : database.mapSubstances())
            if (const auto reaction = substance.reactionSymbol(); !reaction.empty())
                users[reaction].insert(name);
        for (const auto& [name, reaction] : database.mapReactions())
            for (const auto& reactant : reaction.reactants())
                users[reactant.first].insert(name);
        users_are_current = true;
    }
    std::set<std::string> out;
    std::vector<std::string> open = {symbol};
    while (!open.empty())
    {
        const auto current = open.back();
        open.pop_back();
        if (!out.insert(current).second)
            continue;
        if (const auto it = users.find(current); it != users.end())
            open.insert(open.end(), it->second.begin(), it->second.end());
    }
    return out;
}

template <class Edit>
auto ThermoEngine::Impl::editValue(const ParameterRef& ref, Edit edit) -> void
{
    dropStaleResults();
    auto& bound = *bound_cache;
    auto forget = [](auto& map, auto& records, const auto* old) {
        map.erase(old);
        records.erase(std::remove_if(records.begin(), records.end(), [&](const auto& r) { return r.get() == old; }), records.end());
    };
    if (isOfReaction(ref))
    {
        const auto old = database.getReaction(ref.symbol).record();
        auto copy = std::make_shared<v2::ReactionRecord>(*old);
        edit(*copy);
        copy->original_text.clear();
        database.setReaction(Reaction(std::shared_ptr<const v2::ReactionRecord>(copy)));
        std::lock_guard<std::mutex> lock(bound.mutex);
        forget(bound.reactions, bound.reaction_records, old.get());
    }
    else
    {
        const auto old = database.getSubstance(ref.symbol).record();
        auto copy = std::make_shared<v2::SubstanceRecord>(*old);
        edit(*copy);
        copy->original_text.clear();
        database.setSubstance(Substance(std::shared_ptr<const v2::SubstanceRecord>(copy)));
        std::lock_guard<std::mutex> lock(bound.mutex);
        forget(bound.substances, bound.records, old.get());
        bound.solvents.erase(old.get());
        bound.corrections.erase(old.get());
    }
    // a solute can read the solvent as a substance: then every result goes
    if (ref.symbol == preferences.solvent_symbol)
    {
        clearResults();
        return;
    }
    const auto dependents = dependentsOf(ref.symbol);
    substance_results.erase(dependents);
    reaction_results.erase(dependents);
    cached_for.database_revision = static_cast<const Database&>(database).revision();
}

auto ParameterHandle::value() const -> double
{
    return engine_->pimpl->valueOf(ref_).value;
}

auto ParameterHandle::setValue(double value_si) -> void
{
    const auto ref = ref_;
    auto& impl = *engine_->pimpl;
    const auto current = impl.valueOf(ref);
    impl.editValue(ref, [&](auto& record) {
        auto* value = v2::findValue(record, ref);
        // a value of a parameter set: the record gets its own, and the set stays as it is for the other records
        if (!value)
            for (auto& a : record.methods)
                if (a.method == ref.method)
                    value = &(a.parameters[ref.parameter] = current);
        value->value = value_si;
    });
}

auto ParameterHandle::uncertainty() const -> std::optional<double>
{
    const auto q = engine_->pimpl->valueOf(ref_);
    if (!q.uncertainty)
        return std::nullopt;
    return q.uncertainty->standard;
}

auto ThermoEngine::parameter(const std::string& symbol, const std::string& method, const std::string& parameter) -> ParameterHandle
{
    pimpl->dropStaleResults();
    const ParameterRef ref{symbol, method, parameter};
    pimpl->checkParameter(ref);
    pimpl->valueOf(ref);  // the record can use the method without this value
    return ParameterHandle(*this, ref);
}

auto ThermoEngine::parameters(const std::string& symbol) -> std::vector<ParameterHandle>
{
    pimpl->dropStaleResults();
    const auto& db = pimpl->database;
    if (!db.containsSubstance(symbol) && !db.containsReaction(symbol))
        funError("Cannot list the parameters of " + symbol, "The database has no such record.", __LINE__, __FILE__);
    std::vector<v2::RecordValue> values;
    if (db.containsSubstance(symbol))
        values = v2::recordValues(*db.getSubstance(symbol).record());
    if (db.containsReaction(symbol))
        for (auto& value : v2::recordValues(*db.getReaction(symbol).record()))
            values.push_back(std::move(value));
    std::vector<ParameterHandle> out;
    for (const auto& value : values)
    {
        if (!value.ref.method.empty())
        {
            const auto* spec = v2::MethodCatalog::instance().find(value.ref.method);
            if (!spec || spec->role != v2::MethodRole::SubstanceProperties)
                continue;
            const auto base = value.ref.parameter.substr(0, value.ref.parameter.find('['));
            const auto p = std::find_if(spec->parameters.begin(), spec->parameters.end(), [&](const auto& x) { return x.name == base; });
            if (p != spec->parameters.end() && !p->fittable)
                continue;
        }
        out.emplace_back(*this, value.ref);
    }
    return out;
}

auto ThermoEngine::commitParameters() const -> Database
{
    return pimpl->database;
}

auto ThermoEngine::setParameterCovariance(const std::vector<ParameterRef>& parameters, const std::vector<double>& covariance) -> std::size_t
{
    const std::size_t n = parameters.size();
    if (covariance.size() != n * n)
        funError("Cannot set the covariance of parameters", std::to_string(covariance.size()) + " values for " + std::to_string(n) + " parameters.", __LINE__, __FILE__);
    std::map<std::string, std::vector<std::size_t>> byRecord;
    for (std::size_t i = 0; i < n; ++i)
    {
        pimpl->valueOf(parameters[i]);
        byRecord[parameters[i].symbol + (pimpl->isOfReaction(parameters[i]) ? "/r" : "/s")].push_back(i);
    }
    std::size_t dropped = 0;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            if (covariance[i * n + j] != 0.0 && (parameters[i].symbol != parameters[j].symbol || pimpl->isOfReaction(parameters[i]) != pimpl->isOfReaction(parameters[j])))
                ++dropped;
    for (const auto& [record, places] : byRecord)
    {
        (void)record;
        const auto first = parameters[places.front()];
        pimpl->editValue(first, [&](auto& r) {
            v2::CovarianceBlock block;
            const auto values = v2::recordValues(r);
            for (const auto i : places)
            {
                auto* q = v2::findValue(r, parameters[i]);
                if (!q)
                    continue;
                if (!q->uncertainty)
                    q->uncertainty = v2::Uncertainty{};
                q->uncertainty->standard = std::sqrt(std::max(covariance[i * n + i], 0.0));
                const auto it = std::find_if(values.begin(), values.end(), [&](const auto& v) { return v.ref.name() == parameters[i].name(); });
                block.paths.push_back(it->path);
                for (const auto j : places)
                    block.matrix.push_back(covariance[i * n + j]);
            }
            // a block with one of these values is out of date
            r.covariance.erase(std::remove_if(r.covariance.begin(), r.covariance.end(), [&](const auto& old) {
                return std::any_of(old.paths.begin(), old.paths.end(), [&](const auto& path) { return std::find(block.paths.begin(), block.paths.end(), path) != block.paths.end(); });
            }), r.covariance.end());
            if (places.size() > 1)
                r.covariance.push_back(block);
        });
    }
    return dropped;
}

auto ThermoEngine::numberOfCachedResults() const -> std::size_t
{
    return pimpl->substance_results.size() + pimpl->reaction_results.size() + pimpl->solvent_results.size() + pimpl->electro_results.size();
}

} // namespace ThermoFun
