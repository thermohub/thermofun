// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "SubstanceCompat.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>

#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/Legacy/LegacyMethodMap.h>
#include <ThermoFun/IO/Legacy/LegacyReader.h>
#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::compat {

namespace {

// The legacy method of an application: by the name as written in the file, or the first one with this id
auto legacyMethodOf(const v2::MethodApplication& a) -> const v2::legacy::LegacyMethod*
{
    // the name, if it agrees with the method: a file can have the number of one method and the name of another, and
    // the number counts. The water vapour rule of the reader also changes the method of an HKF entry.
    const auto& methods = v2::legacy::legacySubstanceMethods();
    for (const auto& m : methods)
        if (a.method_as_written == m.name && a.method == m.method && a.implementation == m.implementation)
            return &m;
    for (const auto& m : methods)
        if (a.method_as_written == m.name && a.method != m.method)
            return &m;
    for (const auto& m : methods)
        if (a.method == m.method && (a.implementation.empty() || a.implementation == m.implementation))
            return &m;
    return nullptr;
}

auto siUnitOf(const v2::MethodSpec* spec, const std::string& name) -> std::string
{
    if (spec)
        for (const auto& p : spec->parameters)
            if (p.name == name)
                return p.si_unit;
    return "";
}

auto perMole(const std::string& unit) -> std::string
{
    return unit == "J/bar" ? "J/(mol*bar)" : unit;
}

// Sets one place of an array, and makes the array long enough
void setAt(std::vector<double>& array, int index, double value)
{
    if (array.size() <= static_cast<size_t>(index))
        array.resize(index + 1, 0.0);
    array[index] = value;
}

auto arrayOf(ThermoParametersSubstance& p, const std::string& key) -> std::vector<double>*
{
    if (key == "eos_hkf_coeffs") return &p.HKF_parameters;
    if (key == "eos_akinfiev_diamond_coeffs") return &p.Cp_nonElectrolyte_coeff;
    if (key == "m_volume_fpt_coeffs") return &p.volume_coeff;
    if (key == "eos_birch_murnaghan_coeffs") return &p.volume_BirchM_coeff;
    if (key == "eos_churakov_gottschalk_coeffs") return &p.empirical_coeff;
    if (key == "eos_tait_hp11_coeffs") return &p.volume_Tait_coeff;
    if (key == "eos_extended_uniquac_coeffs") return &p.uniquac_coeff;
    if (key == "eos_gas_crit_props") return &p.critical_parameters;
    if (key == "m_landau_phase_trans_props") return &p.m_landau_phase_trans_props;
    if (key == "solute_holland_powell98_coeff") return &p.solute_holland_powell98_coeff;
    return nullptr;
}

auto arrayOf(ThermoParametersReaction& p, const std::string& key) -> std::vector<double>*
{
    if (key == "logk_ft_coeffs") return &p.reaction_logK_fT_coeff;
    if (key == "dr_heat_capacity_ft_coeffs") return &p.reaction_Cp_fT_coeff;
    if (key == "dr_volume_fpt_coeffs") return &p.reaction_V_fT_coeff;
    if (key == "dr_ryzhenko_coeffs") return &p.reaction_RB_coeff;
    if (key == "dr_marshall_franck_coeffs") return &p.reaction_FM_coeff;
    if (key == "dr_dolejs_manning10_coeffs") return &p.reaction_DM10_coeff;
    return nullptr;
}

auto legacyReactionMethodOf(const v2::MethodApplication& a) -> const v2::legacy::LegacyMethod*
{
    const auto& methods = v2::legacy::legacyReactionMethods();
    for (const auto& m : methods)
        if (a.method_as_written == m.name)
            return &m;
    for (const auto& m : methods)
        if (a.method == m.method)
            return &m;
    return nullptr;
}

// The values of one row of parameters in their array, in the units of the array, and their errors. row: the row of the
// errors of this array.
void fillArray(const v2::legacy::LegacyMethod& method, const v2::MethodSpec* spec, const std::map<std::string, v2::Quantity>& parameters,
               const std::string& key, std::vector<double>& array, std::map<std::string, std::vector<std::vector<double>>>& errors, size_t row = 0)
{
    for (const auto& p : method.parameters)
    {
        if (key != p.key)
            continue;
        const auto it = parameters.find(p.name);
        if (it == parameters.end())
            continue;
        const auto si = siUnitOf(spec, p.name);
        setAt(array, p.index, units::convertAny(it->second.value, si, perMole(p.unit)));
        if (it->second.uncertainty && it->second.uncertainty->standard > 0.0)
        {
            auto& rows = errors[key];
            if (rows.size() <= row)
                rows.resize(row + 1);
            setAt(rows[row], p.index, std::fabs(units::convertDifference(it->second.uncertainty->standard, si, perMole(p.unit))));
        }
    }
}

auto limitsOf(const v2::Segment& s) -> std::vector<double>
{
    std::vector<double> row;
    if (std::isfinite(s.t_min))
        row.push_back(s.t_min);
    if (std::isfinite(s.t_max))
        row.push_back(s.t_max);
    return row;
}

// The old codes that ask for no method, and the methods without code: the slot stays empty
auto asksForNothing(int code) -> bool
{
    return code == MethodGenEoS_Thrift::type::CTPM_CON || code == MethodCorrP_Thrift::type::CPM_OFF || code == MethodCorrP_Thrift::type::CPM_NUL ||
           code == MethodCorrT_Thrift::type::CTM_BER || code == MethodCorrT_Thrift::type::CTM_FEI;
}

// The legacy methods of the old codes, in the order of the slots
auto methodsOfCodes(const MethodCodes& codes, const std::vector<v2::legacy::LegacyMethod>& all, const char* what) -> std::vector<const v2::legacy::LegacyMethod*>
{
    std::vector<const v2::legacy::LegacyMethod*> out;
    for (const int code : {static_cast<int>(codes.general), static_cast<int>(codes.temperature), static_cast<int>(codes.pressure)})
    {
        if (code == 0 || asksForNothing(code))
            continue;
        const auto it = std::find_if(all.begin(), all.end(), [&](const auto& m) { return m.thrift_code == code; });
        if (it == all.end())
            throw std::invalid_argument("The old method code " + std::to_string(code) + " has no method for a " + what + ".");
        out.push_back(&*it);
    }
    return out;
}

auto errorsOf(const std::map<std::string, std::vector<std::vector<double>>>& errors, const std::string& key, size_t row) -> std::vector<double>
{
    const auto it = errors.find(key);
    return it == errors.end() || row >= it->second.size() ? std::vector<double>{} : it->second[row];
}

auto methodEntry(const v2::legacy::LegacyMethod& m) -> v2::LegacyMethodEntry
{
    v2::LegacyMethodEntry entry;
    entry.id = m.id;
    entry.name = m.name;
    return entry;
}

// The arrays that one entry of a method holds
template <class Parameters>
void addArrays(v2::LegacyMethodEntry& entry, const v2::legacy::LegacyMethod& m, Parameters& parameters, v2::OperatesOn operatesOn)
{
    const auto* spec = v2::MethodCatalog::instance().find(m.method, operatesOn);
    for (const auto& p : m.parameters)
    {
        const auto* array = arrayOf(parameters, p.key);
        if (!array || array->empty())
            continue;
        entry.arrays[p.key] = *array;
        entry.errors[p.key] = errorsOf(parameters.coefficient_errors, p.key, 0);
        // an empty array can wait for its values; one with values must have all that the method needs
        const bool otherKey = std::string(m.method) == "ChurakovGottschalk03" && std::string(p.key) != "eos_churakov_gottschalk_coeffs";
        if (spec && !otherKey && static_cast<size_t>(p.index) >= array->size())
            for (const auto& declared : spec->parameters)
                if (declared.name == p.name && declared.required)
                    throw std::invalid_argument(std::string("The array ") + p.key + " has " + std::to_string(array->size()) + " values: position " +
                                                std::to_string(p.index) + " (" + p.name + ") of the method " + m.method + " is missing.");
    }
}

// One entry for each segment of a piecewise method
void addPiecewise(std::vector<v2::LegacyMethodEntry>& entries, const v2::legacy::LegacyMethod& m, const ThermoParametersSubstance& p)
{
    const bool nasa = std::string(m.method) == "NASA9";
    const auto& rows = nasa ? p.nasa_coeff : p.Cp_coeff;
    const auto& intervals = nasa ? p.nasa_temperature_intervals : p.temperature_intervals;
    const std::string key = nasa ? "nasa_polynomial_coeffs" : "m_heat_capacity_ft_coeffs";
    const size_t transitions = nasa ? 0 : p.phase_transition_prop.size();
    const size_t n = std::max({rows.size(), intervals.size(), transitions, size_t(1)});
    for (size_t i = 0; i < n; ++i)
    {
        auto entry = methodEntry(m);
        if (i < rows.size())
        {
            entry.arrays[key] = rows[i];
            entry.errors[key] = errorsOf(p.coefficient_errors, key, i);
        }
        if (i < intervals.size())
            entry.temperature_limits = intervals[i];
        if (i < transitions)
            entry.phase_transition = p.phase_transition_prop[i];
        entries.push_back(entry);
    }
}

// What the old arrays cannot hold stays, for a method that stays
void keepWhatArraysCannotHold(std::vector<v2::MethodApplication>& methods, const std::vector<v2::MethodApplication>& before,
                              const std::vector<v2::legacy::LegacyMethod>& all)
{
    for (auto& a : methods)
    {
        const auto old = std::find_if(before.begin(), before.end(), [&](const auto& b) { return b.method == a.method; });
        if (old == before.end())
            continue;
        a.method_as_written = old->method_as_written;
        a.parameter_set = old->parameter_set;
        a.applicability = old->applicability;
        a.interpolation = old->interpolation;
        a.interpolation_order = old->interpolation_order;
        a.extra_json = old->extra_json;
        if (!a.table)
            a.table = old->table;
        for (const auto& [name, value] : old->parameters)
        {
            const bool inArrays = std::any_of(all.begin(), all.end(), [&, &name = name](const auto& m) {
                return a.method == m.method && std::any_of(m.parameters.begin(), m.parameters.end(), [&](const auto& p) { return name == p.name; });
            });
            if (!inArrays && name != "state" && name != "alpha0" && name != "kappa0")
                a.parameters.emplace(name, value);
        }
    }
}

void setProperty(std::optional<v2::Quantity> quantity, const Reaktoro_::ThermoProperty& from, double factor, const std::function<void(v2::Quantity)>& store)
{
    if (from.val == 0.0 && from.err == 0.0 && from.sta.first == Reaktoro_::Status::notdefined)
        return;
    v2::Quantity q = quantity.value_or(v2::Quantity{});
    q.value = from.val * factor;
    q.value_as_written.clear();
    if (from.err != 0.0)
    {
        v2::Uncertainty u;
        u.standard = std::fabs(from.err * factor);
        q.uncertainty = u;
    }
    else
        q.uncertainty.reset();
    store(q);
}

} // namespace

auto methodCodes(const v2::SubstanceRecord& record) -> MethodCodes
{
    MethodCodes out;
    for (const auto& a : record.methods)
    {
        const auto* m = legacyMethodOf(a);
        if (!m || m->thrift_code == 0)
            continue;
        if (m->thrift_code < 200)
            out.general = static_cast<MethodGenEoS_Thrift::type>(m->thrift_code);
        else if (m->thrift_code < 300)
            out.temperature = static_cast<MethodCorrT_Thrift::type>(m->thrift_code);
        else
            out.pressure = static_cast<MethodCorrP_Thrift::type>(m->thrift_code);
        // a water equation of state in the state of a gas: the legacy flag fug_critical_param
        if (const auto state = a.parameters.find("state"); state != a.parameters.end() && state->second.value == 1.0 && record.class_ != v2::SubstanceClass::WaterSolvent)
            out.pressure = MethodCorrP_Thrift::type::CPM_GAS;
    }
    return out;
}

auto thermoParameters(const v2::SubstanceRecord& record) -> ThermoParametersSubstance
{
    ThermoParametersSubstance out;
    for (const auto& a : record.methods)
    {
        const auto* method = legacyMethodOf(a);
        if (!method)
            continue;
        const auto* spec = v2::MethodCatalog::instance().find(a.method, v2::OperatesOn::Substance);
        for (const auto& p : method->parameters)
        {
            // the coefficients of Churakov and Gottschalk have two possible keys: one array in the old API
            if (a.method == "ChurakovGottschalk03" && std::string(p.key) != "eos_churakov_gottschalk_coeffs")
                continue;
            if (auto* array = arrayOf(out, p.key))
                fillArray(*method, spec, a.parameters, p.key, *array, out.coefficient_errors);
        }

        if (a.method == "CpPolynomial" || a.method == "NASA9")
            for (const auto& segment : a.segments)
            {
                std::vector<double> row;
                const std::string key = a.method == "NASA9" ? "nasa_polynomial_coeffs" : "m_heat_capacity_ft_coeffs";
                fillArray(*method, spec, segment.parameters, key, row, out.coefficient_errors, a.method == "NASA9" ? out.nasa_coeff.size() : out.Cp_coeff.size());
                if (a.method == "NASA9")
                {
                    out.nasa_coeff.push_back(row);
                    out.nasa_temperature_intervals.push_back(limitsOf(segment));
                }
                else
                {
                    out.temperature_intervals.push_back(limitsOf(segment));
                    if (!row.empty())
                        out.Cp_coeff.push_back(row);
                }
            }
        for (const auto& t : a.transitions)
        {
            static const std::pair<const char*, std::pair<const char*, const char*>> columns[] = {
                {"T", {"K", "K"}}, {"dS", {"J/(mol.K)", "J/(mol*K)"}}, {"dH", {"J/mol", "J/mol"}}, {"dV", {"m3", "J/bar"}}, {"dTdP", {"K/Pa", "K/bar"}}};
            std::vector<double> row;
            int i = 0;
            for (const auto& [name, unit] : columns)
            {
                if (const auto it = t.find(name); it != t.end())
                    setAt(row, i, units::convertAny(it->second.value, unit.first, unit.second));
                ++i;
            }
            if (!row.empty())
                out.phase_transition_prop.push_back(row);
        }
        if (a.table)
        {
            out.tp_table.temperatures = a.table->temperatures;
            out.tp_table.pressures = a.table->pressures;
            static const std::pair<const char*, const char*> names[] = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"V", "volume"}, {"Cp", "heat_capacity_cp"}};
            for (const auto& [from, to] : names)
                if (const auto it = a.table->values.find(from); it != a.table->values.end())
                    out.tp_table.values[to] = it->second;
            out.tp_table.reference_pressure = a.table->reference_pressure.value_or(0.0);
        }
        // the record-level m_expansivity and m_compressibility of the old API
        if (const auto it = a.parameters.find("alpha0"); it != a.parameters.end())
            out.isobaric_expansivity = it->second.value * 1e5;
        if (const auto it = a.parameters.find("kappa0"); it != a.parameters.end())
            out.isothermal_compresibility = it->second.value * 1e-8;
    }
    // the values that no method of the record declares
    if (const auto it = record.structural.find("alpha0"); it != record.structural.end())
        out.isobaric_expansivity = it->second.value * 1e5;
    if (const auto it = record.structural.find("kappa0"); it != record.structural.end())
        out.isothermal_compresibility = it->second.value * 1e-8;
    return out;
}

auto thermoReferenceProperties(const v2::SubstanceRecord& record) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance out;
    auto load = [&](v2::PropertyName name, Reaktoro_::ThermoProperty& to, double factor = 1.0) {
        const auto it = record.standard_state_properties.find(name);
        if (it == record.standard_state_properties.end())
            return;
        to.val = it->second.value * factor;
        to.sta = {Reaktoro_::Status::read, ""};
        if (it->second.uncertainty)
            to.err = it->second.uncertainty->standard * factor;
    };
    load(v2::PropertyName::G, out.gibbs_energy);
    load(v2::PropertyName::H, out.enthalpy);
    load(v2::PropertyName::S, out.entropy);
    load(v2::PropertyName::Cp, out.heat_capacity_cp);
    load(v2::PropertyName::V, out.volume, 1e5);
    return out;
}

auto molarMass(const v2::SubstanceRecord& record) -> double
{
    return record.molar_mass ? record.molar_mass->value * 1e3 : 0.0;
}

auto thermoCalculationType(const v2::SubstanceRecord& record) -> SubstanceThermoCalculationType::type
{
    return !record.defining_reaction.empty() && record.methods.empty() ? SubstanceThermoCalculationType::type::REACDC
                                                                       : SubstanceThermoCalculationType::type::DCOMP;
}

auto methodCodes(const v2::ReactionRecord& record) -> MethodCodes
{
    MethodCodes out;
    for (const auto& a : record.methods)
    {
        const auto* m = legacyReactionMethodOf(a);
        if (!m || m->thrift_code == 0)
            continue;
        if (m->thrift_code < 200)
            out.general = static_cast<MethodGenEoS_Thrift::type>(m->thrift_code);
        else if (m->thrift_code < 300)
            out.temperature = static_cast<MethodCorrT_Thrift::type>(m->thrift_code);
        else
            out.pressure = static_cast<MethodCorrP_Thrift::type>(m->thrift_code);
        // the reader keeps the coefficients of logk_fpt_function in a method of another form
        if (a.method.rfind("LogK", 0) == 0 && (a.parameters.count("a0") || a.parameters.count("a")))
            out.general = static_cast<MethodGenEoS_Thrift::type>(v2::legacy::legacyReactionMethod(0)->thrift_code);
    }
    return out;
}

auto thermoParameters(const v2::ReactionRecord& record) -> ThermoParametersReaction
{
    ThermoParametersReaction out;
    for (const auto& a : record.methods)
    {
        const auto* method = legacyReactionMethodOf(a);
        if (!method)
            continue;
        const auto* spec = v2::MethodCatalog::instance().find(a.method, v2::OperatesOn::Reaction);
        auto fill = [&](const v2::legacy::LegacyMethod& from) {
            for (const auto& p : from.parameters)
                if (auto* array = arrayOf(out, p.key))
                    if (const auto it = a.parameters.find(p.name); it != a.parameters.end())
                    {
                        const auto si = siUnitOf(spec, p.name);
                        setAt(*array, p.index, units::convertAny(it->second.value, si, perMole(p.unit)));
                        if (it->second.uncertainty && it->second.uncertainty->standard > 0.0)
                        {
                            auto& rows = out.coefficient_errors[p.key];
                            if (rows.empty())
                                rows.resize(1);
                            setAt(rows[0], p.index, std::fabs(units::convertDifference(it->second.uncertainty->standard, si, perMole(p.unit))));
                        }
                    }
        };
        fill(*method);
        // a log K method holds the coefficients of the merged logk_fpt_function and dr_heat_capacity_ft
        if (a.method.rfind("LogK", 0) == 0 || a.method == "ReactionCpPolynomial")
            for (const auto& m : v2::legacy::legacyReactionMethods())
                if (std::string(m.name) == "logk_fpt_function" || std::string(m.name) == "dr_heat_capacity_ft")
                    fill(m);
    }
    return out;
}

auto thermoReferenceProperties(const v2::ReactionRecord& record) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction out;
    auto load = [&](v2::ReactionPropertyName name, Reaktoro_::ThermoProperty& to, double factor = 1.0) {
        const auto it = record.reaction_properties.find(name);
        if (it == record.reaction_properties.end())
            return;
        to.val = it->second.value * factor;
        to.sta = {Reaktoro_::Status::read, ""};
        if (it->second.uncertainty)
            to.err = it->second.uncertainty->standard * factor;
    };
    load(v2::ReactionPropertyName::logK, out.log_equilibrium_constant);
    load(v2::ReactionPropertyName::dG, out.reaction_gibbs_energy);
    load(v2::ReactionPropertyName::dH, out.reaction_enthalpy);
    load(v2::ReactionPropertyName::dS, out.reaction_entropy);
    load(v2::ReactionPropertyName::dCp, out.reaction_heat_capacity_cp);
    load(v2::ReactionPropertyName::dV, out.reaction_volume, 1e5);
    return out;
}

auto reactants(const v2::ReactionRecord& record) -> std::map<std::string, double>
{
    std::map<std::string, double> out;
    for (const auto& [symbol, coefficient] : record.stoichiometry)
        out[symbol] = coefficient;
    return out;
}

void setMethods(v2::SubstanceRecord& record, const MethodCodes& codes, const ThermoParametersSubstance& parameters)
{
    const auto& all = v2::legacy::legacySubstanceMethods();
    auto p = parameters;
    std::vector<v2::LegacyMethodEntry> entries;
    for (const auto* m : methodsOfCodes(codes, all, "substance"))
    {
        if (std::string(m->method) == "CpPolynomial" || std::string(m->method) == "NASA9")
        {
            addPiecewise(entries, *m, p);
            continue;
        }
        auto entry = methodEntry(*m);
        addArrays(entry, *m, p, v2::OperatesOn::Substance);
        if (std::string(m->method) == "TPTable")
        {
            entry.has_table = true;
            entry.table_temperatures = p.tp_table.temperatures;
            entry.table_pressures = p.tp_table.pressures;
            entry.table_values = p.tp_table.values;
            entry.table_reference_pressure = p.tp_table.reference_pressure;
        }
        entries.push_back(entry);
    }
    const auto before = record.methods;
    v2::readLegacyMethods(entries, p.isobaric_expansivity, p.isothermal_compresibility, record);
    keepWhatArraysCannotHold(record.methods, before, all);
}

void setMethods(v2::ReactionRecord& record, const MethodCodes& codes, const ThermoParametersReaction& parameters)
{
    const auto& all = v2::legacy::legacyReactionMethods();
    auto p = parameters;
    std::vector<v2::LegacyMethodEntry> entries;
    for (const auto* m : methodsOfCodes(codes, all, "reaction"))
    {
        auto entry = methodEntry(*m);
        addArrays(entry, *m, p, v2::OperatesOn::Reaction);
        entries.push_back(entry);
    }
    const auto before = record.methods;
    v2::readLegacyMethods(entries, record);
    keepWhatArraysCannotHold(record.methods, before, all);
}

void setThermoReferenceProperties(v2::SubstanceRecord& record, const ThermoPropertiesSubstance& properties)
{
    auto set = [&](v2::PropertyName name, const Reaktoro_::ThermoProperty& from, double factor = 1.0) {
        auto& slots = record.standard_state_properties;
        const auto it = slots.find(name);
        setProperty(it == slots.end() ? std::nullopt : std::optional(it->second), from, factor, [&](v2::Quantity q) { slots[name] = q; });
    };
    set(v2::PropertyName::G, properties.gibbs_energy);
    set(v2::PropertyName::H, properties.enthalpy);
    set(v2::PropertyName::S, properties.entropy);
    set(v2::PropertyName::Cp, properties.heat_capacity_cp);
    set(v2::PropertyName::V, properties.volume, 1e-5);
}

void setThermoReferenceProperties(v2::ReactionRecord& record, const ThermoPropertiesReaction& properties)
{
    auto set = [&](v2::ReactionPropertyName name, const Reaktoro_::ThermoProperty& from, double factor = 1.0) {
        auto& slots = record.reaction_properties;
        const auto it = slots.find(name);
        setProperty(it == slots.end() ? std::nullopt : std::optional(it->second), from, factor, [&](v2::Quantity q) { slots[name] = q; });
    };
    set(v2::ReactionPropertyName::logK, properties.log_equilibrium_constant);
    set(v2::ReactionPropertyName::dG, properties.reaction_gibbs_energy);
    set(v2::ReactionPropertyName::dH, properties.reaction_enthalpy);
    set(v2::ReactionPropertyName::dS, properties.reaction_entropy);
    set(v2::ReactionPropertyName::dCp, properties.reaction_heat_capacity_cp);
    set(v2::ReactionPropertyName::dV, properties.reaction_volume, 1e-5);
}

void addWaiting(ThermoParametersSubstance& parameters, const ThermoParametersSubstance& waiting)
{
    auto add = [](auto& to, const auto& from) {
        if (to.empty())
            to = from;
    };
    add(parameters.temperature_intervals, waiting.temperature_intervals);
    add(parameters.pressure_intervals, waiting.pressure_intervals);
    add(parameters.Cp_coeff, waiting.Cp_coeff);
    add(parameters.Cp_nonElectrolyte_coeff, waiting.Cp_nonElectrolyte_coeff);
    add(parameters.phase_transition_prop, waiting.phase_transition_prop);
    add(parameters.phase_transition_prop_Berman, waiting.phase_transition_prop_Berman);
    add(parameters.m_landau_phase_trans_props, waiting.m_landau_phase_trans_props);
    add(parameters.HKF_parameters, waiting.HKF_parameters);
    add(parameters.volume_coeff, waiting.volume_coeff);
    add(parameters.uniquac_coeff, waiting.uniquac_coeff);
    add(parameters.nasa_coeff, waiting.nasa_coeff);
    add(parameters.nasa_temperature_intervals, waiting.nasa_temperature_intervals);
    add(parameters.critical_parameters, waiting.critical_parameters);
    add(parameters.volume_BirchM_coeff, waiting.volume_BirchM_coeff);
    add(parameters.empirical_coeff, waiting.empirical_coeff);
    add(parameters.volume_Tait_coeff, waiting.volume_Tait_coeff);
    add(parameters.solute_holland_powell98_coeff, waiting.solute_holland_powell98_coeff);
    if (parameters.tp_table.temperatures.empty())
        parameters.tp_table = waiting.tp_table;
    if (parameters.isobaric_expansivity == 0.0)
        parameters.isobaric_expansivity = waiting.isobaric_expansivity;
    if (parameters.isothermal_compresibility == 0.0)
        parameters.isothermal_compresibility = waiting.isothermal_compresibility;
    parameters.coefficient_errors.insert(waiting.coefficient_errors.begin(), waiting.coefficient_errors.end());
}

void addWaiting(ThermoParametersReaction& parameters, const ThermoParametersReaction& waiting)
{
    auto add = [](auto& to, const auto& from) {
        if (to.empty())
            to = from;
    };
    add(parameters.temperature_intervals, waiting.temperature_intervals);
    add(parameters.pressure_intervals, waiting.pressure_intervals);
    add(parameters.reaction_logK_fT_coeff, waiting.reaction_logK_fT_coeff);
    add(parameters.logK_TP_array, waiting.logK_TP_array);
    add(parameters.reaction_Cp_fT_coeff, waiting.reaction_Cp_fT_coeff);
    add(parameters.reaction_V_fT_coeff, waiting.reaction_V_fT_coeff);
    add(parameters.reaction_RB_coeff, waiting.reaction_RB_coeff);
    add(parameters.reaction_FM_coeff, waiting.reaction_FM_coeff);
    add(parameters.reaction_DM10_coeff, waiting.reaction_DM10_coeff);
    parameters.coefficient_errors.insert(waiting.coefficient_errors.begin(), waiting.coefficient_errors.end());
}

} // namespace ThermoFun::compat
