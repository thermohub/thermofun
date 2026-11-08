// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads the methods of a record from TPMethods, and the rules of the old parser for them.

#include <ThermoFun/IO/Legacy/detail/LegacyReaderParts.h>

namespace ThermoFun::model::legacyreader {

auto numbersOf(const json& entry, const char* field) -> std::vector<std::optional<double>>
{
    std::vector<std::optional<double>> out;
    if (entry.contains(field) && entry[field].is_array())
        for (const auto& v : entry[field])
            out.push_back(v.is_number() ? std::optional<double>(v.get<double>()) : std::nullopt);
    return out;
}

// The unit of the i-th value of an array: the unit of the file, or the unit of the model if the file gives none
auto unitOfValue(size_t i, const std::vector<std::string>& from, const std::string& model) -> std::string
{
    std::string unit = i < from.size() ? from[i] : "";
    if (unit.empty())
        unit = model;
    return unit;
}

auto ownsKey(const legacy::LegacyMethod& method, const std::string& key) -> bool
{
    for (const auto& p : method.parameters)
        if (key == p.key)
            return true;
    return false;
}

// The SI unit of a parameter of a method
auto siUnitOf(const MethodSpec& spec, const std::string& name) -> std::string
{
    for (const auto& p : spec.parameters)
        if (p.name == name)
            return p.si_unit;
    return "";
}

// A legacy volume in J/bar is per mole
auto perMole(const std::string& unit) -> std::string
{
    return unit == "J/bar" ? "J/(mol*bar)" : unit;
}

// Reads the array of a key into the parameters of a method, in SI units. Values that no parameter takes are reported.
void readArray(const json& entry, const std::string& key, const legacy::LegacyMethod& method, const MethodSpec& spec,
               std::map<std::string, Quantity>& out, const Context& c)
{
    if (!entry.contains(key) || !entry[key].is_object())
        return;
    const auto& array = entry[key];
    const auto values = numbersOf(array, "values");
    const auto errors = numbersOf(array, "errors");
    if (values.size() > maxCoefficients)
    {
        report(c, Severity::Error, "LEG-QRK-009", key, "too many coefficients: " + std::to_string(values.size()));
        return;
    }

    std::vector<std::string> from;
    if (array.contains("units") && array["units"].is_array())
        for (const auto& u : array["units"])
            from.push_back(u.is_string() ? u.get<std::string>() : "");

    bool unitsShort = !from.empty() && from.size() < values.size();
    if (legacy::hkfUnitsAreWrong(key, from, c.diagnostics, where(c, key)))
    {
        from.clear();  // the units of the method are used
        unitsShort = false;
    }
    for (const auto& u : from)
        unitsShort = unitsShort || u.empty();
    if (unitsShort)
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-011", where(c, key), "the list of units is shorter than the list of values, or it has empty units");

    size_t used = 0;
    for (const auto& p : method.parameters)
    {
        if (key != p.key)
            continue;
        const size_t i = static_cast<size_t>(p.index);
        used = std::max(used, i + 1);
        if (i >= values.size() || !values[i])
            continue;
        const std::string si = siUnitOf(spec, p.name);
        const std::string unit = perMole(unitOfValue(i, from, p.unit));
        Quantity q;
        q.written.single_precision = isSinglePrecision(*values[i]);
        try
        {
            q.value = units::convertAny(*values[i], unit, si);
            if (i < from.size() && !from[i].empty())
                q.written.unit = from[i];
            if (i < errors.size() && errors[i] && *errors[i] > 0.0)
            {
                Uncertainty u;
                u.standard = std::fabs(units::convertDifference(*errors[i], unit, si)) / c.options.error_coverage_factor;
                u.coverage_factor_as_written = c.options.error_coverage_factor;
                u.convention_unknown = true;
                q.uncertainty = u;
            }
        }
        catch (const std::exception& e)
        {
            report(c, Severity::Error, "V2-UNIT-001", key, e.what());
            continue;
        }
        out[p.name] = q;
    }

    if (values.size() > used && used > 0)
        legacy::reportQuirk(c.diagnostics, "LEG-QRK-010", where(c, key),
                            "the array has " + std::to_string(values.size()) + " values, the method uses " + std::to_string(used));
}

// {lowerT, upperT, lowerP, upperP} of a limitsTP object; a missing bound is no limit
auto readLimits(const json& entry) -> Applicability
{
    Applicability a;
    if (!isPresent(entry, legacy::LimitsTP) || !entry[legacy::LimitsTP].is_object())
        return a;
    const auto& l = entry[legacy::LimitsTP];
    auto number = [&](const char* key) -> std::optional<double> {
        return isPresent(l, key) && l[key].is_number() ? std::optional<double>(l[key].get<double>()) : std::nullopt;
    };
    a.t_min = number("lowerT");
    a.t_max = number("upperT");
    a.p_min = number("lowerP");
    a.p_max = number("upperP");
    return a;
}

// The T limits of a segment: a limit that the file does not give is infinite
auto segmentOf(const json& entry) -> Segment
{
    const Applicability a = readLimits(entry);
    Segment s;
    s.t_min = a.t_min.value_or(-inf);
    s.t_max = a.t_max.value_or(inf);
    return s;
}

// A row of m_phase_trans_props: T, dS, dH, dV, dTdP
auto readTransition(const json& entry, const Context& c) -> std::map<std::string, Quantity>
{
    std::map<std::string, Quantity> row;
    static const char* names[] = {"T", "dS", "dH", "dV", "dTdP"};
    static const char* modelUnits[] = {"K", "J/(mol*K)", "J/mol", "J/bar", "K/bar"};
    static const char* si[] = {"K", "J/(mol.K)", "J/mol", "m3", "K/Pa"};
    const auto& array = entry[legacy::MPhaseTransProps];
    const auto values = numbersOf(array, "values");
    std::vector<std::string> from;
    if (array.contains("units") && array["units"].is_array())
        for (const auto& u : array["units"])
            from.push_back(u.is_string() ? u.get<std::string>() : "");
    for (size_t i = 0; i < 5 && i < values.size(); ++i)
    {
        if (!values[i])
            continue;
        try
        {
            Quantity q;
            q.value = units::convertAny(*values[i], unitOfValue(i, from, modelUnits[i]), si[i]);
            row[names[i]] = q;
        }
        catch (const std::exception& e)
        {
            report(c, Severity::Error, "V2-UNIT-001", legacy::MPhaseTransProps, e.what());
        }
    }
    return row;
}

auto readTable(const json& table) -> TableData
{
    TableData t;
    auto vec = [&](const char* key) {
        std::vector<double> v;
        if (table.contains(key) && table[key].is_array())
            v = table[key].get<std::vector<double>>();
        return v;
    };
    t.temperatures = vec("temperatures");
    t.pressures = vec("pressures");
    static const std::pair<const char*, const char*> names[] = {
        {"gibbs_energy", "G"}, {"enthalpy", "H"}, {"volume", "V"}, {"heat_capacity_cp", "Cp"}};
    for (const auto& n : names)
    {
        auto v = vec(n.first);
        if (!v.empty())
            t.values[n.second] = v;
    }
    if (table.contains("reference_pressure") && table["reference_pressure"].is_number())
        t.reference_pressure = table["reference_pressure"].get<double>();
    return t;
}

// Reads the keys of an entry that belong to its method into the application
void readEntryKeys(const json& entry, const legacy::LegacyMethod& method, const MethodSpec& spec, MethodApplication& app, const Context& c)
{
    std::map<std::string, Quantity> parameters;
    std::set<std::string> read;  // each key once
    for (const auto& p : method.parameters)
        if (entry.contains(p.key) && read.insert(p.key).second)
            readArray(entry, p.key, method, spec, parameters, c);

    if (spec.piecewise)
    {
        Segment s = segmentOf(entry);
        s.parameters = parameters;
        app.segments.push_back(s);
        if (entry.contains(legacy::MPhaseTransProps) && entry[legacy::MPhaseTransProps].is_object())
            app.transitions.push_back(readTransition(entry, c));
    }
    else
    {
        for (auto& p : parameters)
            app.parameters[p.first] = p.second;
        app.applicability = readLimits(entry);
        if (entry.contains(legacy::TpTable) && entry[legacy::TpTable].is_object())
            app.table = readTable(entry[legacy::TpTable]);
    }
}

auto readMethods(const json& j, OperatesOn operatesOn, const Context& c, MethodFlags& flags) -> std::vector<MethodApplication>
{
    std::vector<MethodApplication> apps;
    if (!isPresent(j, legacy::TPMethods) || !j[legacy::TPMethods].is_array())
        return apps;

    const MethodCatalog& catalog = MethodCatalog::instance();
    std::vector<EntryInfo> infos;

    for (const auto& entry : j[legacy::TPMethods])
    {
        if (!entry.is_object() || !entry.contains(legacy::Method) || entry[legacy::Method].is_null())
            continue;
        const auto& m = entry[legacy::Method];
        if (!(m.is_object() || m.is_array()) || m.empty() || m.begin()->is_null())
            continue;

        int id = -1;
        std::string name;
        try
        {
            id = std::stoi(m.begin().key());
            name = m.begin().value().is_string() ? m.begin().value().get<std::string>() : "";
        }
        catch (const std::exception&)
        {
            report(c, Severity::Error, "LEG-MTH-001", legacy::TPMethods, "the method id is not a number");
            continue;
        }
        const std::string trimmed = legacy::trimmedMethodName(name, c.diagnostics, where(c, legacy::TPMethods));
        const auto* method = operatesOn == OperatesOn::Substance ? legacy::legacySubstanceMethod(id) : legacy::legacyReactionMethod(id);
        if (!method)
        {
            report(c, Severity::Error, "LEG-MTH-001", legacy::TPMethods, "the legacy method id " + std::to_string(id) + " is not known");
            continue;
        }
        legacy::methodNameDisagrees(trimmed, method->name, c.diagnostics, where(c, legacy::TPMethods));

        // not a method: a flag of the record
        if (std::string(method->name) == "fug_critical_param")
        {
            flags.gas_critical = true;
            continue;
        }
        const MethodSpec* spec = *method->method ? catalog.find(method->method, operatesOn) : nullptr;
        if (!spec || !spec->implemented)
        {
            report(c, Severity::Warning, "LEG-MTH-002", legacy::TPMethods, "the method " + trimmed + " is not implemented, so it is not used");
            continue;
        }
        if (std::string(method->name) == "water_eos_iapws95_gems")
            report(c, Severity::Warning, "LEG-MTH-003", legacy::TPMethods, "water_eos_iapws95_gems is read as WaterIAPWS95 of Reaktoro");

        // a piecewise method is one application with a segment for each entry
        size_t index = apps.size();
        if (spec->piecewise)
            for (size_t i = 0; i < apps.size(); ++i)
                if (apps[i].method == spec->id)
                    index = i;
        if (index == apps.size())
        {
            MethodApplication app;
            app.method = spec->id;
            app.written.method = name;
            app.implementation = method->implementation;
            apps.push_back(app);
        }
        readEntryKeys(entry, *method, *spec, apps[index], c);
        infos.push_back({&entry, method, spec, index});
    }

    // keys of an entry that belong to another method of the record go to that method; others are reported
    for (const auto& info : infos)
        for (const auto& key : legacy::legacyCoefficientKeys())
        {
            if (!info.entry->contains(key) || ownsKey(*info.method, key))
                continue;
            bool placed = false;
            for (const auto& other : infos)
                if (other.application != info.application && ownsKey(*other.method, key))
                {
                    json entryOnly = json::object();
                    entryOnly[key] = (*info.entry)[key];
                    MethodApplication& target = apps[other.application];
                    std::map<std::string, Quantity> parameters;
                    readArray(entryOnly, key, *other.method, *other.spec, parameters, c);
                    if (other.spec->piecewise && !target.segments.empty())
                        for (auto& p : parameters)
                            target.segments.back().parameters.emplace(p.first, p.second);
                    else
                        for (auto& p : parameters)
                            target.parameters.emplace(p.first, p.second);
                    placed = true;
                    break;
                }
            if (!placed)
                report(c, Severity::Warning, "LEG-KEY-002", key, "the key " + key + " does not belong to the method " + info.spec->id + "; it is not used");
        }
    return apps;
}

auto findMethod(std::vector<MethodApplication>& apps, const char* id) -> MethodApplication*
{
    for (auto& a : apps)
        if (a.method == id)
            return &a;
    return nullptr;
}

// A record has one base method. The legacy logk_fpt_function and dr_heat_capacity_ft only hold coefficients for the
// log K method beside them, so their coefficients move to that method.
void mergeLogKMethods(std::vector<MethodApplication>& apps)
{
    MethodApplication* target = nullptr;
    for (const char* id : {"LogKNordstromMunoz88", "LogKConstantCp", "LogKVantHoff", "LogKZeroEntropy", "LogKConstant", "LogKPolynomial"})
        if (!target)
            target = findMethod(apps, id);
    if (!target)
        return;
    const std::string kept = target->method;
    for (const char* id : {"LogKPolynomial", "ReactionCpPolynomial"})
    {
        const auto* from = findMethod(apps, id);
        if (from && from->method != kept)
            target->parameters.insert(from->parameters.begin(), from->parameters.end());
    }
    apps.erase(std::remove_if(apps.begin(), apps.end(), [&](const MethodApplication& a) {
                   return a.method != kept && (a.method == "LogKPolynomial" || a.method == "ReactionCpPolynomial");
               }),
               apps.end());

    // the same beside a method with parameters of its own: only the coefficients of log K, a0 to a6, have other names
    auto* polynomial = findMethod(apps, "LogKPolynomial");
    MethodApplication* other = nullptr;
    for (const char* id : {"LogKTable", "MarshallFranck81", "RyzhenkoBryzgalin", "DolejsManning10"})
        if (!other)
            other = findMethod(apps, id);
    if (polynomial && other)
    {
        for (const auto& p : polynomial->parameters)
            if (p.first.size() == 2 && p.first[0] == 'a' && std::isdigit(static_cast<unsigned char>(p.first[1])))
                other->parameters.insert(p);
        apps.erase(apps.begin() + (polynomial - apps.data()));
    }
}

// Water vapour: HKF with the flag fug_critical_param, or with a water equation of state on a record that is not an
// aqueous solute, is water as a gas; so is a water equation of state on a record in the gas state.
void applyWaterVapourRule(SubstanceRecord& s, const MethodFlags& flags)
{
    auto isWaterEoS = [](const MethodApplication& a) { return a.method == "WaterHGK84" || a.method == "WaterIAPWS95" || a.method == "WaterZhangDuan05"; };
    auto* hkf = findMethod(s.methods, "HKF88");
    MethodApplication* water = nullptr;
    for (auto& a : s.methods)
        if (isWaterEoS(a))
            water = &a;

    const bool vapourByHkf = hkf && (flags.gas_critical || (water && s.class_ != SubstanceClass::AqueousSolute));
    if (vapourByHkf && !water)
    {
        hkf->method = "WaterHGK84";
        hkf->parameters.clear();
        water = hkf;
    }
    else if (vapourByHkf)
    {
        // the name of HKF stays on the water method: it says that this record is vapour
        water->written.method = hkf->written.method;
        s.methods.erase(s.methods.begin() + (hkf - s.methods.data()));
        water = nullptr;
    }

    for (auto& a : s.methods)
        if (isWaterEoS(a) && (vapourByHkf || s.aggregate_state == AggregateState::Gas))
        {
            Quantity state;
            state.value = 1.0;
            a.parameters["state"] = state;
        }
}

// alpha0 and kappa0 of the record-level keys m_expansivity and m_compressibility
void readHp98Values(const json& j, SubstanceRecord& s, const Context& c)
{
    std::optional<Quantity> alpha, kappa;

    if (isPresent(j, legacy::MExpansivity) && j[legacy::MExpansivity].is_object())
    {
        const auto& e = j[legacy::MExpansivity];
        const auto values = numbersOf(e, "values");
        std::string unit = "1/K";
        if (e.contains("units") && e["units"].is_array() && !e["units"].empty() && e["units"][0].is_string())
            unit = e["units"][0].get<std::string>();
        if (!values.empty() && values[0])
        {
            Quantity q;
            q.written.single_precision = isSinglePrecision(*values[0]);
            q.written.unit = unit == "1/K" ? "" : unit;
            try
            {
                if (legacy::expansivityLabelIsSwapped(unit, c.diagnostics, where(c, legacy::MExpansivity)))
                    q.value = *values[0];  // the value is in 1/K
                else
                {
                    // the legacy parser multiplies by 1e5 only for the swapped label
                    report(c, Severity::Warning, "LEG-QRK-012", legacy::MExpansivity, "the unit of m_expansivity is " + unit);
                    q.value = units::convertAny(*values[0], unit, "1/K") * 1e-5;
                }
                if (std::fabs(q.value) > emptyMarker)
                    alpha = q;
            }
            catch (const std::exception& ex)
            {
                report(c, Severity::Error, "V2-UNIT-001", legacy::MExpansivity, ex.what());
            }
        }
    }
    if (isPresent(j, legacy::MCompressibility) && j[legacy::MCompressibility].is_object())
    {
        const auto& e = j[legacy::MCompressibility];
        const auto values = numbersOf(e, "values");
        std::string unit = "kbar";
        if (e.contains("units") && e["units"].is_array() && !e["units"].empty() && e["units"][0].is_string())
            unit = e["units"][0].get<std::string>();
        if (!values.empty() && values[0])
        {
            Quantity q;
            q.written.single_precision = isSinglePrecision(*values[0]);
            q.written.unit = unit == "kbar" ? "" : unit;
            try
            {
                if (legacy::compressibilityLabelIsSwapped(unit, c.diagnostics, where(c, legacy::MCompressibility)))
                    unit = "kbar";
                q.value = units::convertAny(*values[0], unit, "Pa");
                if (std::fabs(*values[0]) > emptyMarker)
                    kappa = q;
            }
            catch (const std::exception& ex)
            {
                report(c, Severity::Error, "V2-UNIT-001", legacy::MCompressibility, ex.what());
            }
        }
    }

    // the values go to every method that declares them, and to the record if no method does
    bool alphaPlaced = false, kappaPlaced = false;
    for (auto& app : s.methods)
    {
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, OperatesOn::Substance);
        if (!spec)
            continue;
        for (const auto& p : spec->parameters)
        {
            if (alpha && p.name == "alpha0")
            {
                app.parameters["alpha0"] = *alpha;
                alphaPlaced = true;
            }
            if (kappa && p.name == "kappa0")
            {
                app.parameters["kappa0"] = *kappa;
                kappaPlaced = true;
            }
        }
    }
    if (alpha && !alphaPlaced)
        s.structural["alpha0"] = *alpha;
    if (kappa && !kappaPlaced)
        s.structural["kappa0"] = *kappa;
}

// The methods of a record, and what depends on them
void readSubstanceMethods(const json& j, SubstanceRecord& s, const Context& c)
{
    MethodFlags flags;
    s.methods = readMethods(j, OperatesOn::Substance, c, flags);
    applyWaterVapourRule(s, flags);
    readHp98Values(j, s, c);

    // from a reaction: a defining reaction and no methods, no method at all, or only a constant volume
    const bool onlyConstantVolume = s.methods.size() == 1 && s.methods[0].method == "ConstantVolume";
    if (s.methods.empty() || onlyConstantVolume)
        s.preferred_property_source = PropertySource::Reaction;
}

void readReactionMethods(const json& j, ReactionRecord& r, const Context& c)
{
    MethodFlags flags;
    r.methods = readMethods(j, OperatesOn::Reaction, c, flags);
    mergeLogKMethods(r.methods);
    if (r.methods.empty())
        r.preferred_property_source = PropertySource::Reactants;
}

} // namespace ThermoFun::model::legacyreader
