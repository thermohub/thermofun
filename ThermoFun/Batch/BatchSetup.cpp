// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <cctype>
#include <ThermoFun/Batch/BatchSetup.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <regex>
#include <set>
#include <sstream>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Units.hpp>
#include <ThermoFun/Common/OutsideRange.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/IO/V2/V2Reader.h>
#include <ThermoFun/Model/Vocabulary.h>
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

namespace model::detail {
struct BuiltinFile
{
    const char* name;
    const unsigned char* text;
    unsigned long size;
};
extern const BuiltinFile builtin_table_templates[];
extern const unsigned long builtin_table_template_count;
} // namespace model::detail

namespace {

using json = nlohmann::ordered_json;

[[noreturn]] auto fail(const std::string& message) -> void
{
    throw BatchSetupError(message);
}

using Scalar = Reaktoro_::ThermoProperty;

const std::map<std::string, std::string> substanceNames = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"S", "entropy"}, {"Cp", "heat_capacity_cp"}, {"Cv", "heat_capacity_cv"},
                                                           {"V", "volume"}, {"U", "internal_energy"}, {"A", "helmholtz_energy"}};
const std::map<std::string, std::string> reactionNames = {{"logK", "log_equilibrium_constant"}, {"lnK", "ln_equilibrium_constant"}, {"dG", "reaction_gibbs_energy"},
                                                          {"dH", "reaction_enthalpy"}, {"dS", "reaction_entropy"}, {"dCp", "reaction_heat_capacity_cp"},
                                                          {"dCv", "reaction_heat_capacity_cv"}, {"dV", "reaction_volume"}, {"dU", "reaction_internal_energy"}, {"dA", "reaction_helmholtz_energy"}};
const std::map<std::string, std::string> baseUnits = {
    {"gibbs_energy", "J/mol"}, {"enthalpy", "J/mol"}, {"internal_energy", "J/mol"}, {"helmholtz_energy", "J/mol"}, {"entropy", "J/K/mol"}, {"heat_capacity_cp", "J/K/mol"},
    {"heat_capacity_cv", "J/K/mol"}, {"volume", "m3/mol"}, {"reaction_gibbs_energy", "J/mol"}, {"reaction_enthalpy", "J/mol"}, {"reaction_internal_energy", "J/mol"},
    {"reaction_helmholtz_energy", "J/mol"}, {"reaction_entropy", "J/K/mol"}, {"reaction_heat_capacity_cp", "J/K/mol"}, {"reaction_heat_capacity_cv", "J/K/mol"},
    {"reaction_volume", "m3/mol"}, {"density", "kg/m3"}, {"Alpha", "1/K"}, {"Beta", "1/Pa"}, {"dAldT", "1/K2"}, {"bornY", "1/K"}, {"bornQ", "1/Pa"}, {"bornX", "1/K2"},
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

enum class Where { Thermo, Solvent, Electro, Route };

auto unitOf(std::string text) -> std::string;

/// A column of the route of a result (decision 0030): "contribution:<method>:<property>", "terms:<method>:<name>",
/// "variables:<method>:<name>"
struct RouteColumn
{
    std::string part;     ///< contribution, terms, variables
    std::string column;   ///< the column of routeTable: "<method>:<name>"
    std::string si_unit;
};

/// The route column of a property name; empty if the name is not one. An error for a name of the form that names nothing.
auto routeColumn(const std::string& name) -> std::optional<RouteColumn>
{
    const auto first = name.find(':');
    const auto last = name.rfind(':');
    if (first == std::string::npos || first == last)
        return std::nullopt;
    const std::string part = name.substr(0, first), method = name.substr(first + 1, last - first - 1), what = name.substr(last + 1);
    if (part != "contribution" && part != "terms" && part != "variables")
        return std::nullopt;
    static const std::map<std::string, std::string> units = {{"G", "J/mol"}, {"H", "J/mol"}, {"S", "J/K/mol"}, {"Cp", "J/K/mol"}, {"V", "m3/mol"}};
    if (part == "contribution")
    {
        const auto unit = units.find(what);
        if (unit == units.end())
            fail("The property " + name + ": a contribution is to G, H, S, Cp or V, for example contribution:HKF88:G.");
        return RouteColumn{part, method + ":" + what, unit->second};
    }
    const auto* spec = model::MethodCatalog::instance().find(method, model::OperatesOn::Substance);
    if (!spec)
        fail("The property " + name + ": there is no method " + method + ".");
    if (part == "terms")
        for (const auto& t : spec->terms)
            if (t.name == what)
                return RouteColumn{part, method + ":" + what, unitOf(t.si_unit)};
    if (part == "variables")
        for (const auto& v : spec->variables)
            if (v.name == what)
                return RouteColumn{part, method + ":" + what, unitOf(v.si_unit)};
    fail("The method " + method + " has no " + (part == "terms" ? "term " : "variable ") + what + ". The terms and variables of a method are on its page, and in "
         "thermofun.methods.catalog(id).");
}

/// Where a property comes from, and the name of its field.
auto fieldOf(bool reaction, const std::string& name) -> std::pair<Where, std::string>
{
    const auto& names = reaction ? reactionNames : substanceNames;
    if (const auto it = names.find(name); it != names.end())
        return {Where::Thermo, it->second};
    for (const auto& item : names)
        if (item.second == name)
            return {Where::Thermo, name};
    if (routeColumn(name))
        return {Where::Route, name};
    if (reaction)
        fail("A reaction has no property " + name + ". Use one of: logK, lnK, dG, dH, dS, dCp, dCv, dV, dU, dA, or a contribution such as contribution:LogKCp:G.");
    const PropertiesSolvent solvent{};
    if (solventScalar(solvent, name) || solventNumber(solvent, name))
        return {Where::Solvent, name};
    if (electroScalar(ElectroPropertiesSolvent{}, name))
        return {Where::Electro, name};
    fail("A substance has no property " + name + ". Use one of: G, H, S, Cp, Cv, V, U, A, a property of water such as density or epsilon, or a part of the route "
         "such as contribution:HKF88:G, terms:HKF88:born_G, variables:HKF88:g.");
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

/// The volume of the engine is in m3 for one mole, and a unit such as cm3/mol has the mole in it.
auto volumeBase(const std::string& to) -> std::string
{
    return to.find("/mol") != std::string::npos ? "m3/mol" : "m3";
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
        {
            if (const auto it = baseUnits.find(field); it != baseUnits.end())
                unit = it->second;
            else if (const auto route = routeColumn(field))
                unit = route->si_unit;
        }
        if (show == "err") text += "_uncertainty";
        if (show == "ddt") text += "_ddT";
        if (show == "ddp") text += "_ddP";
        if (show == "status") text += "_status";
        if (!unit.empty() && unit != "1" && show != "status")
            text += "(" + unit + ")";
    }
    return variant.empty() ? text : text + " [" + variant + "]";
}

auto cellOf(const Scalar& x, const std::string& field, const json& column) -> ResultCell
{
    ResultCell cell;
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
    else if (key == "solvent_state") p.solvent_state = value.get<int>();
    else if (key == "solvent_symbol") p.solvent_symbol = value.get<std::string>();
    else if (key == "physical_constants") p.physical_constants = value.get<std::string>();
    else if (key == "fallback_to_reference_properties") p.fallback_to_reference_properties = value.get<bool>();
    else if (key == "apply_pressure_correction_to_gas_props") p.apply_pressure_correction_to_gas_props = value.get<bool>();
    else if (key == "cancel_fugacity_coefficient_in_gas_props") p.cancel_fugacity_coefficient_in_gas_props = value.get<bool>();
    else if (key == "round_to_uncertainty") p.round_to_uncertainty = value.get<bool>();
    else if (key == "uncertainty_significant_digits") p.uncertainty_significant_digits = value.get<int>();
    else if (key == "propagate_parameter_errors") p.propagate_parameter_errors = value.get<bool>();
    else if (key == "method_implementation") p.method_implementation = value.get<std::map<std::string, std::string>>();
    else if (key == "property_source")
    {
        const auto word = model::fromString<model::PropertySource>(value.get<std::string>());
        if (!word)
            fail(model::unknownWordMessage<model::PropertySource>(value.get<std::string>()));
        p.property_source = *word;
    }
    else
        fail("The engine has no option " + key + ".");
}

template <class E>
auto wordOf(const json& value) -> E
{
    const auto word = model::fromString<E>(value.get<std::string>());
    if (!word)
        fail(model::unknownWordMessage<E>(value.get<std::string>()));
    return *word;
}

// One group of the conventions of a setup: standard_state {pressure, concentration_scale} or energy_reference {elements, water}
auto setConvention(EngineConventions& c, const std::string& key, const json& value) -> void
{
    if (!value.is_object())
        fail("The conventions " + key + " are a mapping, for example standard_state: {pressure: 1atm}.");
    for (const auto& item : value.items())
    {
        const auto& name = item.key();
        if (key == "standard_state" && name == "pressure")
            c.standard_state.pressure = item.value().is_number() ? item.value().get<double>() : model::pascals(wordOf<model::StandardPressure>(item.value()));
        else if (key == "standard_state" && name == "concentration_scale") c.standard_state.concentration_scale = wordOf<model::ConcentrationScale>(item.value());
        else if (key == "energy_reference" && name == "elements") c.energy_reference.elements = wordOf<model::EnergyReference>(item.value());
        else if (key == "energy_reference" && name == "water") c.energy_reference.water = wordOf<model::WaterReference>(item.value());
        else if (key == "energy_reference" && name == "ions") c.energy_reference.ions = wordOf<model::Ions>(item.value());
        else if (key == "standard_state" || key == "energy_reference")
            fail("The conventions " + key + " have no field " + name + ".");
        else
            fail("The conventions have no group " + key + ": they have standard_state and energy_reference.");
    }
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
            return json::parse(model::yamlToJson(text));
        if (json::accept(text))
            return json::parse(text);
        // {a: 1} is YAML too, and the reader of YAML starts from a block
        return json::parse(model::yamlToJson("setup: " + text))["setup"];
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

auto templateFile(const std::string& name) -> const model::detail::BuiltinFile*
{
    for (unsigned long i = 0; i < model::detail::builtin_table_template_count; ++i)
        if (std::filesystem::path(model::detail::builtin_table_templates[i].name).stem().string() == name)
            return &model::detail::builtin_table_templates[i];
    return nullptr;
}

auto textOf(const model::detail::BuiltinFile& file) -> std::string
{
    return std::string(reinterpret_cast<const char*>(file.text), file.size);
}

auto describedBy(const std::string& text) -> std::string
{
    const std::string first = text.substr(0, text.find('\n'));
    return first.rfind("# ", 0) == 0 ? first.substr(2) : first;
}

/// The blocks of a setup that a result can give again (decision 0032): what a result says replaces the top, key by key
const std::vector<std::string> blocks = {"engine", "conditions", "output"};

/// A block over another, key by key: the groups of engine (options, conventions and their groups, property_sources) and the maps
/// units and digits of output one level further
auto mergeBlock(json base, const json& over, const std::string& block) -> json
{
    if (!base.is_object())
        base = json::object();
    for (const auto& item : over.items())
    {
        const auto& key = item.key();
        const bool deeper = (block == "engine" || (block == "output" && (key == "units" || key == "digits"))) && item.value().is_object() && base.contains(key) && base[key].is_object();
        if (!deeper)
        {
            base[key] = item.value();
            continue;
        }
        for (const auto& inner : item.value().items())
        {
            auto& to = base[key];
            if (block == "engine" && key == "conventions" && inner.value().is_object() && to.contains(inner.key()) && to[inner.key()].is_object())
                to[inner.key()].update(inner.value());
            else
                to[inner.key()] = inner.value();
        }
    }
    return base;
}

/// Puts `over` on `base`: its settings replace those of the setup and of its results, and a result of the same name changes that result.
auto mergeOver(json& base, const json& over) -> void
{
    const auto eachProperty = [&](auto&& f) {
        if (base.contains("results"))
            for (auto& result : base["results"])
                if (result.contains("properties"))
                    for (auto& property : result["properties"])
                        if (property.is_object())
                            f(property);
    };
    for (const auto& item : over.items())
    {
        const auto& key = item.key();
        if (key == "results")
            continue;
        if (key == "from_templates")
        {
            if (!base.contains(key))
                base[key] = json::array();
            for (const auto& name : item.value())
                base[key].push_back(name);
            continue;
        }
        if (std::find(blocks.begin(), blocks.end(), key) != blocks.end() && item.value().is_object())
        {
            base[key] = mergeBlock(base.contains(key) ? base[key] : json::object(), item.value(), key);
            // the same keys of the results give way
            if (base.contains("results"))
                for (auto& result : base["results"])
                    if (result.contains(key) && result[key].is_object())
                        for (const auto& inner : item.value().items())
                            result[key].erase(inner.key());
            // a unit or digits for a property replace those that a property of a template has
            if (key == "output" && item.value().contains("units") && item.value()["units"].is_object())
                eachProperty([&](json& property) {
                    if (item.value()["units"].contains(property.value("name", "")))
                        property.erase("unit");
                });
            if (key == "output" && item.value().contains("digits"))
                eachProperty([&](json& property) {
                    if (item.value()["digits"].is_number() || item.value()["digits"].contains(property.value("name", "")))
                        property.erase("digits");
                });
            continue;
        }
        base[key] = item.value();
    }
    if (!over.contains("results"))
        return;
    if (!base.contains("results"))
        base["results"] = json::array();
    for (const auto& result : over["results"])
    {
        const std::string name = stringOr(result, "name", "");
        const auto same = std::find_if(base["results"].begin(), base["results"].end(), [&](const json& r) { return !name.empty() && stringOr(r, "name", "") == name; });
        if (same == base["results"].end())
            base["results"].push_back(result);
        else
            for (const auto& item : result.items())
                (*same)[item.key()] = std::find(blocks.begin(), blocks.end(), item.key()) != blocks.end() && item.value().is_object()
                                          ? mergeBlock(same->contains(item.key()) ? (*same)[item.key()] : json::object(), item.value(), item.key())
                                          : item.value();
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
        mergeOver(result, expandTemplates(parseText(BatchSetup::templateText(name)), next));
        result["from_templates"].push_back(name);
    }
    mergeOver(result, document);
    return result;
}

// The keys of a batch setup (decision 0032)
const std::vector<std::string> setupKeys = {"database", "template", "from_templates", "engine", "conditions", "results", "output"};
const std::vector<std::string> engineKeys = {"options", "conventions", "property_sources"};
const std::vector<std::string> conditionKeys = {"T", "P", "points", "vary_first"};
const std::vector<std::string> resultKeys = {"name", "description", "substances", "reactions", "properties", "compare", "compare_difference", "engine", "conditions", "output"};
const std::vector<std::string> propertyKeys = {"name", "show", "unit", "digits", "number_format", "label"};
const std::vector<std::string> compareKeys = {"label", "database", "replace", "engine"};
const std::vector<std::string> outputKeys = {"file", "file_format", "number_format", "digits", "units", "layout", "grid_across", "rows_by", "csv_separator", "undefined", "provenance"};

auto checkKeys(const json& node, const std::vector<std::string>& known, const std::string& where) -> void
{
    if (!node.is_object())
        return;
    for (const auto& item : node.items())
        if (std::find(known.begin(), known.end(), item.key()) == known.end())
            fail(detail::unknownKeyMessage(where, item.key(), known));
}

/// A value that must be one of some words
auto checkWord(const json& node, const char* key, const std::vector<std::string>& words, const std::string& where) -> std::string
{
    if (!node.contains(key))
        return words.front();
    const std::string value = node[key].is_string() ? node[key].get<std::string>() : node[key].dump();
    if (std::find(words.begin(), words.end(), value) == words.end())
    {
        std::string list;
        for (const auto& w : words)
            list += (list.empty() ? "" : ", ") + w;
        fail(where + ": " + key + " is one of " + list + ", not " + value + ".");
    }
    return value;
}

/// The paths of the placeholders <...> that a setup still has (the template blank has them)
auto placeholders(const json& node, const std::string& path, std::vector<std::string>& out) -> void
{
    if (node.is_string())
    {
        const auto& text = node.get_ref<const std::string&>();
        if (text.size() >= 2 && text.front() == '<' && text.back() == '>')
            out.push_back(path + " (" + text + ")");
    }
    else if (node.is_object())
        for (const auto& item : node.items())
            placeholders(item.value(), path + "/" + item.key(), out);
    else if (node.is_array())
        for (std::size_t i = 0; i < node.size(); ++i)
            placeholders(node[i], path + "/" + std::to_string(i), out);
}

auto checkPlaceholders(const json& document) -> void
{
    std::vector<std::string> found;
    placeholders(document, "", found);
    if (found.empty())
        return;
    std::string list;
    for (const auto& f : found)
        list += (list.empty() ? "" : ", ") + f;
    fail("The setup still has placeholders: " + list + ". Replace each one, for example with setup.set(\"" + found.front().substr(0, found.front().find(' ')) + "\", value).");
}

/// The settings of one result: the blocks at the top of the setup, with those of the result over them, key by key
auto mergedResult(const json& document, const json& result) -> json
{
    json merged = json::object();
    for (const auto& item : result.items())
        if (std::find(blocks.begin(), blocks.end(), item.key()) == blocks.end())
            merged[item.key()] = item.value();
    for (const auto& block : blocks)
    {
        json value = document.contains(block) && document[block].is_object() ? document[block] : json::object();
        if (result.contains(block) && result[block].is_object())
            value = mergeBlock(value, result[block], block);
        if (!value.empty())
            merged[block] = value;
    }
    return merged;
}

/// The settings of a result in the form the calculation reads. Checks the keys and the words.
auto flatOf(const json& merged, const std::string& name) -> json
{
    const std::string where = "The result " + name;
    checkKeys(merged, resultKeys, where);
    json flat = json::object();
    for (const char* key : {"name", "description", "reactions", "compare_difference"})
        if (merged.contains(key))
            flat[key] = merged[key];
    if (merged.contains("substances"))
        flat["records"] = merged["substances"];
    if (merged.contains("properties"))
    {
        flat["columns"] = json::array();
        for (const auto& item : merged["properties"])
        {
            json property = item.is_string() ? json{{"name", item}} : item;
            if (!property.is_object() || !property.contains("name"))
                fail(where + ": a property needs a name, for example G or {name: G, unit: kJ/mol}, not " + item.dump() + ".");
            checkKeys(property, propertyKeys, "A property of the result " + name);
            json column = {{"property", property["name"]}};
            static const std::map<std::string, std::string> shows = {{"value", "val"}, {"uncertainty", "err"}, {"ddt", "ddt"}, {"ddp", "ddp"}, {"status", "status"}};
            const std::string show = checkWord(property, "show", {"value", "uncertainty", "ddt", "ddp", "status"}, "A property of the result " + name);
            column["show"] = shows.at(show);
            for (const char* key : {"unit", "digits", "label"})
                if (property.contains(key))
                    column[key] = property[key];
            if (property.contains("number_format"))
                column["format"] = property["number_format"];
            flat["columns"].push_back(column);
        }
    }
    const auto engineOf = [&](const json& engine, json& to, const std::string& in) {
        checkKeys(engine, engineKeys, "The engine of " + in);
        if (engine.contains("options"))
            to["options"] = engine["options"];
        if (engine.contains("conventions"))
            to["conventions"] = engine["conventions"];
        if (engine.contains("property_sources"))
            to["sources"] = engine["property_sources"];
    };
    if (merged.contains("engine"))
        engineOf(merged["engine"], flat, "the result " + name);
    if (merged.contains("compare"))
    {
        flat["compare"] = json::array();
        for (const auto& variant : merged["compare"])
        {
            checkKeys(variant, compareKeys, "A comparison of the result " + name);
            json v = json::object();
            for (const char* key : {"label", "database", "replace"})
                if (variant.contains(key))
                    v[key] = variant[key];
            if (variant.contains("engine"))
                engineOf(variant["engine"], v, "a comparison of the result " + name);
            flat["compare"].push_back(v);
        }
    }
    if (merged.contains("conditions"))
    {
        const auto& c = merged["conditions"];
        checkKeys(c, conditionKeys, "The conditions of the result " + name);
        json conditions = json::object();
        for (const char* key : {"T", "P"})
            if (c.contains(key))
                conditions[key] = c[key];
        conditions["pairs"] = checkWord(c, "points", {"grid", "pairs"}, "The conditions of the result " + name) == "pairs";
        // the condition that changes from one row to the next: P (the default: the rows of one T together) or T
        conditions["order"] = checkWord(c, "vary_first", {"P", "T"}, "The conditions of the result " + name) == "T" ? "P_first" : "T_first";
        flat["conditions"] = conditions;
    }
    if (merged.contains("output"))
    {
        const auto& o = merged["output"];
        checkKeys(o, outputKeys, "The output of the result " + name);
        const std::string in = "The output of the result " + name;
        if (o.contains("file")) flat["output"] = o["file"];
        if (o.contains("file_format")) flat["format_file"] = checkWord(o, "file_format", {"csv", "md", "tex"}, in);
        if (o.contains("number_format")) flat["format"] = checkWord(o, "number_format", {"floating", "fixed", "scientific"}, in);
        for (const char* key : {"digits", "units"})
            if (o.contains(key))
            {
                json value = o[key];
                if (value.is_object())
                {
                    const std::string suffix = std::string(key) == "digits" ? "digits_" : "unit_shown_";
                    for (const char* axis : {"T", "P"})
                        if (value.contains(axis))
                        {
                            flat[suffix + axis] = value[axis];
                            value.erase(axis);
                        }
                }
                flat[key] = value;
            }
        const std::string layout = checkWord(o, "layout", {"long", "wide", "grid"}, in);
        flat["layout"] = layout == "wide" ? "transposed" : layout;
        flat["across"] = checkWord(o, "grid_across", {"P", "T"}, in);
        flat["loop"] = checkWord(o, "rows_by", {"substance", "condition"}, in) == "condition" ? "conditions" : "records";
        if (o.contains("csv_separator")) flat["separator"] = o["csv_separator"];
        if (o.contains("undefined"))
        {
            if (!o["undefined"].is_string())
                fail(in + ": undefined is a text to write for a value that is not defined, or the word number, not " + o["undefined"].dump() + ".");
            if (o["undefined"] == "number")
                flat["write_not_defined"] = false;
            else
                flat["not_defined"] = o["undefined"];
        }
        flat["provenance"] = checkWord(o, "provenance", {"header", "file", "none"}, in);
    }
    flat["_setup"] = merged;
    return flat;
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
            fail(std::string(reaction ? "reactions" : "substances") + " has a value that is not a symbol or a rule: " + item.dump() + ".");
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

auto ResultTable::values() const -> std::vector<std::vector<double>>
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

auto ResultTable::pressureUsed(const std::string& record, double T, double P, bool at_saturation) const -> double
{
    for (const auto& row : rows_)
        if (row.record == record && row.T == T && row.saturation == at_saturation && (at_saturation || row.P == P))
            return !row.saturation ? row.P : row.P_used ? *row.P_used : std::nan("");
    fail("The result " + name_ + " has no row " + record + " at T = " + std::to_string(T) + (at_saturation ? " and saturation." : " and P = " + std::to_string(P) + "."));
}

auto ResultTable::cell(const std::string& record, double T, double P, const std::string& column, bool at_saturation) const -> const ResultCell&
{
    std::size_t index = columns_.size();
    for (std::size_t i = 0; i < columns_.size() && index == columns_.size(); ++i)
        if (columns_[i].label == column || columns_[i].property == column)
            index = i;
    if (index == columns_.size())
        fail("The result " + name_ + " has no column " + column + ".");
    for (const auto& row : rows_)
        if (row.record == record && row.T == T && row.saturation == at_saturation && (at_saturation || row.P == P))
            return row.cells[index];
    fail("The result " + name_ + " has no row for " + record + " at this T and P.");
}

auto ResultTable::format(const ResultColumn& column, double value) const -> std::string
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

auto ResultTable::number(const ResultColumn& column, const ResultCell* cell) const -> std::string
{
    if (!cell)
        return not_defined_;
    if (!cell->text.empty())
        return cell->text;
    if (!cell->defined && write_not_defined_)
        return not_defined_;
    return format(column, cell->value);
}

auto ResultTable::condition(double value, bool saturation, const std::optional<int>& digits, const std::optional<double>& used) const -> std::string
{
    if (saturation && !used)
        return "saturation";
    if (saturation)
        value = *used;
    char text[64];
    if (digits)
        std::snprintf(text, sizeof text, "%.*f", *digits, value);
    else
        std::snprintf(text, sizeof text, "%g", value);
    return text;
}

auto ResultTable::shown(double value, bool isT) const -> double
{
    const auto& from = isT ? unit_T_ : unit_P_;
    const auto& to = isT ? shown_T_ : shown_P_;
    return to.empty() || to == from ? value : convert(value, from, to);
}

auto ResultTable::shownUsed(const ResultRow& row) const -> std::optional<double>
{
    return row.P_used ? std::optional<double>(shown(*row.P_used, false)) : std::nullopt;
}

auto ResultTable::lines() const -> std::vector<std::vector<std::string>>
{
    std::vector<std::vector<std::string>> out;
    const std::string labelT = "T(" + (shown_T_.empty() ? unit_T_ : shown_T_) + ")", labelP = "P(" + (shown_P_.empty() ? unit_P_ : shown_P_) + ")";
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
            std::stable_sort(ordered.begin(), ordered.end(), [&](const ResultRow& a, const ResultRow& b) {
                return std::find(points.begin(), points.end(), std::make_pair(a.T, a.P)) < std::find(points.begin(), points.end(), std::make_pair(b.T, b.P));
            });
        }
        for (const auto& row : ordered)
        {
            std::vector<std::string> line = {row.record, condition(shown(row.T, true), false, digits_T_), condition(shown(row.P, false), row.saturation, digits_P_, shownUsed(row))};
            for (std::size_t i = 0; i < columns_.size(); ++i)
                line.push_back(number(columns_[i], i < row.cells.size() ? &row.cells[i] : nullptr));
            out.push_back(std::move(line));
        }
        return out;
    }
    if (layout_ != "transposed")
        fail("The layout is long, wide or grid, not " + layout_ + ".");
    std::vector<std::string> records;
    std::vector<const ResultRow*> points;
    for (const auto& row : rows_)
    {
        if (std::find(records.begin(), records.end(), row.record) == records.end())
            records.push_back(row.record);
        if (std::none_of(points.begin(), points.end(), [&](const ResultRow* p) { return p->T == row.T && p->P == row.P && p->saturation == row.saturation; }))
            points.push_back(&row);
    }
    std::vector<std::string> header = {"Property", labelT, labelP};
    header.insert(header.end(), records.begin(), records.end());
    out.push_back(header);
    for (std::size_t i = 0; i < columns_.size(); ++i)
        for (const auto* point : points)
        {
            std::vector<std::string> line = {columns_[i].label, condition(shown(point->T, true), false, digits_T_), condition(shown(point->P, false), point->saturation, digits_P_, shownUsed(*point))};
            for (const auto& record : records)
            {
                const auto row = std::find_if(rows_.begin(), rows_.end(), [&](const ResultRow& r) { return r.record == record && r.T == point->T && r.P == point->P && r.saturation == point->saturation; });
                line.push_back(row == rows_.end() ? "" : number(columns_[i], &row->cells[i]));
            }
            out.push_back(std::move(line));
        }
    return out;
}

auto ResultTable::propertyGrids() const -> std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>
{
    const bool acrossT = across_ == "T";
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> out;
    const std::string uT = shown_T_.empty() ? unit_T_ : shown_T_, uP = shown_P_.empty() ? unit_P_ : shown_P_;
    const std::string corner = acrossT ? "P(" + uP + ")_T(" + uT + ")" : "T(" + uT + ")_P(" + uP + ")";
    std::vector<std::string> records;
    for (const auto& row : rows_)
        if (std::find(records.begin(), records.end(), row.record) == records.end())
            records.push_back(row.record);
    const auto textT = [&](std::size_t i) { return condition(shown(Ts_[i], true), false, digits_T_); };
    const auto textP = [&](std::size_t i) { return condition(shown(Ps_[i], false), Ps_saturation_[i], digits_P_); };
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
                    const auto row = std::find_if(rows_.begin(), rows_.end(), [&](const ResultRow& r) { return r.record == record && r.T == Ts_[iT] && r.P == Ps_[iP] && r.saturation == static_cast<bool>(Ps_saturation_[iP]); });
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

auto Provenance::text() const -> std::string
{
    std::string out;
    for (const auto& line : lines())
        out += line + "\n";
    return out;
}

auto Provenance::lines() const -> std::vector<std::string>
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
    for (const auto& [label, summary] : calculations)
    {
        std::stringstream text(summary.text());
        for (std::string line; std::getline(text, line);)
            out.push_back((label.empty() ? "" : "[" + label + "] ") + line);
    }
    return out;
}

auto Provenance::toYaml() const -> std::string
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

auto writeProvenanceFile(const Provenance& provenance, const std::string& where) -> void
{
    std::ofstream file(where + ".provenance.yaml");
    if (!file)
        fail("Cannot write the file " + where + ".provenance.yaml.");
    file << provenance.toYaml();
}

} // namespace

auto ResultTable::toText(const std::string& kind, bool with_provenance) const -> std::string
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

auto ResultTable::write(const std::string& path) const -> std::string
{
    const std::string where = path.empty() ? output_ : path;
    if (where.empty())
        fail("The result " + name_ + " has no output.");
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

auto BatchResults::at(const std::string& name) const -> const ResultTable&
{
    for (const auto& table : tables_)
        if (table.name() == name)
            return table;
    fail("There is no table " + name + ".");
}

auto BatchResults::write() const -> std::vector<std::string>
{
    std::vector<std::string> paths;
    for (const auto& table : tables_)
        if (!table.output().empty())
            paths.push_back(table.write());
    return paths;
}

auto BatchResults::problems() const -> std::vector<BatchProblem>
{
    std::vector<BatchProblem> out;
    for (const auto& table : tables_)
        out.insert(out.end(), table.problems().begin(), table.problems().end());
    return out;
}

// ---------------------------------------------------------------------------------------------------------------------

struct BatchSetup::Impl
{
    json document = json::object();
    std::string base;
    std::optional<Database> database;

    template <class F>
    auto forResults(const std::string& name, F&& f) -> void
    {
        if (!document.contains("results") || !document["results"].is_array())
            fail("The setup has no results.");
        bool found = false;
        for (auto& result : document["results"])
            if (name.empty() || stringOr(result, "name", "") == name)
            {
                f(result);
                found = true;
            }
        if (!found)
            fail("The setup has no result " + name + ".");
    }

    /// A key of a block: at the top for all results (the same key of the results gives way), or in one result
    auto setInBlock(const std::string& result, const char* block, const char* key, const json& value) -> void
    {
        if (!result.empty())
        {
            forResults(result, [&](json& r) { r[block][key] = value; });
            return;
        }
        document[block][key] = value;
        if (document.contains("results"))
            for (auto& r : document["results"])
                if (r.contains(block) && r[block].is_object())
                    r[block].erase(key);
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
        for (const auto* scope : {&spec, &variant})
            if (scope->contains("sources"))
                for (const auto& item : (*scope)["sources"].items())
                    engine.setPropertySource(item.key(), item.value().get<std::string>());
        return engine;
    }

    /// The settings of a result in the form the calculation reads: those of the top of the setup, with those of the result over them.
    auto mergedSpec(const json& result, const std::string& name) const -> json
    {
        checkKeys(document, setupKeys, "The setup");
        return flatOf(mergedResult(document, result), name);
    }

    /// The settings of the result with this name. Without a name, the setup must have one result.
    auto oneTable(const std::string& result) const -> json
    {
        if (!document.contains("results") || !document["results"].is_array() || document["results"].empty())
            fail("The setup has no results.");
        checkPlaceholders(document);
        std::string names;
        for (const auto& r : document["results"])
        {
            if (result.empty() ? document["results"].size() == 1 : stringOr(r, "name", "") == result)
                return mergedSpec(r, stringOr(r, "name", "result1"));
            names += (names.empty() ? "" : ", ") + stringOr(r, "name", "");
        }
        fail("Name one result of the setup. It has: " + names + ".");
    }

    /// The part of the provenance that is the same for a table and for a batch.
    auto provenanceOf(const json& spec) const -> Provenance
    {
        Provenance p;
        p.database = document.contains("database") && document["database"].is_string() ? document["database"].get<std::string>() : "given in memory";
        if (document.contains("from_templates"))
            for (const auto& name : document["from_templates"])
            {
                const auto where = BatchSetup::catalog(name.get<std::string>()).source;
                p.templates.push_back(name.get<std::string>() + (where == "library" ? "" : " (" + where + ")"));
            }
        json settings = spec.contains("_setup") ? spec["_setup"] : spec;
        settings.erase("from_templates");
        p.setup = blockText(settings, 0);
        return p;
    }

    auto runTable(const std::string& name, const json& spec, const Database& source) const -> ResultTable
    {
        ResultTable table;
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
        table.shown_T_ = stringOr(spec, "unit_shown_T", "");
        table.shown_P_ = stringOr(spec, "unit_shown_P", "");
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
            fail("The result " + name + " has no substances or reactions: give substances or reactions.");
        if (!spec.contains("conditions") || !spec["conditions"].contains("T") || !spec["conditions"].contains("P"))
            fail("The conditions of the result " + name + " need T and P.");
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
            fail("The result " + name + " has no properties.");

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
            table.provenance_.calculations.emplace_back(stringOr(v, "label", ""), engines.back().summary());
        }
        for (const auto& v : variants)
            for (const auto& column : columns)
            {
                ResultColumn c;
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
                BatchProblem problem;
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
                ResultRow row;
                row.record = record;
                row.T = point.T;
                row.P = point.P;
                row.saturation = point.saturation;
                for (auto& engine : engines)
                    for (const auto& column : columns)
                    {
                        const auto [where, field] = fieldOf(reaction, column["property"].get<std::string>());
                        double Pcall = P;
                        const std::size_t cellsBefore = row.cells.size();
                        try
                        {
                            if (where == Where::Solvent)
                            {
                                const auto x = engine.propertiesSolvent(T, Pcall, record);
                                if (const auto* s = solventScalar(x, field))
                                    row.cells.push_back(cellOf(*s, field, column));
                                else
                                {
                                    ResultCell cell;
                                    cell.value = convert(*solventNumber(x, field), baseUnits.count(field) ? baseUnits.at(field) : "", stringOr(column, "unit", ""));
                                    cell.defined = true;
                                    row.cells.push_back(cell);
                                }
                            }
                            else if (where == Where::Electro)
                                row.cells.push_back(cellOf(*electroScalar(engine.electroPropertiesSolvent(T, Pcall, record), field), field, column));
                            else if (where == Where::Route)
                            {
                                if (column["show"] != "val")
                                    fail("The column " + field + " is a part of the route of a result: it has a value only, not " + column["show"].get<std::string>() + ".");
                                const auto route = *routeColumn(field);
                                const auto table = engine.routeTable({T}, {Pcall}, record, route.part);
                                const auto it = std::find(table.columns.begin(), table.columns.end(), route.column);
                                const double value = it == table.columns.end() ? std::numeric_limits<double>::quiet_NaN() : table.values.front()[it - table.columns.begin()];
                                if (it == table.columns.end() && route.part != "contribution")
                                    throw std::runtime_error("The record " + record + " has no " + field + ": it does not use that method, or the implementation of the method gives none.");
                                if (point.saturation && !row.P_used && table.P.front() > 0.0)
                                    row.P_used = convert(table.P.front(), "Pa", axisP.unit);
                                ResultCell cell;
                                cell.value = std::isfinite(value) ? convert(value, route.si_unit, stringOr(column, "unit", "")) : 0.0;
                                cell.defined = std::isfinite(value);
                                row.cells.push_back(cell);
                            }
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
                        catch (const BatchSetupError&)
                        {
                            throw;
                        }
                        catch (const std::exception& error)
                        {
                            BatchProblem problem;
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
                        if (row.cells.size() > cellsBefore && row.cells.back().has_result)
                        {
                            // a saturation point: the pressure that the calculation used
                            if (point.saturation && !row.P_used && Pcall > 0.0)
                                row.P_used = convert(Pcall, "Pa", axisP.unit);
                            // a value that is calculated, with a message: for example a point outside the range of a method
                            std::string note = row.cells.back().result.sta.second;
                            while (!note.empty() && (note.back() == ' ' || note.back() == ';' || note.back() == '\n'))
                                note.pop_back();
                            const bool known = std::any_of(table.problems_.begin(), table.problems_.end(), [&](const BatchProblem& q) {
                                return q.record == record && q.T == point.T && q.P == point.P && q.saturation == point.saturation && q.message == note;
                            });
                            // a value calculated outside the range of its model (the methods mark the message, see OutsideRange.h)
                            const bool outside = extrapolated(row.cells.back().result);
                            if (outside && !known)
                            {
                                BatchProblem problem;
                                problem.table = name;
                                problem.record = record;
                                problem.T = point.T;
                                problem.P = point.P;
                                problem.saturation = point.saturation;
                                problem.has_point = true;
                                problem.message = note;
                                table.problems_.push_back(std::move(problem));
                            }
                        }
                    }
                table.rows_.push_back(std::move(row));
            }
        }
        // compare_difference: for each property, the difference of each variant to the first one, in the unit of the property
        if (spec.contains("compare_difference") && spec["compare_difference"].get<bool>() && variants.size() > 1)
        {
            const std::size_t n = columns.size();
            const std::string first = stringOr(variants.front(), "label", "1");
            for (std::size_t v = 1; v < variants.size(); ++v)
                for (std::size_t c = 0; c < n; ++c)
                {
                    ResultColumn d = table.columns_[c];
                    d.label = labelOf(columns[c], fieldOf(kinds.front().first, d.property).second, stringOr(variants[v], "label", std::to_string(v + 1)) + " - " + first);
                    table.columns_.push_back(std::move(d));
                    for (auto& row : table.rows_)
                    {
                        ResultCell cell;
                        const auto& a = row.cells[c];
                        const auto& b = row.cells[v * n + c];
                        cell.defined = a.defined && b.defined && a.text.empty() && b.text.empty();
                        cell.value = cell.defined ? b.value - a.value : 0.0;
                        row.cells.push_back(cell);
                    }
                }
        }
        return table;
    }
};

BatchSetup::BatchSetup() : pimpl(new Impl()) {}
BatchSetup::BatchSetup(const BatchSetup& other) : pimpl(new Impl(*other.pimpl)) {}
auto BatchSetup::operator=(BatchSetup other) -> BatchSetup&
{
    pimpl = std::move(other.pimpl);
    return *this;
}
BatchSetup::~BatchSetup() = default;

auto BatchSetup::fromText(const std::string& text) -> BatchSetup
{
    BatchSetup setup;
    json document = parseText(text);
    if (!document.is_object())
        fail("The setup is not a mapping.");
    setup.pimpl->document = expandTemplates(std::move(document));
    return setup;
}

auto BatchSetup::fromFile(const std::string& path) -> BatchSetup
{
    auto setup = fromText(readFile(path));
    setup.pimpl->base = std::filesystem::absolute(path).parent_path().string();
    return setup;
}

auto BatchSetup::catalog() -> std::vector<BatchTemplateInfo>
{
    std::map<std::string, BatchTemplateInfo> rows;
    const auto add = [&](const std::string& name, const std::string& text, const std::string& source) {
        BatchTemplateInfo row{name, describedBy(text), {}, source};
        try
        {
            const json document = parseText(text);
            for (const char* block : {"database", "engine", "conditions", "results", "output"})
                if (document.is_object() && document.contains(block))
                    row.blocks.emplace_back(block);
        }
        catch (const std::exception&)
        {
            // a file that cannot be read is listed without its blocks; using it gives the error
        }
        rows[name] = std::move(row);
    };
    for (unsigned long i = 0; i < model::detail::builtin_table_template_count; ++i)
    {
        const auto& file = model::detail::builtin_table_templates[i];
        add(std::filesystem::path(file.name).stem().string(), textOf(file), "library");
    }
    for (const auto& item : userTemplateFiles())
        add(item.first, readFile(item.second.string()), item.second.string());
    std::vector<BatchTemplateInfo> out;
    for (auto& item : rows)
        out.push_back(std::move(item.second));
    return out;
}

auto BatchSetup::catalog(const std::string& name) -> BatchTemplateInfo
{
    std::string names;
    for (auto& row : catalog())
    {
        if (row.name == name)
            return row;
        names += (names.empty() ? "" : ", ") + row.name;
    }
    fail("There is no template " + name + ". Use one of: " + names + ".");
    return {};
}

auto BatchSetup::templateFolders() -> std::vector<std::string>
{
    return folderList();
}

auto BatchSetup::templateText(const std::string& name) -> std::string
{
    const auto files = userTemplateFiles();
    if (const auto it = files.find(name); it != files.end())
        return readFile(it->second.string());
    const auto* file = templateFile(name);
    if (!file)
        catalog(name);  // fails, with the names there are
    return textOf(*file);
}

auto BatchSetup::fromTemplate(const std::string& name) -> BatchSetup
{
    return fromText(json{{"template", name}}.dump());
}

auto BatchSetup::useTemplate(const std::string& name) -> BatchSetup&
{
    mergeOver(pimpl->document, expandTemplates(json{{"template", name}}));
    return *this;
}

auto BatchSetup::setDatabase(const Database& database) -> BatchSetup&
{
    pimpl->database = database;
    pimpl->document.erase("database");
    return *this;
}

auto BatchSetup::setDatabase(const std::string& path) -> BatchSetup&
{
    pimpl->database.reset();
    pimpl->document["database"] = path;
    return *this;
}

auto BatchSetup::setSubstances(const std::vector<std::string>& symbols, const std::string& result) -> BatchSetup&
{
    pimpl->forResults(result, [&](json& r) { r["substances"] = symbols; });
    return *this;
}

auto BatchSetup::setReactions(const std::vector<std::string>& symbols, const std::string& result) -> BatchSetup&
{
    pimpl->forResults(result, [&](json& r) { r["reactions"] = symbols; });
    return *this;
}

auto BatchSetup::setSubstanceRule(const std::string& rule, const std::string& result) -> BatchSetup&
{
    const json parsed = parseText(rule);
    pimpl->forResults(result, [&](json& r) { r["substances"] = parsed; });
    return *this;
}

auto BatchSetup::setReactionRule(const std::string& rule, const std::string& result) -> BatchSetup&
{
    const json parsed = parseText(rule);
    pimpl->forResults(result, [&](json& r) { r["reactions"] = parsed; });
    return *this;
}

auto BatchSetup::setTemperatures(const std::vector<double>& values, const std::string& unit, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "conditions", "T", json{{"values", values}, {"unit", unit}});
    return *this;
}

auto BatchSetup::setTemperatures(double first, double last, double step, const std::string& unit, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "conditions", "T", json{{"from", first}, {"to", last}, {"step", step}, {"unit", unit}});
    return *this;
}

auto BatchSetup::setPressures(const std::vector<double>& values, const std::string& unit, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "conditions", "P", json{{"values", values}, {"unit", unit}});
    return *this;
}

auto BatchSetup::setPressures(double first, double last, double step, const std::string& unit, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "conditions", "P", json{{"from", first}, {"to", last}, {"step", step}, {"unit", unit}});
    return *this;
}

auto BatchSetup::setSaturationPressure(const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "conditions", "P", json{{"values", json::array({"saturation"})}, {"unit", "bar"}});
    return *this;
}

auto BatchSetup::setLayout(const std::string& layout, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "output", "layout", layout);
    return *this;
}

auto BatchSetup::setOutputFile(const std::string& path, const std::string& result) -> BatchSetup&
{
    pimpl->setInBlock(result, "output", "file", path);
    return *this;
}

namespace {

/// A property entry as an object
auto propertyOf(const json& spec) -> json
{
    json property = spec.is_string() ? json{{"name", spec}} : spec;
    if (!property.is_object() || !property.contains("name"))
        fail("A property needs a name, for example G or {name: G, unit: kJ/mol}, not " + spec.dump() + ".");
    return property;
}

} // namespace

auto BatchSetup::changeProperty(const std::string& property, const std::string& text, const std::string& result) -> BatchSetup&
{
    const json changes = json::parse(text);
    bool found = false;
    pimpl->forResults(result, [&](json& r) {
        if (!r.contains("properties"))
            return;
        for (auto& spec : r["properties"])
        {
            json entry = propertyOf(spec);
            if (entry["name"] == property)
            {
                entry.update(changes);
                spec = entry;
                found = true;
            }
        }
    });
    if (!found)
        fail("No result has the property " + property + ". Add it with addProperty.");
    return *this;
}

auto BatchSetup::setProperty(const std::string& property, const std::string& unit, int digits, const std::string& result) -> BatchSetup&
{
    json changes = json::object();
    if (!unit.empty())
        changes["unit"] = unit;
    if (digits >= 0)
        changes["digits"] = digits;
    return changeProperty(property, changes.dump(), result);
}

auto BatchSetup::addPropertyFromJson(const std::string& text, const std::string& result) -> BatchSetup&
{
    const json entry = propertyOf(json::parse(text));
    pimpl->forResults(result, [&](json& r) { r["properties"].push_back(entry); });
    return *this;
}

auto BatchSetup::addProperty(const std::string& property, const std::string& unit, int digits, const std::string& result) -> BatchSetup&
{
    json entry = {{"name", property}};
    if (!unit.empty())
        entry["unit"] = unit;
    if (digits >= 0)
        entry["digits"] = digits;
    return addPropertyFromJson(entry.dump(), result);
}

auto BatchSetup::removeProperty(const std::string& property, const std::string& result) -> BatchSetup&
{
    pimpl->forResults(result, [&](json& r) {
        if (!r.contains("properties"))
            return;
        json kept = json::array();
        for (const auto& spec : r["properties"])
            if (propertyOf(spec)["name"] != property)
                kept.push_back(spec);
        r["properties"] = kept;
    });
    return *this;
}

auto BatchSetup::set(const std::string& path, const std::string& text) -> BatchSetup&
{
    json value = json::parse(text);
    if (path.empty() || path == "/")
    {
        if (!value.is_object())
            fail("The setup is a mapping.");
        pimpl->document = expandTemplates(std::move(value));
        return *this;
    }
    json::json_pointer pointer;
    try
    {
        pointer = json::json_pointer(path);
    }
    catch (const std::exception&)
    {
        fail("The path " + path + " is not a path of the setup: it starts with / and names the keys, for example /results/0/name or /conditions/T.");
    }
    // a list grows by one with the index of its end, or "-"
    try
    {
        pimpl->document[pointer] = std::move(value);
    }
    catch (const std::exception& error)
    {
        fail("Cannot set " + path + ": " + error.what());
    }
    if (path == "/database")
        pimpl->database.reset();
    return *this;
}

auto BatchSetup::get(const std::string& path) const -> std::string
{
    try
    {
        return pimpl->document.at(json::json_pointer(path.empty() ? "" : path)).dump();
    }
    catch (const std::exception&)
    {
        fail("The setup has nothing at " + path + ".");
    }
}

auto BatchSetup::toYaml() const -> std::string
{
    return blockText(pimpl->document, 0);
}

auto BatchSetup::save(const std::string& path) const -> void
{
    // a database is found next to the file: write its path from there
    BatchSetup saved(*this);
    auto& document = saved.pimpl->document;
    if (document.contains("database") && document["database"].is_string())
    {
        const std::filesystem::path database(document["database"].get<std::string>());
        if (!database.is_absolute())
        {
            const auto from = pimpl->base.empty() ? std::filesystem::absolute(database) : std::filesystem::path(pimpl->base) / database;
            document["database"] = std::filesystem::proximate(from, std::filesystem::absolute(path).parent_path()).generic_string();
        }
    }
    std::ofstream file(path);
    if (!file)
        fail("Cannot write the file " + path + ".");
    file << saved.toYaml();
}

auto BatchSetup::run() const -> BatchResults
{
    const auto& document = pimpl->document;
    if (!document.contains("results") || !document["results"].is_array() || document["results"].empty())
        fail("The setup has no results: give results, each with substances or reactions and properties.");
    checkPlaceholders(document);
    // all the results are checked before the first calculation
    std::vector<std::pair<std::string, json>> specs;
    int index = 0;
    for (const auto& result : document["results"])
    {
        ++index;
        const std::string name = stringOr(result, "name", "result" + std::to_string(index));
        specs.emplace_back(name, pimpl->mergedSpec(result, name));
    }
    const Database source = pimpl->sourceDatabase();
    BatchResults results;
    for (const auto& [name, spec] : specs)
        results.tables_.push_back(pimpl->runTable(name, spec, source));
    return results;
}

namespace {

/// The name that ThermoBatch has for a property.
auto batchName(bool reaction, const std::string& property) -> std::string
{
    const auto [where, field] = fieldOf(reaction, property);
    if (where == Where::Route)
        fail("A ThermoBatch does not calculate the part " + property + " of a route: use a table.");
    if (where != Where::Thermo)
        fail("A ThermoBatch does not calculate the property " + property + " of water: use a table.");
    return field == "log_equilibrium_constant" ? "logKr" : field == "ln_equilibrium_constant" ? "lnKr" : field;
}

} // namespace

auto BatchSetup::applyTo(ThermoBatch& batch, const std::string& table) const -> void
{
    const json spec = pimpl->oneTable(table);
    const std::string name = stringOr(spec, "name", "result1");
    if (spec.contains("compare"))
        fail("The result " + name + " has compare: a ThermoBatch has one engine. Use tf.batch.");
    const bool hasRecords = spec.contains("records") && !spec["records"].empty(), hasReactions = spec.contains("reactions") && !spec["reactions"].empty();
    if (hasRecords && hasReactions)
        fail("The result " + name + " has substances and reactions: a ThermoBatch calculates one or the other.");

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
                fail("A ThermoBatch writes values: the property " + property + " with show " + column["show"].get<std::string>() + " needs tf.batch.");
            const auto batchField = batchName(hasReactions, property);
            column = withTableDefaults(column, spec, fieldOf(hasReactions, property).second);
            if (column.contains("unit"))
                batch.setPropertyUnit(batchField, unitOf(column["unit"].get<std::string>()));
            if (const auto digits = digitsOf(column, "digits"); digits || tableDigits)
                batch.setPropertyDigit(batchField, digits ? *digits : *tableDigits);
        }

    if (const auto digits = digitsOf(spec, "digits_T"))
        batch.setPropertyDigit("temperature", *digits);
    if (const auto digits = digitsOf(spec, "digits_P"))
        batch.setPropertyDigit("pressure", *digits);

    auto& p = batch.preferences();
    if (spec.contains("format"))
    {
        const std::string style = spec["format"].get<std::string>();
        if (style != "fixed" && style != "floating" && style != "scientific")
            fail("number_format is fixed, floating or scientific, not " + style + ".");
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
    const std::string word = spec.contains("options") && spec["options"].contains("property_source") ? spec["options"]["property_source"].get<std::string>() : "";
    p.substancePropertiesFromReaction = word == "reaction";
    p.reactionPropertiesFromReactants = word == "reactants";
    p.writeProvenance = stringOr(spec, "provenance", "header") == "header";
    p.provenanceNotes = pimpl->provenanceOf(spec).lines();
}

auto BatchSetup::batch(const std::string& table) const -> BatchCalculation
{
    const json spec = pimpl->oneTable(table);
    const std::string name = stringOr(spec, "name", "result1");

    const Database source = pimpl->sourceDatabase();
    ThermoEngine engine = pimpl->engineOf(source, spec, json::object());
    const bool reactions = spec.contains("reactions") && !spec["reactions"].empty();
    std::vector<std::string> symbols = spec.contains(reactions ? "reactions" : "records") ? symbolsOf(spec[reactions ? "reactions" : "records"], source, reactions) : std::vector<std::string>{};
    if (symbols.empty())
        fail("The result " + name + " has no substances or reactions: give substances or reactions.");
    if (!spec.contains("conditions") || !spec["conditions"].contains("T") || !spec["conditions"].contains("P"))
        fail("The conditions of the result " + name + " need T and P.");

    BatchCalculation calculation{ThermoBatch(engine)};
    applyTo(calculation.batch_, table);
    calculation.symbols_ = std::move(symbols);
    calculation.reactions_ = reactions;
    calculation.layout_ = stringOr(spec, "layout", "long");
    calculation.provenance_mode_ = stringOr(spec, "provenance", "header");
    calculation.provenance_ = pimpl->provenanceOf(spec);
    calculation.provenance_.calculations.emplace_back("", engine.summary());
    if (spec.contains("columns"))
        for (const auto& c : spec["columns"])
            calculation.properties_.push_back(batchName(reactions, columnOf(c)["property"].get<std::string>()));
    if (calculation.properties_.empty())
        fail("The result " + name + " has no properties.");
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
