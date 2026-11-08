// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// ThermoFun includes
#include "ThermoEngineImpl.h"
#include <ThermoFun/Common/PhysicalConstants.h>

namespace ThermoFun
{

namespace {

/// The public results give V in m3/mol, SI as P in Pa (so dG/dP = V); inside, the engine works in J/bar (1 J/bar = 1e-5 m3)
void volumeToSI(Reaktoro_::ThermoProperty& V)
{
    V.val *= 1e-5;
    V.ddt *= 1e-5;
    V.ddp *= 1e-5;
    V.err *= 1e-5;
}

} // namespace


// The checks of the settings: an error says what is allowed
template <class E>
auto checkedWord(const std::string& word) -> E
{
    const auto value = model::fromString<E>(word);
    if (!value)
        throw std::invalid_argument(model::unknownWordMessage<E>(word));
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
        const auto* spec = model::MethodCatalog::instance().find(id);
        if (!spec)
            throw std::invalid_argument("method_implementation: there is no method " + id + ".");
        const auto known = model::MethodRegistry::instance().implementations(spec->id);
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
        c.energy_reference = checkedWord<model::EnergyReference>(c.apparent_properties);
        c.apparent_properties.clear();
    }
    if (!c.water_properties.empty())
    {
        c.water_reference = checkedWord<model::WaterReference>(c.water_properties);
        c.water_properties.clear();
    }
}

void ThermoEngine::Impl::dropStaleResults()
{
    applyOldWords(conventions);
    const Database& db = database;
    if (db.revision() == cached_for.database_revision && preferences == cached_for.preferences &&
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

auto ThermoEngine::operator=(const ThermoEngine& other) -> ThermoEngine&
{
    if (this != &other)
        pimpl.reset(new Impl(*other.pimpl));
    return *this;
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tps = pimpl->toOutput(pimpl->cachedSubstance(T, P, substance), T, P, pimpl->database.getSubstance(substance));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addSubstanceErrors(tps, T, P, pimpl->database.getSubstance(substance), pimpl->inputsOf(pimpl->recordsOfSubstance(substance)));
    volumeToSI(tps.volume);
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    return pimpl->cachedElectro(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    return pimpl->cachedSolvent(T, P, solvent, state);
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, const Substance& substance) const -> ThermoPropertiesSubstance
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tps = pimpl->toOutput(pimpl->thermoPropertiesSubstance(T, P, substance), T, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
    {
        ThermoEngine::Impl::Records records;
        if (!substance.reactionSymbol().empty()) pimpl->collectReaction(records, substance.reactionSymbol());
        pimpl->addSubstanceErrors(tps, T, P, substance, pimpl->inputsOf(records, &substance));
    }
    volumeToSI(tps.volume);
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> ElectroPropertiesSolvent
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    return pimpl->electroPropertiesSolvent(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> PropertiesSolvent
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    return pimpl->propertiesSolvent(T, P, solvent, state);
}

// Reaction
auto ThermoEngine::thermoPropertiesReaction(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tpr = pimpl->toOutput(pimpl->cachedReaction(T, P, reaction), T, P, pimpl->reactionOfSymbol(reaction));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, pimpl->reactionOfSymbol(reaction), false, pimpl->inputsOf(pimpl->recordsOfReaction(reaction)));
    volumeToSI(tpr.reaction_volume);
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReactionFromReactants(T, P, symbol), T, P, pimpl->reactionOfSymbol(symbol));
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, pimpl->reactionOfSymbol(symbol), true, pimpl->inputsOf(pimpl->recordsOfReaction(symbol)));
    volumeToSI(tpr.reaction_volume);
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReaction(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReaction(T, P, reaction), T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, reaction, false, pimpl->inputsOf(pimpl->recordsOfReactants(reaction), nullptr, &reaction));
    volumeToSI(tpr.reaction_volume);
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, P);
    auto tpr = pimpl->toOutput(pimpl->thermoPropertiesReactionFromReactants(T, P, reaction), T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addReactionErrors(tpr, T, P, reaction, true, pimpl->inputsOf(pimpl->recordsOfReactants(reaction), nullptr, &reaction));
    volumeToSI(tpr.reaction_volume);
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

auto ThermoEngine::setPropertySource(const std::string& symbol, model::PropertySource source) -> void
{
    if (source == model::PropertySource::Record)
    {
        pimpl->substance_sources.erase(symbol);
        pimpl->reaction_sources.erase(symbol);
    }
    if (source == model::PropertySource::Direct || source == model::PropertySource::Reaction)
        pimpl->substance_sources[symbol] = source;
    if (source == model::PropertySource::Direct || source == model::PropertySource::Reactants)
        pimpl->reaction_sources[symbol] = source;
    pimpl->clearResults();
}

auto ThermoEngine::setPropertySource(const std::string& symbol, const std::string& source) -> void
{
    setPropertySource(symbol, checkedWord<model::PropertySource>(source));
}

auto ThermoEngine::propertySource(const std::string& symbol) const -> model::PropertySource
{
    // the choice that is not the plain one, if a substance and a reaction with this symbol differ
    for (const auto* own : {&pimpl->reaction_sources, &pimpl->substance_sources})
        if (const auto it = own->find(symbol); it != own->end() && it->second != model::PropertySource::Direct)
            return it->second;
    return pimpl->substance_sources.count(symbol) ? model::PropertySource::Direct : pimpl->preferences.property_source;
}

auto ThermoEngine::setSolventSymbol(const std::string solvent_symbol) -> void
{
    pimpl->preferences.solvent_symbol = solvent_symbol;
}

auto ThermoEngine::saturationPressure(double T) const -> double
{
    pimpl->dropStaleResults();
    Impl::checkStatePoint(T, 0.0);
    return pimpl->saturationPressure(T);
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
    Impl::checkStatePoint(T, P);
    auto detailed = pimpl->detailedSubstance(T, P, pimpl->database.getSubstance(substance));
    volumeToSI(detailed.properties.volume);
    return detailed;
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
    s.physical_constants = PHYSICAL_CONSTANTS_NAME;
    const auto& p = pimpl->preferences;
    pimpl->dropStaleResults();  // the solvent in use is chosen there
    if (const auto& water = pimpl->solvent_in_use; database.containsSubstance(water))
    {
        s.water_symbol = water;
        s.water_methods = calculationPath(water).methods;
    }
    const EnginePreferences defaults;
    const auto word = [](bool value) { return std::string(value ? "true" : "false"); };
    if (p.solvent_state != defaults.solvent_state) s.options["solvent_state"] = std::to_string(p.solvent_state);
    if (p.fallback_to_reference_properties != defaults.fallback_to_reference_properties) s.options["fallback_to_reference_properties"] = word(p.fallback_to_reference_properties);
    if (p.apply_pressure_correction_to_gas_props != defaults.apply_pressure_correction_to_gas_props) s.options["apply_pressure_correction_to_gas_props"] = word(p.apply_pressure_correction_to_gas_props);
    if (p.cancel_fugacity_coefficient_in_gas_props != defaults.cancel_fugacity_coefficient_in_gas_props) s.options["cancel_fugacity_coefficient_in_gas_props"] = word(p.cancel_fugacity_coefficient_in_gas_props);
    if (p.round_to_uncertainty != defaults.round_to_uncertainty) s.options["round_to_uncertainty"] = word(p.round_to_uncertainty);
    if (p.uncertainty_significant_digits != defaults.uncertainty_significant_digits) s.options["uncertainty_significant_digits"] = std::to_string(p.uncertainty_significant_digits);
    if (p.propagate_parameter_errors != defaults.propagate_parameter_errors) s.options["propagate_parameter_errors"] = word(p.propagate_parameter_errors);
    if (p.property_source != defaults.property_source) s.options["property_source"] = model::toString(p.property_source);
    const auto& c = pimpl->conventions;
    s.conventions["energy_reference"] = model::toString(c.energy_reference);
    s.conventions["standard_pressure"] = model::toString(c.standard_pressure);
    s.conventions["concentration_scale"] = model::toString(c.concentration_scale);
    s.conventions["water_reference"] = model::toString(c.water_reference);
    return s;
}

auto snapshotLines(const CalculationSnapshot& s) -> std::vector<std::string>
{
    const auto joined = [](const std::map<std::string, std::string>& items) {
        std::string out;
        for (const auto& [key, value] : items)
            out += (out.empty() ? "" : ", ") + key + " = " + value;
        return out;
    };
    std::vector<std::string> lines;
    lines.push_back("ThermoFun " + (s.thermofun_version.empty() ? std::string("(version not known)") : s.thermofun_version) + ", physical constants " + s.physical_constants);
    std::string data = "data: format " + s.format_version;
    if (!s.dataset.empty())
        data += ", dataset " + s.dataset + (s.dataset_version.empty() ? "" : " " + s.dataset_version);
    lines.push_back(data + ", sha256 " + s.content_hash);
    if (!s.water_symbol.empty())
    {
        std::string water = "water: " + s.water_symbol + ":";
        for (std::size_t i = 0; i < s.water_methods.size(); ++i)
            water += (i ? ", " : " ") + s.water_methods[i];
        lines.push_back(water);
    }
    lines.push_back("conventions: " + joined(s.conventions));
    if (!s.options.empty())
        lines.push_back("options: " + joined(s.options));
    if (!s.implementations.empty())
        lines.push_back("implementations: " + joined(s.implementations));
    return lines;
}

auto ThermoEngine::consistencyReport(const std::vector<std::pair<double, double>>& TP, const ConsistencyOptions& options) const -> std::vector<ConsistencyItem>
{
    pimpl->dropStaleResults();
    ThermoEngine direct(*this), derived(*this);
    EnginePreferences p = preferences();
    p.enable_memoize = false;
    p.property_source = model::PropertySource::Direct;
    direct.setPreferences(p);
    p.property_source = model::PropertySource::Reaction;
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
