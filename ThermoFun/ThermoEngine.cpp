// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// ThermoFun includes
#include "ThermoEngineImpl.h"

namespace ThermoFun
{

// The checks of the settings: an error says what is allowed
template <class E>
auto checkedWord(const std::string& word) -> E
{
    const auto value = v2::fromString<E>(word);
    if (!value)
        throw std::invalid_argument(v2::unknownWordMessage<E>(word));
    return *value;
}

void checkPreferences(const EnginePreferences& p)
{
    if (p.solvent_state != 0 && p.solvent_state != 1)
        throw std::invalid_argument("The solvent state is " + std::to_string(p.solvent_state) + ": it must be 0 (liquid) or 1 (vapour).");
    if (p.uncertainty_significant_digits < 1)
        throw std::invalid_argument("The number of significant digits of an uncertainty is " + std::to_string(p.uncertainty_significant_digits) + ": it must be 1 or more.");
    for (const auto& [id, implementation] : p.method_implementation)
    {
        const auto* spec = v2::MethodCatalog::instance().find(id);
        if (!spec)
            throw std::invalid_argument("method_implementation: there is no method " + id + ".");
        const auto known = v2::MethodRegistry::instance().implementations(spec->id);
        if (std::find(known.begin(), known.end(), implementation) == known.end())
        {
            std::string message = "method_implementation: the method " + spec->id + " has no implementation \"" + implementation + "\". Allowed:";
            for (const auto& k : known)
                if (!k.empty())
                    message += " " + k;
            throw std::invalid_argument(message + ".");
        }
    }
}

/// A word set in an old field replaces the typed value, and the field is empty after it
void applyOldWords(EngineConventions& c)
{
    if (!c.apparent_properties.empty())
    {
        c.energy_reference = checkedWord<v2::EnergyReference>(c.apparent_properties);
        c.apparent_properties.clear();
    }
    if (!c.water_properties.empty())
    {
        c.water_reference = checkedWord<v2::WaterReference>(c.water_properties);
        c.water_properties.clear();
    }
}

void ThermoEngine::Impl::dropStaleResults()
{
    applyOldWords(conventions);
    const Database& db = database;
    // solvent_state is not compared: each calculation of a substance sets it (to vapor for a gas), so it is
    // working state of the engine, not a setting of the user
    auto settings = cached_for.preferences;
    settings.solvent_state = preferences.solvent_state;
    if (db.revision() == cached_for.database_revision && preferences == settings &&
        conventions == cached_for.conventions && db.conventions() == cached_for.database_conventions)
        return;
    checkPreferences(preferences);
    clearResults();
}

ThermoEngine::ThermoEngine(const std::string filename)
    : pimpl()
{
    Database db(filename);
    pimpl.reset(new Impl(db));
}

ThermoEngine::ThermoEngine(const Database &database)
    : pimpl(new Impl(database))
{
}

ThermoEngine::ThermoEngine(const ThermoEngine &other)
    : pimpl(new Impl(*other.pimpl))
{
    // the caches of results start empty
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    auto tps = pimpl->toOutput(pimpl->cachedSubstance(T, P, substance), T, P, pimpl->database.getSubstance(substance));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addSubstanceErrors(tps, T, P, pimpl->database.getSubstance(substance), pimpl->inputsOf(pimpl->recordsOfSubstance(substance)));
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->cachedElectro(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->cachedSolvent(T, P, solvent, state);
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, const Substance& substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    auto tps = pimpl->toOutput(pimpl->thermoPropertiesSubstance(T, P, substance), T, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
    {
        ThermoEngine::Impl::Records records;
        if (!substance.reactionSymbol().empty()) pimpl->collectReaction(records, substance.reactionSymbol());
        pimpl->addSubstanceErrors(tps, T, P, substance, pimpl->inputsOf(records, &substance));
    }
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->electroPropertiesSolvent(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    return pimpl->propertiesSolvent(T, P, solvent, state);
}

// Reaction
auto ThermoEngine::thermoPropertiesReaction(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    auto tpr = pimpl->toOutput(pimpl->cachedReaction(T, P, reaction), T, P, pimpl->reactionOfSymbol(reaction));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, pimpl->reactionOfSymbol(reaction), false, pimpl->inputsOf(pimpl->recordsOfReaction(reaction)));
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReactionFromReactants(T, P, symbol), T, P, pimpl->database.getReaction(symbol));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, pimpl->database.getReaction(symbol), true, pimpl->inputsOf(pimpl->recordsOfReaction(symbol)));
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReaction(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReaction(T, P, reaction), T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, reaction, false, pimpl->inputsOf(pimpl->recordsOfReactants(reaction), nullptr, &reaction));
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReactionFromReactants(T, P, reaction), T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, reaction, true, pimpl->inputsOf(pimpl->recordsOfReactants(reaction), nullptr, &reaction));
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::setPreferences(const EnginePreferences preferences) -> void
{
    checkPreferences(preferences);
    pimpl->preferences = preferences;
    pimpl->clearResults();
}

auto ThermoEngine::setConventions(const EngineConventions conventions) -> void
{
    auto checked = conventions;
    applyOldWords(checked);
    pimpl->conventions = checked;
    pimpl->clearResults();
}

auto ThermoEngine::setPropertySource(const std::string& symbol, v2::PropertySource source) -> void
{
    if (source == v2::PropertySource::Record)
    {
        pimpl->substance_sources.erase(symbol);
        pimpl->reaction_sources.erase(symbol);
    }
    if (source == v2::PropertySource::Direct || source == v2::PropertySource::Reaction)
        pimpl->substance_sources[symbol] = source;
    if (source == v2::PropertySource::Direct || source == v2::PropertySource::Reactants)
        pimpl->reaction_sources[symbol] = source;
    pimpl->clearResults();
}

auto ThermoEngine::setPropertySource(const std::string& symbol, const std::string& source) -> void
{
    setPropertySource(symbol, checkedWord<v2::PropertySource>(source));
}

auto ThermoEngine::propertySource(const std::string& symbol) const -> v2::PropertySource
{
    // the choice that is not the plain one, if a substance and a reaction with this symbol differ
    for (const auto* own : {&pimpl->reaction_sources, &pimpl->substance_sources})
        if (const auto it = own->find(symbol); it != own->end() && it->second != v2::PropertySource::Direct)
            return it->second;
    return pimpl->substance_sources.count(symbol) ? v2::PropertySource::Direct : pimpl->preferences.property_source;
}

auto ThermoEngine::setSolventSymbol(const std::string solvent_symbol) -> void
{
    pimpl->preferences.solvent_symbol = solvent_symbol;
}

auto ThermoEngine::solventSymbol() -> std::string&
{
    return pimpl->preferences.solvent_symbol;
}

EnginePreferences& ThermoEngine::preferences() {
    return pimpl->preferences;
}

const EnginePreferences& ThermoEngine::preferences() const {
    return pimpl->preferences;
}

auto ThermoEngine::database() const -> const Database &
{
    return pimpl->database;
}

auto ThermoEngine::appendData(std::string filename) -> void
{
    pimpl->database.appendData(filename);
}

auto ThermoEngine::appendData(std::vector<std::string> jsonRecords, std::string _label = "unknown label") -> void
{
    pimpl->database.appendData(jsonRecords, _label);
}

auto ThermoEngine::explain(double T, double P, std::string substance) const -> std::vector<ExplainStep>
{
    return thermoPropertiesSubstanceDetailed(T, P, substance).steps;
}

auto ThermoEngine::thermoPropertiesSubstanceDetailed(double T, double &P, const std::string& substance) const -> DetailedPropertiesSubstance
{
    pimpl->dropStaleResults();
    return pimpl->detailedSubstance(T, P, pimpl->database.getSubstance(substance));
}

auto ThermoEngine::snapshot() const -> CalculationSnapshot
{
    CalculationSnapshot s;
#ifdef THERMOFUN_VERSION_STRING
    s.thermofun_version = THERMOFUN_VERSION_STRING;
#endif
    const Database& database = pimpl->database;
    const auto& info = database.thermoDataSet();
    s.format_version = info.schema_version.empty() ? "legacy" : info.schema_version;
    s.dataset = info.symbol;
    s.dataset_version = info.version;
    s.content_hash = sha256(database.toJson(false));
    s.implementations = pimpl->preferences.method_implementation;
    return s;
}

auto ThermoEngine::consistencyReport(const std::vector<std::pair<double, double>>& TP, const ConsistencyOptions& options) const -> std::vector<ConsistencyItem>
{
    pimpl->dropStaleResults();
    ThermoEngine direct(*this), derived(*this);
    EnginePreferences p = preferences();
    p.enable_memoize = false;
    p.property_source = v2::PropertySource::Direct;
    direct.setPreferences(p);
    p.property_source = v2::PropertySource::Reaction;
    derived.setPreferences(p);
    const Database& database = pimpl->database;

    std::vector<ConsistencyItem> items;
    // a calculation that fails is an item too: a record must not leave the report without a sign
    const auto compare = [&](const std::string& symbol, const char* kind, const std::pair<double, double>& point, auto&& differences) {
        ConsistencyItem item;
        item.symbol = symbol;
        item.kind = kind;
        item.T = point.first;
        item.P = point.second;
        try
        {
            differences(item);
            item.flag = std::fabs(item.delta_G) > options.delta_G_threshold ? "inconsistent" : "consistent";
        }
        catch (const std::exception& error)
        {
            item.flag = "not calculated";
            item.message = error.what();
        }
        items.push_back(item);
    };
    for (const auto& [symbol, substance] : database.mapSubstances())
    {
        const auto reaction = substance.reactionSymbol();
        // without data of its own or without a reaction, there are not two ways
        if (reaction.empty() || !database.containsReaction(reaction) || substance.record()->methods.empty())
            continue;
        for (const auto& point : TP)
            compare(symbol, "substance_vs_reaction", point, [&, symbol = symbol](ConsistencyItem& item) {
                double P1 = point.second, P2 = point.second;
                item.delta_G = direct.thermoPropertiesSubstance(point.first, P1, symbol).gibbs_energy.val - derived.thermoPropertiesSubstance(point.first, P2, symbol).gibbs_energy.val;
            });
    }
    for (const auto& [symbol, reaction] : database.mapReactions())
    {
        if (reaction.record()->methods.empty() || reaction.reactants().empty())
            continue;
        for (const auto& point : TP)
            compare(symbol, "reaction_vs_reactants", point, [&, symbol = symbol](ConsistencyItem& item) {
                double P1 = point.second, P2 = point.second;
                const auto a = direct.thermoPropertiesReaction(point.first, P1, symbol);
                const auto b = direct.thermoPropertiesReactionFromReactants(point.first, P2, symbol);
                item.delta_G = a.reaction_gibbs_energy.val - b.reaction_gibbs_energy.val;
                item.delta_logK = a.log_equilibrium_constant.val - b.log_equilibrium_constant.val;
            });
    }
    return items;
}

auto ThermoEngine::parseSubstanceFormula(std::string formula) const -> std::map<Element, double>
{
    return pimpl->database.parseSubstanceFormula(formula);
}

EngineConventions& ThermoEngine::conventions() {
    return pimpl->conventions;
}

const EngineConventions& ThermoEngine::conventions() const {
    return pimpl->conventions;
}

} // namespace ThermoFun
