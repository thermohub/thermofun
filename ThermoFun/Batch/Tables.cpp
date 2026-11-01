// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/Batch/Tables.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Units.hpp>
#include <ThermoFun/Database.h>
#include <ThermoFun/IO/V2/V2Reader.h>
#include <ThermoFun/Model/Vocabulary.h>
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

namespace v2::detail {
struct BuiltinFile
{
    const char* name;
    const unsigned char* text;
    unsigned long size;
};
extern const BuiltinFile builtin_table_templates[];
extern const unsigned long builtin_table_template_count;
} // namespace v2::detail

namespace {

using json = nlohmann::ordered_json;

[[noreturn]] auto fail(const std::string& message) -> void
{
    throw TableSetupError(message);
}

using Scalar = Reaktoro_::ThermoProperty;

const std::map<std::string, std::string> substanceNames = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"S", "entropy"}, {"Cp", "heat_capacity_cp"}, {"Cv", "heat_capacity_cv"},
                                                           {"V", "volume"}, {"U", "internal_energy"}, {"A", "helmholtz_energy"}};
const std::map<std::string, std::string> reactionNames = {{"logK", "log_equilibrium_constant"}, {"lnK", "ln_equilibrium_constant"}, {"dG", "reaction_gibbs_energy"},
                                                          {"dH", "reaction_enthalpy"}, {"dS", "reaction_entropy"}, {"dCp", "reaction_heat_capacity_cp"},
                                                          {"dCv", "reaction_heat_capacity_cv"}, {"dV", "reaction_volume"}, {"dU", "reaction_internal_energy"}, {"dA", "reaction_helmholtz_energy"}};
const std::map<std::string, std::string> baseUnits = {
    {"gibbs_energy", "J/mol"}, {"enthalpy", "J/mol"}, {"internal_energy", "J/mol"}, {"helmholtz_energy", "J/mol"}, {"entropy", "J/K/mol"}, {"heat_capacity_cp", "J/K/mol"},
    {"heat_capacity_cv", "J/K/mol"}, {"volume", "J/bar"}, {"reaction_gibbs_energy", "J/mol"}, {"reaction_enthalpy", "J/mol"}, {"reaction_internal_energy", "J/mol"},
    {"reaction_helmholtz_energy", "J/mol"}, {"reaction_entropy", "J/K/mol"}, {"reaction_heat_capacity_cp", "J/K/mol"}, {"reaction_heat_capacity_cv", "J/K/mol"},
    {"reaction_volume", "J/bar"}, {"density", "kg/m3"}, {"Alpha", "1/K"}, {"Beta", "1/Pa"}, {"dAldT", "1/K2"}, {"bornY", "1/K"}, {"bornQ", "1/Pa"}, {"bornX", "1/K2"},
    {"log_equilibrium_constant", "1"}, {"ln_equilibrium_constant", "1"}, {"epsilon", "1"}, {"bornZ", "1"}};

auto substanceScalar(const ThermoPropertiesSubstance& x, const std::string& n) -> const Scalar*
{
    if (n == "gibbs_energy") return &x.gibbs_energy;
    if (n == "enthalpy") return &x.enthalpy;
    if (n == "entropy") return &x.entropy;
    if (n == "heat_capacity_cp") return &x.heat_capacity_cp;
    if (n == "heat_capacity_cv") return &x.heat_capacity_cv;
    if (n == "volume") return &x.volume;
    if (n == "internal_energy") return &x.internal_energy;
    if (n == "helmholtz_energy") return &x.helmholtz_energy;
    return nullptr;
}

auto reactionScalar(const ThermoPropertiesReaction& x, const std::string& n) -> const Scalar*
{
    if (n == "log_equilibrium_constant") return &x.log_equilibrium_constant;
    if (n == "ln_equilibrium_constant") return &x.ln_equilibrium_constant;
    if (n == "reaction_gibbs_energy") return &x.reaction_gibbs_energy;
    if (n == "reaction_enthalpy") return &x.reaction_enthalpy;
    if (n == "reaction_entropy") return &x.reaction_entropy;
    if (n == "reaction_heat_capacity_cp") return &x.reaction_heat_capacity_cp;
    if (n == "reaction_heat_capacity_cv") return &x.reaction_heat_capacity_cv;
    if (n == "reaction_volume") return &x.reaction_volume;
    if (n == "reaction_internal_energy") return &x.reaction_internal_energy;
    if (n == "reaction_helmholtz_energy") return &x.reaction_helmholtz_energy;
    return nullptr;
}

auto solventScalar(const PropertiesSolvent& x, const std::string& n) -> const Scalar*
{
    if (n == "density") return &x.density;
    if (n == "densityT") return &x.densityT;
    if (n == "densityP") return &x.densityP;
    if (n == "densityTT") return &x.densityTT;
    if (n == "densityTP") return &x.densityTP;
    if (n == "densityPP") return &x.densityPP;
    if (n == "Alpha") return &x.Alpha;
    if (n == "Beta") return &x.Beta;
    if (n == "dAldT") return &x.dAldT;
    if (n == "Albe") return &x.Albe;
    return nullptr;
}

auto solventNumber(const PropertiesSolvent& x, const std::string& n) -> std::optional<double>
{
    if (n == "speed_of_sound") return x.speed_of_sound;
    if (n == "dynamic_viscosity") return x.dynamic_viscosity;
    if (n == "thermal_conductivity") return x.thermal_conductivity;
    if (n == "surface_tension") return x.surface_tension;
    if (n == "Tdiff") return x.Tdiff;
    if (n == "Prndtl") return x.Prndtl;
    if (n == "Visck") return x.Visck;
    return std::nullopt;
}

auto electroScalar(const ElectroPropertiesSolvent& x, const std::string& n) -> const Scalar*
{
    if (n == "epsilon") return &x.epsilon;
    if (n == "epsilonT") return &x.epsilonT;
    if (n == "epsilonP") return &x.epsilonP;
    if (n == "epsilonTT") return &x.epsilonTT;
    if (n == "epsilonTP") return &x.epsilonTP;
    if (n == "epsilonPP") return &x.epsilonPP;
    if (n == "bornZ") return &x.bornZ;
    if (n == "bornY") return &x.bornY;
    if (n == "bornQ") return &x.bornQ;
    if (n == "bornN") return &x.bornN;
    if (n == "bornU") return &x.bornU;
    if (n == "bornX") return &x.bornX;
    return nullptr;
}

enum class Where { Thermo, Solvent, Electro };

/// Where a property comes from, and the name of its field.
auto fieldOf(bool reaction, const std::string& name) -> std::pair<Where, std::string>
{
    const auto& names = reaction ? reactionNames : substanceNames;
    if (const auto it = names.find(name); it != names.end())
        return {Where::Thermo, it->second};
    for (const auto& item : names)
        if (item.second == name)
            return {Where::Thermo, name};
    if (reaction)
        fail("A reaction has no property " + name + ". Use one of: logK, lnK, dG, dH, dS, dCp, dCv, dV, dU, dA.");
    const PropertiesSolvent solvent{};
    if (solventScalar(solvent, name) || solventNumber(solvent, name))
        return {Where::Solvent, name};
    if (electroScalar(ElectroPropertiesSolvent{}, name))
        return {Where::Electro, name};
    fail("A substance has no property " + name + ". Use one of: G, H, S, Cp, Cv, V, U, A, or a property of water such as density or epsilon.");
}

/// J/(mol K) as the converter writes it: J/K/mol.
auto unitOf(std::string text) -> std::string
{
    static const std::regex molK(R"(/\(mol[ .*]K\)$)"), Kmol(R"(/\(K[ .*]mol\)$)");
    return std::regex_replace(std::regex_replace(text, molK, "/K/mol"), Kmol, "/K/mol");
}

auto convert(double value, const std::string& from, const std::string& to) -> double
{
    if (to.empty() || from == to)
        return value;
    if (!units::convertible(unitOf(from), unitOf(to)))
        fail("The unit " + from + " cannot change to " + to + ".");
    return units::convert(value, unitOf(from), unitOf(to));
}

/// The volume of the engine is for one mole, and a unit such as cm3/mol has the mole in it.
auto volumeBase(const std::string& to) -> std::string
{
    return to.find("/mol") != std::string::npos ? "J/bar/mol" : "J/bar";
}

auto shortest(double value) -> std::string
{
    char text[64];
    for (const int digits : {15, 16, 17})
    {
        std::snprintf(text, sizeof text, "%.*g", digits, value);
        if (std::strtod(text, nullptr) == value)
            break;
    }
    return text;
}

auto stringOr(const json& node, const char* key, const std::string& otherwise) -> std::string
{
    return node.contains(key) && node[key].is_string() ? node[key].get<std::string>() : otherwise;
}

auto digitsOf(const json& node, const char* key) -> std::optional<int>
{
    return node.contains(key) && node[key].is_number() ? std::optional<int>(node[key].get<int>()) : std::nullopt;
}

struct Axis
{
    std::vector<double> values;
    std::vector<char> saturation;
    std::string unit;
};

auto axisOf(const json& spec, const char* name, const char* defaultUnit) -> Axis
{
    Axis axis;
    axis.unit = spec.is_object() ? stringOr(spec, "unit", defaultUnit) : defaultUnit;
    const auto add = [&](const json& x) {
        if (x.is_string() && x.get<std::string>() == "saturation")
        {
            axis.values.push_back(0.0);
            axis.saturation.push_back(1);
        }
        else if (x.is_number())
        {
            axis.values.push_back(x.get<double>());
            axis.saturation.push_back(0);
        }
        else
            fail(std::string(name) + " has a value that is not a number: " + x.dump());
    };
    if (spec.is_object() && spec.contains("values"))
    {
        for (const auto& x : spec["values"])
            add(x);
    }
    else if (spec.is_object())
    {
        for (const char* key : {"from", "to", "step"})
            if (!spec.contains(key))
                fail(std::string(name) + " needs values, or from, to and step. It has no " + key + ".");
        const double first = spec["from"].get<double>(), last = spec["to"].get<double>(), step = spec["step"].get<double>();
        const int count = step == 0.0 ? 1 : static_cast<int>(std::floor((last - first) / step + 1e-9)) + 1;
        for (int i = 0; i < count; ++i)
            add(json(first + i * step));
    }
    else if (spec.is_array())
    {
        for (const auto& x : spec)
            add(x);
    }
    else
        add(spec);
    return axis;
}

struct Point
{
    double T, P;
    bool saturation;
};

auto columnOf(const json& spec) -> json
{
    json column = spec.is_string() ? json{{"property", spec}} : spec;
    if (!column.is_object() || !column.contains("property"))
        fail("A column needs a property: " + spec.dump());
    if (!column.contains("show"))
        column["show"] = "val";
    const auto show = column["show"].get<std::string>();
    if (show != "val" && show != "err" && show != "ddt" && show != "ddp" && show != "status")
        fail("show is one of val, err, ddt, ddp, status, not " + show + ".");
    return column;
}

auto labelOf(const json& column, const std::string& field, const std::string& variant) -> std::string
{
    std::string text = stringOr(column, "label", "");
    if (text.empty())
    {
        text = column["property"].get<std::string>();
        const auto show = column["show"].get<std::string>();
        std::string unit = stringOr(column, "unit", "");
        if (unit.empty())
            if (const auto it = baseUnits.find(field); it != baseUnits.end())
                unit = it->second;
        if (show == "err") text += "_err";
        if (show == "ddt") text += "_ddT";
        if (show == "ddp") text += "_ddP";
        if (show == "status") text += "_status";
        if (!unit.empty() && unit != "1" && show != "status")
            text += "(" + unit + ")";
    }
    return variant.empty() ? text : text + " [" + variant + "]";
}

auto cellOf(const Scalar& x, const std::string& field, const json& column) -> TableCell
{
    TableCell cell;
    cell.result = x;
    cell.has_result = true;
    const auto show = column["show"].get<std::string>();
    const std::string unit = stringOr(column, "unit", "");
    std::string base;
    if (const auto it = baseUnits.find(field); it != baseUnits.end())
        base = it->second;
    if (field == "volume" || field == "reaction_volume")
        base = volumeBase(unit);
    cell.defined = x.sta.first != Reaktoro_::Status::notdefined;
    if (show == "status")
    {
        static const std::map<Reaktoro_::Status, std::string> words = {{Reaktoro_::Status::notdefined, "notdefined"}, {Reaktoro_::Status::read, "read"}, {Reaktoro_::Status::assigned, "assigned"},
                                                                       {Reaktoro_::Status::calculated, "calculated"}, {Reaktoro_::Status::initialized, "initialized"}};
        cell.text = words.count(x.sta.first) ? words.at(x.sta.first) : "";
        cell.defined = true;
        return cell;
    }
    if (!unit.empty() && base.empty())
        fail("ThermoFun does not know the unit of " + field + ": leave out the unit.");
    if (show == "val")
        cell.value = base.empty() ? static_cast<double>(x.val) : convert(static_cast<double>(x.val), base, unit);
    else
    {
        // a difference or a derivative: only the factor of the unit
        const double factor = base.empty() || unit.empty() ? 1.0 : convert(1.0, base, unit) - convert(0.0, base, unit);
        cell.value = factor * (show == "err" ? static_cast<double>(x.err) : show == "ddt" ? static_cast<double>(x.ddt) : static_cast<double>(x.ddp));
    }
    return cell;
}

/// Sets an option of the engine by its name.
auto setOption(EnginePreferences& p, const std::string& key, const json& value) -> void
{
    if (key == "enable_memoize") p.enable_memoize = value.get<bool>();
    else if (key == "max_cache_size") p.max_cache_size = value.get<std::size_t>();
    else if (key == "solvent_state" || key == "solventState") p.solvent_state = value.get<int>();
    else if (key == "solvent_symbol" || key == "solventSymbol") p.solvent_symbol = value.get<std::string>();
    else if (key == "fallback_to_reference_properties") p.fallback_to_reference_properties = value.get<bool>();
    else if (key == "apply_pressure_correction_to_gas_props") p.apply_pressure_correction_to_gas_props = value.get<bool>();
    else if (key == "cancel_fugacity_coefficient_in_gas_props") p.cancel_fugacity_coefficient_in_gas_props = value.get<bool>();
    else if (key == "round_to_uncertainty") p.round_to_uncertainty = value.get<bool>();
    else if (key == "uncertainty_significant_digits") p.uncertainty_significant_digits = value.get<int>();
    else if (key == "propagate_parameter_errors") p.propagate_parameter_errors = value.get<bool>();
    else if (key == "method_implementation") p.method_implementation = value.get<std::map<std::string, std::string>>();
    else if (key == "property_source")
    {
        const auto word = v2::fromString<v2::PropertySource>(value.get<std::string>());
        if (!word)
            fail(v2::unknownWordMessage<v2::PropertySource>(value.get<std::string>()));
        p.property_source = *word;
    }
    else
        fail("The engine has no option " + key + ".");
}

template <class E>
auto wordOf(const json& value) -> E
{
    const auto word = v2::fromString<E>(value.get<std::string>());
    if (!word)
        fail(v2::unknownWordMessage<E>(value.get<std::string>()));
    return *word;
}

auto setConvention(EngineConventions& c, const std::string& key, const json& value) -> void
{
    if (key == "energy_reference") c.energy_reference = wordOf<v2::EnergyReference>(value);
    else if (key == "standard_pressure") c.standard_pressure = wordOf<v2::StandardPressure>(value);
    else if (key == "concentration_scale") c.concentration_scale = wordOf<v2::ConcentrationScale>(value);
    else if (key == "water_reference") c.water_reference = wordOf<v2::WaterReference>(value);
    else
        fail("The conventions have no field " + key + ".");
}

auto scalarText(const json& value) -> std::string
{
    static const std::regex plain(R"([A-Za-z_][\w./()+@-]*)");
    if (value.is_string())
    {
        const auto text = value.get<std::string>();
        static const std::set<std::string> words = {"true", "false", "null", "yes", "no", "on", "off"};
        if (std::regex_match(text, plain) && !words.count(text))
            return text;
    }
    return value.dump();
}

auto isSmall(const json& x) -> bool
{
    return std::all_of(x.begin(), x.end(), [](const json& v) { return !v.is_structured(); });
}

auto flowText(const json& x) -> std::string
{
    std::string out = x.is_object() ? "{" : "[";
    bool first = true;
    if (x.is_object())
        for (const auto& item : x.items())
        {
            out += (first ? "" : ", ") + scalarText(item.key()) + ": " + scalarText(item.value());
            first = false;
        }
    else
        for (const auto& v : x)
        {
            out += (first ? "" : ", ") + scalarText(v);
            first = false;
        }
    return out + (x.is_object() ? "}" : "]");
}

auto blockText(const json& value, int indent) -> std::string
{
    const std::string pad(2 * indent, ' ');
    std::string out;
    if (value.is_object())
    {
        for (const auto& item : value.items())
        {
            const auto& v = item.value();
            if (v.is_structured() && !isSmall(v))
                out += pad + scalarText(item.key()) + ":\n" + blockText(v, indent + 1);
            else
                out += pad + scalarText(item.key()) + ": " + (v.is_structured() ? flowText(v) : scalarText(v)) + "\n";
        }
    }
    else
    {
        for (const auto& v : value)
        {
            if (v.is_object() && !isSmall(v))
                out += pad + "- " + blockText(v, indent + 1).substr(pad.size() + 2);
            else
                out += pad + "- " + (v.is_structured() ? flowText(v) : scalarText(v)) + "\n";
        }
    }
    return out;
}

auto parseText(const std::string& text) -> json
{
    const auto start = text.find_first_not_of(" \t\r\n");
    try
    {
        if (start == std::string::npos || text[start] != '{')
            return json::parse(v2::yamlToJson(text));
        if (json::accept(text))
            return json::parse(text);
        // {a: 1} is YAML too, and the reader of YAML starts from a block
        return json::parse(v2::yamlToJson("setup: " + text))["setup"];
    }
    catch (const std::exception& error)
    {
        fail(std::string("The setup cannot be read: ") + error.what());
    }
}

auto readFile(const std::string& path) -> std::string
{
    std::ifstream file(path);
    if (!file)
        fail("Cannot read the file " + path + ".");
    std::stringstream text;
    text << file.rdbuf();
    return text.str();
}

auto folderList() -> std::vector<std::string>
{
    std::vector<std::string> out;
    const char* value = std::getenv("THERMOFUN_TEMPLATE_PATH");
    std::stringstream folders(value ? value : "");
#ifdef _WIN32
    const char separator = ';';
#else
    const char separator = ':';
#endif
    for (std::string folder; std::getline(folders, folder, separator);)
        if (!folder.empty())
            out.push_back(folder);
    return out;
}

/// The files of the template folders: the first folder that has a name wins.
auto userTemplateFiles() -> std::map<std::string, std::filesystem::path>
{
    std::map<std::string, std::filesystem::path> out;
    for (const auto& folder : folderList())
    {
        std::error_code ignored;
        for (const auto& entry : std::filesystem::directory_iterator(folder, ignored))
            if (const auto extension = entry.path().extension(); extension == ".yaml" || extension == ".yml")
                out.emplace(entry.path().stem().string(), entry.path());
    }
    return out;
}

auto templateFile(const std::string& name) -> const v2::detail::BuiltinFile*
{
    for (unsigned long i = 0; i < v2::detail::builtin_table_template_count; ++i)
        if (std::filesystem::path(v2::detail::builtin_table_templates[i].name).stem().string() == name)
            return &v2::detail::builtin_table_templates[i];
    return nullptr;
}

auto textOf(const v2::detail::BuiltinFile& file) -> std::string
{
    return std::string(reinterpret_cast<const char*>(file.text), file.size);
}

auto describedBy(const std::string& text) -> std::string
{
    const std::string first = text.substr(0, text.find('\n'));
    return first.rfind("# ", 0) == 0 ? first.substr(2) : first;
}

/// Puts `over` on `base`: its settings replace those of the setup and of its tables, and a table of the same name changes that table.
auto mergeOver(json& base, const json& over) -> void
{
    const auto eachColumn = [&](auto&& f) {
        if (base.contains("tables"))
            for (auto& table : base["tables"])
                if (table.contains("columns"))
                    for (auto& column : table["columns"])
                        if (column.is_object())
                            f(column);
    };
    for (const auto& item : over.items())
    {
        const auto& key = item.key();
        if (key == "tables")
            continue;
        if (key == "from_templates")
        {
            if (!base.contains(key))
                base[key] = json::array();
            for (const auto& name : item.value())
                base[key].push_back(name);
            continue;
        }
        if (key == "conditions")
        {
            if (!base.contains(key))
                base[key] = json::object();
            base[key].update(item.value());
            if (base.contains("tables"))
                for (auto& table : base["tables"])
                    if (table.contains(key))
                        table[key].update(item.value());
            continue;
        }
        base[key] = item.value();
        if (base.contains("tables"))
            for (auto& table : base["tables"])
                table.erase(key);
        // a unit or digits for a property replace those that a column of a template has
        if (key == "units" && item.value().is_object())
            eachColumn([&](json& column) {
                if (item.value().contains(column.value("property", "")))
                    column.erase("unit");
            });
        if (key == "digits")
            eachColumn([&](json& column) {
                if (item.value().is_number() || item.value().contains(column.value("property", "")))
                    column.erase("digits");
            });
    }
    if (!over.contains("tables"))
        return;
    if (!base.contains("tables"))
        base["tables"] = json::array();
    for (const auto& table : over["tables"])
    {
        const std::string name = stringOr(table, "name", "");
        const auto same = std::find_if(base["tables"].begin(), base["tables"].end(), [&](const json& t) { return !name.empty() && stringOr(t, "name", "") == name; });
        if (same == base["tables"].end())
            base["tables"].push_back(table);
        else
            same->update(table);
    }
}

/// Replaces the key `template` by what the templates say, under what the document says.
auto expandTemplates(json document, std::vector<std::string> chain = {}) -> json
{
    if (!document.contains("template"))
        return document;
    const json node = document["template"];
    document.erase("template");
    json result = json::object();
    result["from_templates"] = json::array();
    for (const auto& item : node.is_array() ? node : json::array({node}))
    {
        if (!item.is_string())
            fail("template has a value that is not a name: " + item.dump() + ".");
        const auto name = item.get<std::string>();
        if (std::find(chain.begin(), chain.end(), name) != chain.end())
            fail("The template " + name + " uses itself.");
        auto next = chain;
        next.push_back(name);
        mergeOver(result, expandTemplates(parseText(TableSetup::templateText(name)), next));
        result["from_templates"].push_back(name);
    }
    mergeOver(result, document);
    return result;
}

const std::vector<std::string> tableKeys = {"database", "conditions", "tables", "template", "from_templates", "name", "description", "records", "reactions", "columns", "layout", "across", "loop",
                                            "format", "digits", "digits_T", "digits_P", "units", "separator", "not_defined", "write_not_defined", "output", "format_file", "provenance", "options",
                                            "conventions", "solvent", "source", "sources", "compare"};
const std::vector<std::string> conditionKeys = {"T", "P", "pairs", "order"};
const std::vector<std::string> columnKeys = {"property", "show", "unit", "digits", "format", "label"};
const std::vector<std::string> compareKeys = {"label", "replace", "options", "conventions", "database"};

auto checkKeys(const json& node, const std::vector<std::string>& known, const std::string& where) -> void
{
    if (!node.is_object())
        return;
    for (const auto& item : node.items())
        if (std::find(known.begin(), known.end(), item.key()) == known.end())
            fail(detail::unknownKeyMessage(where, item.key(), known));
}

auto checkTable(const json& spec, const std::string& name) -> void
{
    const std::string where = "The table " + name;
    checkKeys(spec, tableKeys, where);
    if (spec.contains("conditions"))
        checkKeys(spec["conditions"], conditionKeys, "The conditions of the table " + name);
    if (spec.contains("columns"))
        for (const auto& column : spec["columns"])
            checkKeys(column, columnKeys, "A column of the table " + name);
    if (spec.contains("compare"))
        for (const auto& variant : spec["compare"])
            checkKeys(variant, compareKeys, "A comparison of the table " + name);
    const std::string mode = stringOr(spec, "provenance", "header");
    if (mode != "header" && mode != "file" && mode != "none")
        fail("provenance is header, file or none, not " + mode + ".");
}

/// The symbols of `records` or `reactions`: a list, a rule, or a list with rules in it.
auto symbolsOf(const json& node, const Database& database, bool reaction) -> std::vector<std::string>
{
    std::vector<std::string> out;
    const auto add = [&](const std::string& symbol) {
        if (std::find(out.begin(), out.end(), symbol) == out.end())
            out.push_back(symbol);
    };
    for (const auto& item : node.is_array() ? node : json::array({node}))
    {
        if (item.is_object())
            for (const auto& symbol : reaction ? selectReactions(database, item.dump()) : selectSubstances(database, item.dump()))
                add(symbol);
        else if (item.is_string())
            add(item.get<std::string>());
        else
            fail(std::string(reaction ? "reactions" : "records") + " has a value that is not a symbol or a rule: " + item.dump() + ".");
    }
    return out;
}

/// The points of the conditions, in the order of the setup.
auto pointsOf(const json& conditions, const Axis& axisT, const Axis& axisP) -> std::vector<Point>
{
    std::vector<Point> points;
    if (conditions.contains("pairs") && conditions["pairs"].get<bool>())
    {
        if (axisT.values.size() != axisP.values.size())
            fail("pairs: T has " + std::to_string(axisT.values.size()) + " values and P has " + std::to_string(axisP.values.size()) + ".");
        for (std::size_t i = 0; i < axisT.values.size(); ++i)
            points.push_back({axisT.values[i], axisP.values[i], static_cast<bool>(axisP.saturation[i])});
    }
    else if (stringOr(conditions, "order", "T_first") == "P_first")
    {
        for (std::size_t p = 0; p < axisP.values.size(); ++p)
            for (const double t : axisT.values)
                points.push_back({t, axisP.values[p], static_cast<bool>(axisP.saturation[p])});
    }
    else
    {
        for (const double t : axisT.values)
            for (std::size_t p = 0; p < axisP.values.size(); ++p)
                points.push_back({t, axisP.values[p], static_cast<bool>(axisP.saturation[p])});
    }
    return points;
}

/// A column with the units and the digits that the table gives for its property.
auto withTableDefaults(json column, const json& spec, const std::string& field) -> json
{
    const auto lookup = [&](const char* key) -> const json* {
        if (!spec.contains(key) || !spec[key].is_object())
            return nullptr;
        const auto property = column["property"].get<std::string>();
        return spec[key].contains(property) ? &spec[key][property] : spec[key].contains(field) ? &spec[key][field] : nullptr;
    };
    if (!column.contains("unit"))
        if (const auto* unit = lookup("units"))
            column["unit"] = *unit;
    if (!column.contains("digits"))
        if (const auto* digits = lookup("digits"))
            column["digits"] = *digits;
    return column;
}

/// A copy of a database with other methods in some records: {record: {old method: new method or a method record}}.
auto withReplaced(const Database& database, const json& changes) -> Database
{
    json document = json::parse(database.toJson(false));
    for (const auto& record : changes.items())
    {
        if (!document.contains("substances") || !document["substances"].contains(record.key()))
            fail("replace: the database has no substance " + record.key() + ".");
        auto& methods = document["substances"][record.key()]["methods"];
        for (const auto& change : record.value().items())
        {
            const auto at = std::find_if(methods.begin(), methods.end(), [&](const json& m) { return m["method"] == change.key(); });
            if (at == methods.end())
                fail("replace: " + record.key() + " has no method " + change.key() + ".");
            *at = change.value().is_string() ? json{{"method", change.value()}} : change.value();
        }
    }
    return Database(document.dump());
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------------

auto Table::values() const -> std::vector<std::vector<double>>
{
    std::vector<std::vector<double>> out;
    for (const auto& row : rows_)
    {
        std::vector<double> line;
        for (const auto& cell : row.cells)
            line.push_back(cell.defined ? cell.value : std::nan(""));
        out.push_back(std::move(line));
    }
    return out;
}

auto Table::cell(const std::string& record, double T, double P, const std::string& column, bool at_saturation) const -> const TableCell&
{
    std::size_t index = columns_.size();
    for (std::size_t i = 0; i < columns_.size() && index == columns_.size(); ++i)
        if (columns_[i].label == column || columns_[i].property == column)
            index = i;
    if (index == columns_.size())
        fail("The table " + name_ + " has no column " + column + ".");
    for (const auto& row : rows_)
        if (row.record == record && row.T == T && row.saturation == at_saturation && (at_saturation || row.P == P))
            return row.cells[index];
    fail("The table " + name_ + " has no row for " + record + " at this T and P.");
}

auto Table::format(const TableColumn& column, double value) const -> std::string
{
    const auto digits = column.digits ? column.digits : digits_;
    const std::string style = !column.format.empty() ? column.format : !number_format_.empty() ? number_format_ : digits ? "fixed" : "floating";
    char text[64];
    if (style == "scientific")
        std::snprintf(text, sizeof text, "%.*e", digits.value_or(6), value);
    else if (style == "fixed" && digits)
        std::snprintf(text, sizeof text, "%.*f", *digits, value);
    else if (digits)
        std::snprintf(text, sizeof text, "%.*g", *digits, value);
    else
        return shortest(value);
    return text;
}

auto Table::number(const TableColumn& column, const TableCell* cell) const -> std::string
{
    if (!cell)
        return not_defined_;
    if (!cell->text.empty())
        return cell->text;
    if (!cell->defined && write_not_defined_)
        return not_defined_;
    return format(column, cell->value);
}

auto Table::condition(double value, bool saturation, const std::optional<int>& digits) const -> std::string
{
    if (saturation)
        return "saturation";
    char text[64];
    if (digits)
        std::snprintf(text, sizeof text, "%.*f", *digits, value);
    else
        std::snprintf(text, sizeof text, "%g", value);
    return text;
}

auto Table::lines() const -> std::vector<std::vector<std::string>>
{
    std::vector<std::vector<std::string>> out;
    const std::string labelT = "T(" + unit_T_ + ")", labelP = "P(" + unit_P_ + ")";
    if (layout_ == "long")
    {
        std::vector<std::string> header = {"Symbol", labelT, labelP};
        for (const auto& c : columns_)
            header.push_back(c.label);
        out.push_back(header);
        auto ordered = rows_;
        if (loop_ == "conditions")
        {
            std::vector<std::pair<double, double>> points;
            for (const auto& r : rows_)
                if (std::find(points.begin(), points.end(), std::make_pair(r.T, r.P)) == points.end())
                    points.emplace_back(r.T, r.P);
            std::stable_sort(ordered.begin(), ordered.end(), [&](const TableRow& a, const TableRow& b) {
                return std::find(points.begin(), points.end(), std::make_pair(a.T, a.P)) < std::find(points.begin(), points.end(), std::make_pair(b.T, b.P));
            });
        }
        for (const auto& row : ordered)
        {
            std::vector<std::string> line = {row.record, condition(row.T, false, digits_T_), condition(row.P, row.saturation, digits_P_)};
            for (std::size_t i = 0; i < columns_.size(); ++i)
                line.push_back(number(columns_[i], i < row.cells.size() ? &row.cells[i] : nullptr));
            out.push_back(std::move(line));
        }
        return out;
    }
    if (layout_ != "transposed")
        fail("The layout is long, transposed or grid, not " + layout_ + ".");
    std::vector<std::string> records;
    std::vector<const TableRow*> points;
    for (const auto& row : rows_)
    {
        if (std::find(records.begin(), records.end(), row.record) == records.end())
            records.push_back(row.record);
        if (std::none_of(points.begin(), points.end(), [&](const TableRow* p) { return p->T == row.T && p->P == row.P && p->saturation == row.saturation; }))
            points.push_back(&row);
    }
    std::vector<std::string> header = {"Property", labelT, labelP};
    header.insert(header.end(), records.begin(), records.end());
    out.push_back(header);
    for (std::size_t i = 0; i < columns_.size(); ++i)
        for (const auto* point : points)
        {
            std::vector<std::string> line = {columns_[i].label, condition(point->T, false, digits_T_), condition(point->P, point->saturation, digits_P_)};
            for (const auto& record : records)
            {
                const auto row = std::find_if(rows_.begin(), rows_.end(), [&](const TableRow& r) { return r.record == record && r.T == point->T && r.P == point->P && r.saturation == point->saturation; });
                line.push_back(row == rows_.end() ? "" : number(columns_[i], &row->cells[i]));
            }
            out.push_back(std::move(line));
        }
    return out;
}

auto Table::propertyGrids() const -> std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>
{
    const bool acrossT = across_ == "T";
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> out;
    const std::string corner = acrossT ? "P(" + unit_P_ + ")_T(" + unit_T_ + ")" : "T(" + unit_T_ + ")_P(" + unit_P_ + ")";
    std::vector<std::string> records;
    for (const auto& row : rows_)
        if (std::find(records.begin(), records.end(), row.record) == records.end())
            records.push_back(row.record);
    const auto textT = [&](std::size_t i) { return condition(Ts_[i], false, std::nullopt); };
    const auto textP = [&](std::size_t i) { return condition(Ps_[i], Ps_saturation_[i], std::nullopt); };
    for (std::size_t c = 0; c < columns_.size(); ++c)
    {
        std::vector<std::vector<std::string>> lines_of = {{"Symbol", corner}};
        for (std::size_t i = 0; i < (acrossT ? Ts_.size() : Ps_.size()); ++i)
            lines_of[0].push_back(acrossT ? textT(i) : textP(i));
        for (const auto& record : records)
            for (std::size_t d = 0; d < (acrossT ? Ps_.size() : Ts_.size()); ++d)
            {
                std::vector<std::string> line = {record, acrossT ? textP(d) : textT(d)};
                for (std::size_t a = 0; a < (acrossT ? Ts_.size() : Ps_.size()); ++a)
                {
                    const std::size_t iT = acrossT ? a : d, iP = acrossT ? d : a;
                    const auto row = std::find_if(rows_.begin(), rows_.end(), [&](const TableRow& r) { return r.record == record && r.T == Ts_[iT] && r.P == Ps_[iP] && r.saturation == static_cast<bool>(Ps_saturation_[iP]); });
                    line.push_back(row == rows_.end() ? "" : number(columns_[c], &row->cells[c]));
                }
                lines_of.push_back(std::move(line));
            }
        out.emplace_back(columns_[c].label, std::move(lines_of));
    }
    return out;
}

namespace {

auto replaced(std::string text, const std::string& from, const std::string& to) -> std::string
{
    for (std::size_t at = text.find(from); at != std::string::npos; at = text.find(from, at + to.size()))
        text.replace(at, from.size(), to);
    return text;
}

auto csvText(const std::vector<std::vector<std::string>>& lines, const std::string& separator) -> std::string
{
    std::string out;
    for (const auto& line : lines)
    {
        for (std::size_t i = 0; i < line.size(); ++i)
            out += (i ? separator : "") + replaced(line[i], separator, "_");
        out += "\n";
    }
    return out;
}

auto markdownText(const std::vector<std::vector<std::string>>& lines) -> std::string
{
    std::string out;
    for (std::size_t n = 0; n < lines.size(); ++n)
    {
        out += "|";
        for (const auto& x : lines[n])
            out += " " + replaced(x, "|", "\\|") + " |";
        out += "\n";
        if (n == 0)
        {
            out += "|";
            for (std::size_t i = 0; i < lines[0].size(); ++i)
                out += "---|";
            out += "\n";
        }
    }
    return out;
}

auto latexText(const std::vector<std::vector<std::string>>& lines) -> std::string
{
    static const std::regex special(R"(([%&_#$]))");
    std::string out = "\\begin{tabular}{l" + std::string(lines[0].size() - 1, 'r') + "}\n\\hline\n";
    for (std::size_t n = 0; n < lines.size(); ++n)
    {
        for (std::size_t i = 0; i < lines[n].size(); ++i)
            out += (i ? " & " : "") + std::regex_replace(lines[n][i], special, "\\$1");
        out += " \\\\\n";
        if (n == 0)
            out += "\\hline\n";
    }
    return out + "\\hline\n\\end{tabular}\n";
}

} // namespace

auto TableProvenance::lines() const -> std::vector<std::string>
{
    std::vector<std::string> out;
    if (!database.empty())
        out.push_back("database: " + database);
    if (!templates.empty())
    {
        std::string text = "templates:";
        for (std::size_t i = 0; i < templates.size(); ++i)
            text += (i ? ", " : " ") + templates[i];
        out.push_back(text);
    }
    for (const auto& [label, snapshot] : calculations)
        for (const auto& line : snapshotLines(snapshot))
            out.push_back((label.empty() ? "" : "[" + label + "] ") + line);
    return out;
}

auto TableProvenance::toYaml() const -> std::string
{
    json document = json::object();
    document["database"] = database;
    document["templates"] = templates;
    document["calculations"] = json::array();
    for (const auto& [label, s] : calculations)
    {
        json item = {{"label", label}, {"thermofun", s.thermofun_version}, {"physical_constants", s.physical_constants}, {"format", s.format_version},
                     {"dataset", s.dataset}, {"dataset_version", s.dataset_version}, {"sha256", s.content_hash}, {"water", s.water_symbol}, {"water_methods", s.water_methods},
                     {"conventions", s.conventions}, {"options", s.options}, {"implementations", s.implementations}};
        document["calculations"].push_back(std::move(item));
    }
    std::string text = blockText(document, 0) + "setup:\n";
    std::stringstream lines_of(setup);
    for (std::string line; std::getline(lines_of, line);)
        text += "  " + line + "\n";
    return text;
}

namespace {

auto commented(const std::vector<std::string>& lines, const std::string& kind) -> std::string
{
    std::string out = kind == "md" ? "<!--\n" : "";
    for (const auto& line : lines)
        out += (kind == "csv" ? "# " : kind == "tex" ? "% " : "") + line + "\n";
    return kind == "md" ? out + "-->\n\n" : out;
}

auto writeProvenanceFile(const TableProvenance& provenance, const std::string& where) -> void
{
    std::ofstream file(where + ".provenance.yaml");
    if (!file)
        fail("Cannot write the file " + where + ".provenance.yaml.");
    file << provenance.toYaml();
}

} // namespace

auto Table::toText(const std::string& kind, bool with_provenance) const -> std::string
{
    if (kind != "csv" && kind != "md" && kind != "tex")
        fail("The format of an output is csv, md or tex, not " + kind + ".");
    if (with_provenance)
        return commented(provenance_.lines(), kind) + toText(kind, false);
    const auto text = [&](const std::vector<std::vector<std::string>>& lines) {
        return kind == "csv" ? csvText(lines, separator_) : kind == "md" ? markdownText(lines) : latexText(lines);
    };
    if (layout_ != "grid")
        return text(lines());
    std::string out;
    for (const auto& [label, grid] : propertyGrids())
        out += (kind == "md" ? "**" + label + "**\n\n" : kind == "tex" ? "% " + label + "\n" : label + "\n") + text(grid) + "\n";
    return out;
}

auto Table::write(const std::string& path) const -> std::string
{
    const std::string where = path.empty() ? output_ : path;
    if (where.empty())
        fail("The table " + name_ + " has no output.");
    std::string kind = file_format_;
    if (kind.empty())
    {
        kind = std::filesystem::path(where).extension().string();
        if (!kind.empty())
            kind.erase(0, 1);
        if (kind == "markdown") kind = "md";
        if (kind == "txt" || kind == "dat" || kind.empty()) kind = "csv";
    }
    const auto text = toText(kind, provenance_mode_ == "header");
    std::ofstream file(where);
    if (!file)
        fail("Cannot write the file " + where + ".");
    file << text;
    if (provenance_mode_ == "file")
        writeProvenanceFile(provenance_, where);
    return where;
}

auto TableResults::at(const std::string& name) const -> const Table&
{
    for (const auto& table : tables_)
        if (table.name() == name)
            return table;
    fail("There is no table " + name + ".");
}

auto TableResults::write() const -> std::vector<std::string>
{
    std::vector<std::string> paths;
    for (const auto& table : tables_)
        if (!table.output().empty())
            paths.push_back(table.write());
    return paths;
}

auto TableResults::problems() const -> std::vector<TableProblem>
{
    std::vector<TableProblem> out;
    for (const auto& table : tables_)
        out.insert(out.end(), table.problems().begin(), table.problems().end());
    return out;
}

// ---------------------------------------------------------------------------------------------------------------------

struct TableSetup::Impl
{
    json document = json::object();
    std::string base;
    std::optional<Database> database;

    template <class F>
    auto forTables(const std::string& name, F&& f) -> void
    {
        if (!document.contains("tables") || !document["tables"].is_array())
            fail("The setup has no tables.");
        bool found = false;
        for (auto& table : document["tables"])
            if (name.empty() || stringOr(table, "name", "") == name)
            {
                f(table);
                found = true;
            }
        if (!found)
            fail("The setup has no table " + name + ".");
    }

    auto setCondition(const std::string& table, const char* key, const json& value) -> void
    {
        forTables(table, [&](json& t) {
            if (!t.contains("conditions"))
                t["conditions"] = document.contains("conditions") ? document["conditions"] : json::object();
            t["conditions"][key] = value;
        });
    }

    auto sourceDatabase() const -> Database
    {
        if (database)
            return *database;
        if (!document.contains("database") || !document["database"].is_string())
            fail("The setup has no database.");
        return databaseAt(document["database"].get<std::string>());
    }

    auto databaseAt(const std::string& path) const -> Database
    {
        const std::filesystem::path p(path);
        return Database(p.is_absolute() || base.empty() ? path : (std::filesystem::path(base) / p).string());
    }

    auto engineOf(const Database& source, const json& spec, const json& variant) const -> ThermoEngine
    {
        Database database_ = variant.contains("database") ? databaseAt(variant["database"].get<std::string>()) : source;
        if (variant.contains("replace"))
            database_ = withReplaced(database_, variant["replace"]);
        ThermoEngine engine(database_);
        auto preferences = engine.preferences();
        for (const auto* scope : {&spec, &variant})
            if (scope->contains("options"))
                for (const auto& item : (*scope)["options"].items())
                    setOption(preferences, item.key(), item.value());
        engine.setPreferences(preferences);
        if (spec.contains("conventions") || variant.contains("conventions"))
        {
            EngineConventions conventions;
            for (const auto* scope : {&spec, &variant})
                if (scope->contains("conventions"))
                    for (const auto& item : (*scope)["conventions"].items())
                        setConvention(conventions, item.key(), item.value());
            engine.setConventions(conventions);
        }
        if (spec.contains("solvent"))
            engine.setSolventSymbol(spec["solvent"].get<std::string>());
        if (spec.contains("sources"))
            for (const auto& item : spec["sources"].items())
                engine.setPropertySource(item.key(), item.value().get<std::string>());
        return engine;
    }

    /// The settings of a table: those of the top of the setup, with those of the table over them.
    auto mergedSpec(const json& table) const -> json
    {
        json merged = json::object();
        for (const auto& item : document.items())
            if (item.key() != "tables" && item.key() != "database")
                merged[item.key()] = item.value();
        for (const auto& item : table.items())
            merged[item.key()] = item.value();
        return merged;
    }

    /// The settings of the table with this name. Without a name, the setup must have one table.
    auto oneTable(const std::string& table) const -> json
    {
        if (!document.contains("tables") || !document["tables"].is_array() || document["tables"].empty())
            fail("The setup has no tables.");
        std::string names;
        for (const auto& t : document["tables"])
        {
            if (table.empty() ? document["tables"].size() == 1 : stringOr(t, "name", "") == table)
                return mergedSpec(t);
            names += (names.empty() ? "" : ", ") + stringOr(t, "name", "");
        }
        fail("Name one table of the setup. It has: " + names + ".");
    }

    /// The part of the provenance that is the same for a table and for a batch.
    auto provenanceOf(const json& spec) const -> TableProvenance
    {
        TableProvenance p;
        p.database = document.contains("database") && document["database"].is_string() ? document["database"].get<std::string>() : "given in memory";
        if (document.contains("from_templates"))
            for (const auto& name : document["from_templates"])
            {
                const auto where = TableSetup::templateSource(name.get<std::string>());
                p.templates.push_back(name.get<std::string>() + (where == "library" ? "" : " (" + where + ")"));
            }
        json settings = spec;
        settings.erase("from_templates");
        p.setup = blockText(settings, 0);
        return p;
    }

    auto runTable(const std::string& name, const json& spec, const Database& source) const -> Table
    {
        Table table;
        table.name_ = name;
        table.output_ = stringOr(spec, "output", "");
        table.layout_ = stringOr(spec, "layout", "long");
        table.separator_ = stringOr(spec, "separator", ",");
        table.not_defined_ = stringOr(spec, "not_defined", "NaN");
        table.number_format_ = stringOr(spec, "format", "");
        table.file_format_ = stringOr(spec, "format_file", "");
        table.across_ = stringOr(spec, "across", "P");
        table.loop_ = stringOr(spec, "loop", "records");
        table.provenance_mode_ = stringOr(spec, "provenance", "header");
        table.provenance_ = provenanceOf(spec);
        table.digits_ = digitsOf(spec, "digits");
        table.digits_T_ = digitsOf(spec, "digits_T");
        table.digits_P_ = digitsOf(spec, "digits_P");
        if (spec.contains("write_not_defined"))
            table.write_not_defined_ = spec["write_not_defined"].get<bool>();

        std::vector<std::pair<bool, std::string>> kinds;
        if (spec.contains("records"))
            for (const auto& r : symbolsOf(spec["records"], source, false))
                kinds.emplace_back(false, r);
        if (spec.contains("reactions"))
            for (const auto& r : symbolsOf(spec["reactions"], source, true))
                kinds.emplace_back(true, r);
        if (kinds.empty())
            fail("The table " + name + " has no records or reactions: give records or reactions.");
        if (!spec.contains("conditions") || !spec["conditions"].contains("T") || !spec["conditions"].contains("P"))
            fail("The conditions of the table " + name + " need T and P.");
        const auto& conditions = spec["conditions"];
        const Axis axisT = axisOf(conditions["T"], "T", "K"), axisP = axisOf(conditions["P"], "P", "Pa");
        table.unit_T_ = axisT.unit;
        table.unit_P_ = axisP.unit;
        table.Ts_ = axisT.values;
        table.Ps_ = axisP.values;
        table.Ps_saturation_ = axisP.saturation;
        const std::vector<Point> points = pointsOf(conditions, axisT, axisP);

        std::vector<json> columns;
        if (spec.contains("columns"))
            for (const auto& c : spec["columns"])
            {
                json column = columnOf(c);
                columns.push_back(withTableDefaults(column, spec, fieldOf(kinds.front().first, column["property"].get<std::string>()).second));
            }
        if (columns.empty())
            fail("The table " + name + " has no columns.");

        std::vector<json> variants;
        if (spec.contains("compare"))
            for (const auto& v : spec["compare"])
                variants.push_back(v);
        if (variants.empty())
            variants.push_back(json::object());
        std::vector<ThermoEngine> engines;
        for (const auto& v : variants)
        {
            engines.push_back(engineOf(source, spec, v));
            table.provenance_.calculations.emplace_back(stringOr(v, "label", ""), engines.back().snapshot());
        }
        for (const auto& v : variants)
            for (const auto& column : columns)
            {
                TableColumn c;
                c.property = column["property"].get<std::string>();
                c.show = column["show"].get<std::string>();
                c.unit = stringOr(column, "unit", "");
                c.format = stringOr(column, "format", "");
                c.digits = digitsOf(column, "digits");
                c.label = labelOf(column, fieldOf(kinds.front().first, c.property).second, stringOr(v, "label", ""));
                table.columns_.push_back(std::move(c));
            }

        const std::string sourceWord = stringOr(spec, "source", "");
        for (const auto& [reaction, record] : kinds)
        {
            const auto& first = engines.front().database();
            const bool known = reaction ? first.containsReaction(record) || record.find('=') != std::string::npos : first.containsSubstance(record);
            if (!known)
            {
                TableProblem problem;
                problem.table = name;
                problem.record = record;
                problem.message = std::string("The database has no ") + (reaction ? "reaction " : "substance ") + record + ".";
                table.problems_.push_back(std::move(problem));
                continue;
            }
            for (const auto& point : points)
            {
                const double T = convert(point.T, axisT.unit, "K");
                const double P = point.saturation ? 0.0 : convert(point.P, axisP.unit, "Pa");
                TableRow row;
                row.record = record;
                row.T = point.T;
                row.P = point.P;
                row.saturation = point.saturation;
                for (auto& engine : engines)
                    for (const auto& column : columns)
                    {
                        const auto [where, field] = fieldOf(reaction, column["property"].get<std::string>());
                        double Pcall = P;
                        try
                        {
                            if (where == Where::Solvent)
                            {
                                const auto x = engine.propertiesSolvent(T, Pcall, record);
                                if (const auto* s = solventScalar(x, field))
                                    row.cells.push_back(cellOf(*s, field, column));
                                else
                                {
                                    TableCell cell;
                                    cell.value = convert(*solventNumber(x, field), baseUnits.count(field) ? baseUnits.at(field) : "", stringOr(column, "unit", ""));
                                    cell.defined = true;
                                    row.cells.push_back(cell);
                                }
                            }
                            else if (where == Where::Electro)
                                row.cells.push_back(cellOf(*electroScalar(engine.electroPropertiesSolvent(T, Pcall, record), field), field, column));
                            else if (reaction)
                            {
                                const auto x = sourceWord == "reactants" ? engine.thermoPropertiesReactionFromReactants(T, Pcall, record) : engine.thermoPropertiesReaction(T, Pcall, record);
                                row.cells.push_back(cellOf(*reactionScalar(x, field), field, column));
                            }
                            else
                            {
                                if (sourceWord == "reaction")
                                    engine.setPropertySource(record, "reaction");
                                row.cells.push_back(cellOf(*substanceScalar(engine.thermoPropertiesSubstance(T, Pcall, record), field), field, column));
                            }
                        }
                        catch (const TableSetupError&)
                        {
                            throw;
                        }
                        catch (const std::exception& error)
                        {
                            TableProblem problem;
                            problem.table = name;
                            problem.record = record;
                            problem.T = point.T;
                            problem.P = point.P;
                            problem.saturation = point.saturation;
                            problem.has_point = true;
                            problem.message = std::regex_replace(std::string(error.what()), std::regex(R"(\s*\*+\s*)"), " ");
                            table.problems_.push_back(std::move(problem));
                            row.cells.emplace_back();
                        }
                    }
                table.rows_.push_back(std::move(row));
            }
        }
        return table;
    }
};

TableSetup::TableSetup() : pimpl(new Impl()) {}
TableSetup::TableSetup(const TableSetup& other) : pimpl(new Impl(*other.pimpl)) {}
auto TableSetup::operator=(TableSetup other) -> TableSetup&
{
    pimpl = std::move(other.pimpl);
    return *this;
}
TableSetup::~TableSetup() = default;

auto TableSetup::fromText(const std::string& text) -> TableSetup
{
    TableSetup setup;
    json document = parseText(text);
    if (!document.is_object())
        fail("The setup is not a mapping.");
    setup.pimpl->document = expandTemplates(std::move(document));
    return setup;
}

auto TableSetup::fromFile(const std::string& path) -> TableSetup
{
    auto setup = fromText(readFile(path));
    setup.pimpl->base = std::filesystem::absolute(path).parent_path().string();
    return setup;
}

auto TableSetup::templates() -> std::map<std::string, std::string>
{
    std::map<std::string, std::string> out;
    for (unsigned long i = 0; i < v2::detail::builtin_table_template_count; ++i)
    {
        const auto& file = v2::detail::builtin_table_templates[i];
        out[std::filesystem::path(file.name).stem().string()] = describedBy(textOf(file));
    }
    for (const auto& item : userTemplateFiles())
        out[item.first] = describedBy(readFile(item.second.string()));
    return out;
}

auto TableSetup::templateFolders() -> std::vector<std::string>
{
    return folderList();
}

auto TableSetup::templateSource(const std::string& name) -> std::string
{
    const auto files = userTemplateFiles();
    if (const auto it = files.find(name); it != files.end())
        return it->second.string();
    return templateFile(name) ? "library" : "";
}

auto TableSetup::templateText(const std::string& name) -> std::string
{
    const auto files = userTemplateFiles();
    if (const auto it = files.find(name); it != files.end())
        return readFile(it->second.string());
    const auto* file = templateFile(name);
    if (!file)
    {
        std::string names;
        for (const auto& item : templates())
            names += (names.empty() ? "" : ", ") + item.first;
        fail("There is no template " + name + ". Use one of: " + names + ".");
    }
    return textOf(*file);
}

auto TableSetup::fromTemplate(const std::string& name) -> TableSetup
{
    return fromText(json{{"template", name}}.dump());
}

auto TableSetup::useTemplate(const std::string& name) -> TableSetup&
{
    mergeOver(pimpl->document, expandTemplates(json{{"template", name}}));
    return *this;
}

auto TableSetup::setDatabase(const Database& database) -> TableSetup&
{
    pimpl->database = database;
    pimpl->document.erase("database");
    return *this;
}

auto TableSetup::setDatabase(const std::string& path) -> TableSetup&
{
    pimpl->database.reset();
    pimpl->document["database"] = path;
    return *this;
}

auto TableSetup::setRecords(const std::vector<std::string>& symbols, const std::string& table) -> TableSetup&
{
    pimpl->forTables(table, [&](json& t) { t["records"] = symbols; });
    return *this;
}

auto TableSetup::setReactions(const std::vector<std::string>& symbols, const std::string& table) -> TableSetup&
{
    pimpl->forTables(table, [&](json& t) { t["reactions"] = symbols; });
    return *this;
}

auto TableSetup::setRecordRule(const std::string& rule, const std::string& table) -> TableSetup&
{
    const json parsed = parseText(rule);
    pimpl->forTables(table, [&](json& t) { t["records"] = parsed; });
    return *this;
}

auto TableSetup::setReactionRule(const std::string& rule, const std::string& table) -> TableSetup&
{
    const json parsed = parseText(rule);
    pimpl->forTables(table, [&](json& t) { t["reactions"] = parsed; });
    return *this;
}

auto TableSetup::setTemperatures(const std::vector<double>& values, const std::string& unit, const std::string& table) -> TableSetup&
{
    pimpl->setCondition(table, "T", json{{"values", values}, {"unit", unit}});
    return *this;
}

auto TableSetup::setTemperatures(double first, double last, double step, const std::string& unit, const std::string& table) -> TableSetup&
{
    pimpl->setCondition(table, "T", json{{"from", first}, {"to", last}, {"step", step}, {"unit", unit}});
    return *this;
}

auto TableSetup::setPressures(const std::vector<double>& values, const std::string& unit, const std::string& table) -> TableSetup&
{
    pimpl->setCondition(table, "P", json{{"values", values}, {"unit", unit}});
    return *this;
}

auto TableSetup::setPressures(double first, double last, double step, const std::string& unit, const std::string& table) -> TableSetup&
{
    pimpl->setCondition(table, "P", json{{"from", first}, {"to", last}, {"step", step}, {"unit", unit}});
    return *this;
}

auto TableSetup::setSaturationPressure(const std::string& table) -> TableSetup&
{
    pimpl->setCondition(table, "P", json{{"values", json::array({"saturation"})}, {"unit", "bar"}});
    return *this;
}

auto TableSetup::setLayout(const std::string& layout, const std::string& table) -> TableSetup&
{
    pimpl->forTables(table, [&](json& t) { t["layout"] = layout; });
    return *this;
}

auto TableSetup::setOutput(const std::string& path, const std::string& table) -> TableSetup&
{
    pimpl->forTables(table, [&](json& t) { t["output"] = path; });
    return *this;
}

auto TableSetup::changeColumn(const std::string& property, const std::string& text, const std::string& table) -> TableSetup&
{
    const json changes = json::parse(text);
    bool found = false;
    pimpl->forTables(table, [&](json& t) {
        if (!t.contains("columns"))
            return;
        for (auto& spec : t["columns"])
        {
            json column = columnOf(spec);
            if (column["property"] == property)
            {
                column.update(changes);
                spec = column;
                found = true;
            }
        }
    });
    if (!found)
        fail("No table has a column " + property + ". Add it with addColumn.");
    return *this;
}

auto TableSetup::setColumn(const std::string& property, const std::string& unit, int digits, const std::string& table) -> TableSetup&
{
    json changes = json::object();
    if (!unit.empty())
        changes["unit"] = unit;
    if (digits >= 0)
        changes["digits"] = digits;
    return changeColumn(property, changes.dump(), table);
}

auto TableSetup::addColumnFromJson(const std::string& text, const std::string& table) -> TableSetup&
{
    const json column = columnOf(json::parse(text));
    pimpl->forTables(table, [&](json& t) { t["columns"].push_back(column); });
    return *this;
}

auto TableSetup::addColumn(const std::string& property, const std::string& unit, int digits, const std::string& table) -> TableSetup&
{
    json column = {{"property", property}};
    if (!unit.empty())
        column["unit"] = unit;
    if (digits >= 0)
        column["digits"] = digits;
    return addColumnFromJson(column.dump(), table);
}

auto TableSetup::removeColumn(const std::string& property, const std::string& table) -> TableSetup&
{
    pimpl->forTables(table, [&](json& t) {
        if (!t.contains("columns"))
            return;
        json kept = json::array();
        for (const auto& spec : t["columns"])
            if (columnOf(spec)["property"] != property)
                kept.push_back(spec);
        t["columns"] = kept;
    });
    return *this;
}

auto TableSetup::set(const std::string& key, const std::string& text, const std::string& table) -> TableSetup&
{
    const json value = json::parse(text);
    if (key == "T" || key == "P" || key == "pairs" || key == "order")
    {
        pimpl->setCondition(table, key.c_str(), value);
        return *this;
    }
    const std::string name = key == "substances" ? "records" : key;
    pimpl->forTables(table, [&](json& t) { t[name] = value; });
    return *this;
}

auto TableSetup::toYaml() const -> std::string
{
    return blockText(pimpl->document, 0);
}

auto TableSetup::save(const std::string& path) const -> void
{
    std::ofstream file(path);
    if (!file)
        fail("Cannot write the file " + path + ".");
    file << toYaml();
}

auto TableSetup::run() const -> TableResults
{
    const auto& document = pimpl->document;
    if (!document.contains("tables") || !document["tables"].is_array() || document["tables"].empty())
        fail("The setup has no tables.");
    const Database source = pimpl->sourceDatabase();
    TableResults results;
    int index = 0;
    for (const auto& spec : document["tables"])
    {
        ++index;
        const json merged = pimpl->mergedSpec(spec);
        const std::string name = stringOr(merged, "name", "table" + std::to_string(index));
        checkTable(merged, name);
        results.tables_.push_back(pimpl->runTable(name, merged, source));
    }
    return results;
}

namespace {

/// The name that ThermoBatch has for a property.
auto batchName(bool reaction, const std::string& property) -> std::string
{
    const auto [where, field] = fieldOf(reaction, property);
    if (where != Where::Thermo)
        fail("A ThermoBatch does not calculate the property " + property + " of water: use a table.");
    return field == "log_equilibrium_constant" ? "logKr" : field == "ln_equilibrium_constant" ? "lnKr" : field;
}

} // namespace

auto TableSetup::applyTo(ThermoBatch& batch, const std::string& table) const -> void
{
    const json spec = pimpl->oneTable(table);
    const std::string name = stringOr(spec, "name", "table1");
    checkTable(spec, name);
    if (spec.contains("compare"))
        fail("The table " + name + " has compare: a ThermoBatch has one engine. Use a table.");
    const bool hasRecords = spec.contains("records") && !spec["records"].empty(), hasReactions = spec.contains("reactions") && !spec["reactions"].empty();
    if (hasRecords && hasReactions)
        fail("The table " + name + " has records and reactions: a ThermoBatch calculates one or the other.");

    if (spec.contains("conditions"))
    {
        if (spec["conditions"].contains("T"))
            batch.setPropertyUnit("temperature", axisOf(spec["conditions"]["T"], "T", "K").unit);
        if (spec["conditions"].contains("P"))
            batch.setPropertyUnit("pressure", axisOf(spec["conditions"]["P"], "P", "Pa").unit);
    }
    const auto tableDigits = digitsOf(spec, "digits");
    if (spec.contains("columns"))
        for (const auto& c : spec["columns"])
        {
            json column = columnOf(c);
            const auto property = column["property"].get<std::string>();
            if (column["show"] != "val")
                fail("A ThermoBatch writes values: the column " + property + " with show " + column["show"].get<std::string>() + " needs a table.");
            const auto batchField = batchName(hasReactions, property);
            column = withTableDefaults(column, spec, fieldOf(hasReactions, property).second);
            if (column.contains("unit"))
                batch.setPropertyUnit(batchField, unitOf(column["unit"].get<std::string>()));
            if (const auto digits = digitsOf(column, "digits"); digits || tableDigits)
                batch.setPropertyDigit(batchField, digits ? *digits : *tableDigits);
        }

    auto& p = batch.preferences();
    if (spec.contains("format"))
    {
        const std::string style = spec["format"].get<std::string>();
        if (style != "fixed" && style != "floating" && style != "scientific")
            fail("format is fixed, floating or scientific, not " + style + ".");
        p.isFixed = style == "fixed";
        p.isFloating = style == "floating";
        p.isScientific = style == "scientific";
    }
    p.separator = stringOr(spec, "separator", p.separator);
    p.fileName = stringOr(spec, "output", p.fileName);
    p.loopOverTPpairsFirst = stringOr(spec, "loop", "records") == "records";
    p.loopTemperatureThenPressure = stringOr(spec, "layout", "long") == "grid" ? stringOr(spec, "across", "P") == "P"
                                  : !spec.contains("conditions") || stringOr(spec["conditions"], "order", "T_first") != "P_first";
    if (spec.contains("write_not_defined"))
        p.writeNaNifNotDefinedValue = spec["write_not_defined"].get<bool>();
    const std::string word = stringOr(spec, "source", "");
    p.substancePropertiesFromReaction = word == "reaction";
    p.reactionPropertiesFromReactants = word == "reactants";
    p.writeProvenance = stringOr(spec, "provenance", "header") == "header";
    auto notes = pimpl->provenanceOf(spec);
    p.provenanceNotes = notes.lines();
}

auto TableSetup::batch(const std::string& table) const -> BatchCalculation
{
    const json spec = pimpl->oneTable(table);
    const std::string name = stringOr(spec, "name", "table1");
    checkTable(spec, name);

    const Database source = pimpl->sourceDatabase();
    ThermoEngine engine = pimpl->engineOf(source, spec, json::object());
    const bool reactions = spec.contains("reactions") && !spec["reactions"].empty();
    std::vector<std::string> symbols = spec.contains(reactions ? "reactions" : "records") ? symbolsOf(spec[reactions ? "reactions" : "records"], source, reactions) : std::vector<std::string>{};
    if (symbols.empty())
        fail("The table " + name + " has no records or reactions: give records or reactions.");
    if (!reactions && stringOr(spec, "source", "") == "reaction")
        for (const auto& symbol : symbols)
            engine.setPropertySource(symbol, "reaction");
    if (!spec.contains("conditions") || !spec["conditions"].contains("T") || !spec["conditions"].contains("P"))
        fail("The conditions of the table " + name + " need T and P.");

    BatchCalculation calculation{ThermoBatch(engine)};
    applyTo(calculation.batch_, table);
    calculation.symbols_ = std::move(symbols);
    calculation.reactions_ = reactions;
    calculation.layout_ = stringOr(spec, "layout", "long");
    calculation.provenance_mode_ = stringOr(spec, "provenance", "header");
    calculation.provenance_ = pimpl->provenanceOf(spec);
    calculation.provenance_.calculations.emplace_back("", engine.snapshot());
    if (spec.contains("columns"))
        for (const auto& c : spec["columns"])
            calculation.properties_.push_back(batchName(reactions, columnOf(c)["property"].get<std::string>()));
    if (calculation.properties_.empty())
        fail("The table " + name + " has no columns.");
    json conditions = spec["conditions"];
    // a grid of Output reads the results in this order
    if (calculation.layout_ == "grid")
        conditions["order"] = stringOr(spec, "across", "P") == "T" ? "P_first" : "T_first";
    const Axis axisT = axisOf(conditions["T"], "T", "K"), axisP = axisOf(conditions["P"], "P", "Pa");
    for (const auto& point : pointsOf(conditions, axisT, axisP))
        calculation.points_.push_back({point.T, point.saturation ? 0.0 : point.P});
    return calculation;
}

auto BatchCalculation::run() -> Output
{
    return reactions_ ? batch_.thermoPropertiesReaction(points_, symbols_, properties_) : batch_.thermoPropertiesSubstance(points_, symbols_, properties_);
}

auto BatchCalculation::write(const std::string& path) -> std::string
{
    const std::string where = path.empty() ? batch_.preferences().fileName : path;
    auto output = run();
    if (layout_ == "transposed")
        output.toCSVTransposed(where);
    else if (layout_ == "grid")
        output.toCSVPropertyGrid(where);
    else
        output.toCSV(where);
    if (provenance_mode_ == "file")
        writeProvenanceFile(provenance_, where);
    return where;
}

} // namespace ThermoFun
