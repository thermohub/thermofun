// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/Methods/Symbolic/SymbolicMethod.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/Sha256.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/Methods/MethodHelpers.h>
#include <ThermoFun/Methods/Symbolic/SymbolicExpression.h>

namespace ThermoFun::v2 {

namespace {

/// G and its derivatives by T and P, parsed and differentiated once
struct Compiled
{
    SymbolicExpression G, dT, dP, dTT;
};

auto expressionsOf(const MethodDefinition& definition, std::size_t piece) -> const std::map<std::string, std::string>&
{
    return definition.expression_segments.empty() ? definition.expression : definition.expression_segments.at(piece);
}

auto compile(const std::string& text, const std::vector<std::string>& variables, const std::string& id) -> std::shared_ptr<const Compiled>
{
    static std::mutex mutex;
    static std::map<std::string, std::shared_ptr<const Compiled>> cache;
    std::string key = text;
    for (const auto& name : variables)
        key += "|" + name;
    const std::lock_guard<std::mutex> lock(mutex);
    if (const auto it = cache.find(key); it != cache.end())
        return it->second;
    auto compiled = std::make_shared<Compiled>();
    try
    {
        compiled->G = SymbolicExpression::parse(text, variables);
        compiled->dT = compiled->G.diff("T");
        compiled->dP = compiled->G.diff("P");
        compiled->dTT = compiled->dT.diff("T");
    }
    catch (const SymbolicError& error)
    {
        funError("Cannot make the symbolic method " + id, error.code + ": " + error.what(), __LINE__, __FILE__);
    }
    cache.emplace(key, compiled);
    return compiled;
}

auto specOf(const MethodDefinition& definition) -> MethodSpec
{
    MethodSpec spec;
    spec.id = definition.symbol;
    spec.operates_on = OperatesOn::Substance;
    spec.role = MethodRole::SubstanceProperties;
    spec.implemented = true;
    spec.depends_on = Dependence::TP;
    const bool base = isSymbolicBase(definition);
    const Effect effect = base ? Effect::Set : Effect::Add;
    spec.effects.G = spec.effects.H = spec.effects.S = spec.effects.Cp = spec.effects.V = effect;
    spec.effects.base = base;
    spec.effects.rank = base ? 0 : 250;
    return spec;
}

class SymbolicSubstanceMethod final : public SubstanceMethod
{
public:
    SymbolicSubstanceMethod(const MethodDefinition& definition, const MethodApplication& application) : spec_(specOf(definition)), base_(isSymbolicBase(definition))
    {
        const auto variables = symbolicVariables(definition);
        const std::string key = base_ ? "G" : "G_increment";
        const std::size_t pieces = application.segments.empty() ? 1 : application.segments.size();
        if (!definition.expression_segments.empty() && pieces > definition.expression_segments.size())
            funError("Cannot make the symbolic method " + definition.symbol, "The record has " + std::to_string(pieces) + " pieces, the method " + std::to_string(definition.expression_segments.size()) + ".", __LINE__, __FILE__);
        for (std::size_t piece = 0; piece < pieces; ++piece)
        {
            pieces_.push_back(compile(expressionsOf(definition, piece).at(key), variables, definition.symbol));
            const auto& given = application.segments.empty() ? application.parameters : application.segments[piece].parameters;
            if (!application.segments.empty())
                limits_.emplace_back(application.segments[piece].t_min, application.segments[piece].t_max);
            for (const auto& p : definition.parameter_definitions)
            {
                const auto it = given.find(p.name);
                values_.push_back(it != given.end() ? it->second.value : p.default_value.value_or(0.0));
                names_.push_back(pieces > 1 && piece > 0 ? p.name + "[" + std::to_string(piece) + "]" : p.name);
            }
        }
        per_piece_ = definition.parameter_definitions.size();
    }

    auto spec() const -> const MethodSpec& override { return spec_; }

    auto apply(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void override { needsAPass(spec_.id); }

    auto apply(SubstanceState<TPReal>& state, const SubstanceInputs<TPReal>& in) const -> void override
    {
        // the lower piece at a limit that two pieces share
        std::size_t piece = 0;
        if (!limits_.empty())
        {
            piece = limits_.size();
            for (std::size_t i = 0; i < limits_.size() && piece == limits_.size(); ++i)
                if (in.T >= limits_[i].first && in.T <= limits_[i].second)
                    piece = i;
            if (piece == limits_.size())
                funError("Cannot calculate the symbolic method " + spec_.id, "T = " + std::to_string(in.T.val()) + " K is outside the pieces of the record.", __LINE__, __FILE__);
        }
        std::vector<TPReal> v = {in.T, in.P, TPReal(in.reference.T), TPReal(in.reference.P), TPReal(R_CONSTANT), in.reference.G.value_or(TPReal(0.0)), in.reference.H.value_or(TPReal(0.0)),
                                 in.reference.S.value_or(TPReal(0.0)), in.reference.Cp.value_or(TPReal(0.0)), in.reference.V.value_or(TPReal(0.0))};
        for (std::size_t i = 0; i < per_piece_; ++i)
        {
            const std::size_t at = piece * per_piece_ + i;
            TPReal value = values_[at];
            if (in.seeded_parameter == static_cast<int>(at))
                value[1] = 1.0;
            v.push_back(value);
        }
        const auto& c = *pieces_[piece];
        const TPReal G = c.G.evaluate<TPReal>(v);
        const TPReal S = -c.dT.evaluate<TPReal>(v);
        const TPReal V = c.dP.evaluate<TPReal>(v);
        const TPReal Cp = -in.T * c.dTT.evaluate<TPReal>(v);
        const TPReal H = G + in.T * S;
        if (base_)
        {
            state.G = G; state.H = H; state.S = S; state.Cp = Cp; state.V = V;
        }
        else
        {
            state.G += G; state.H += H; state.S += S; state.Cp += Cp; state.V += V;
        }
    }

    auto parameterValues() const -> std::vector<double> override { return values_; }
    auto parameterNames() const -> std::vector<std::string> override { return names_; }

    auto withParameter(std::size_t index, double value_si) const -> std::shared_ptr<const SubstanceMethod> override
    {
        if (index >= values_.size())
            funError("Cannot change a parameter of the method " + spec_.id, "It has " + std::to_string(values_.size()) + " parameters.", __LINE__, __FILE__);
        auto copy = std::make_shared<SymbolicSubstanceMethod>(*this);
        copy->values_[index] = value_si;
        return copy;
    }

    auto info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&) const -> void override
    {
        if (base_)
            allCalculated(out);
    }

private:
    MethodSpec spec_;
    bool base_;
    std::vector<std::shared_ptr<const Compiled>> pieces_;
    std::vector<std::pair<double, double>> limits_;
    std::vector<double> values_;
    std::vector<std::string> names_;
    std::size_t per_piece_ = 0;
};

} // namespace

auto symbolicVariables(const MethodDefinition& definition) -> std::vector<std::string>
{
    std::vector<std::string> names = {"T", "P", "Tr", "Pr", "R", "Gr", "Hr", "Sr", "Cpr", "Vr"};
    for (const auto& p : definition.parameter_definitions)
        names.push_back(p.name);
    return names;
}

auto isSymbolicBase(const MethodDefinition& definition) -> bool
{
    const auto& first = definition.expression_segments.empty() ? definition.expression : definition.expression_segments.front();
    return first.count("G") > 0;
}

auto isSymbolicSubstanceDefinition(const MethodDefinition& definition) -> bool
{
    if (definition.kind != MethodKind::Symbolic || definition.operates_on != OperatesOn::Substance)
        return false;
    if (definition.expression.empty() && definition.expression_segments.empty())
        return false;
    const auto& first = definition.expression_segments.empty() ? definition.expression : definition.expression_segments.front();
    return first.count("G") > 0 || first.count("G_increment") > 0;
}

namespace {

auto report(Diagnostics& diagnostics, const std::string& code, Severity severity, const MethodDefinition& definition, const std::string& message) -> void
{
    Diagnostic d;
    d.code = code;
    d.severity = severity;
    d.record = definition.symbol;
    d.path = "/methods/" + definition.symbol;
    d.message = message;
    diagnostics.add(d);
}

/// The definition with one piece
auto pieceOf(const MethodDefinition& definition, std::size_t piece) -> MethodDefinition
{
    auto one = definition;
    if (!definition.expression_segments.empty())
    {
        one.expression = definition.expression_segments.at(piece);
        one.expression_segments.clear();
    }
    return one;
}

/// The values in the order of symbolicVariables; values with kg in their units are scaled by mass^power.
auto valuesAt(const MethodDefinition& definition, double mass) -> std::vector<double>
{
    std::vector<double> v = {450.0, 3e7 * mass, 298.15, 1e5 * mass, R_CONSTANT * mass, -1e6 * mass, -1.1e6 * mass, 80.0 * mass, 90.0 * mass, 3e-5};
    for (const auto& p : definition.parameter_definitions)
    {
        const auto powers = ThermoFun::units::dimension(p.unit.empty() ? "1" : p.unit);
        v.push_back(1.5 * std::pow(mass, powers[0]));
    }
    return v;
}

} // namespace

auto checkSymbolicDefinition(const MethodDefinition& definition, Diagnostics& diagnostics) -> void
{
    if (definition.kind != MethodKind::Symbolic)
        return;
    if (!definition.source_hash.empty() && !definition.source_json.empty() && definition.source_hash != "sha256:" + sha256(definition.source_json))
        report(diagnostics, "V2-SYM-003", Severity::Warning, definition, "The source of the method changed after its expression was made. Run: python -m thermofun.method_tool build");
    if (!SymbolicExpression::available())
        return;
    const auto variables = symbolicVariables(definition);
    std::vector<std::map<std::string, std::string>> all = definition.expression_segments;
    if (all.empty())
        all.push_back(definition.expression);
    bool readable = true;
    for (const auto& piece : all)
        for (const auto& [name, text] : piece)
            try
            {
                const auto expression = SymbolicExpression::parse(text, variables);
                // an energy per mole: a change of the unit of mass changes G by the same factor. Units with kelvin stay out of it, as in ln T
                try
                {
                    const double plain = expression.evaluate<double>(valuesAt(definition, 1.0));
                    const double heavier = expression.evaluate<double>(valuesAt(definition, 2.0));
                    if (std::isfinite(plain) && std::isfinite(heavier) && std::fabs(heavier - 2.0 * plain) > 1e-8 * std::fabs(plain) + 1e-300)
                        report(diagnostics, "V2-SYM-005", Severity::Error, definition, "The expression of " + name + " does not give an energy per mole with the units of the parameters.");
                }
                catch (const std::exception&)
                {
                    // a unit that the check does not know
                }
            }
            catch (const SymbolicError& error)
            {
                readable = false;
                report(diagnostics, error.code, Severity::Error, definition, name + ": " + error.what());
            }
    if (!readable || !isSymbolicSubstanceDefinition(definition))
        return;
    for (std::size_t i = 0; i < definition.tests.size(); ++i)
    {
        const auto& test = definition.tests[i];
        if (static_cast<std::size_t>(test.piece) >= all.size())
        {
            report(diagnostics, "V2-SYM-004", Severity::Error, definition, "Test case " + std::to_string(i) + ": the method has no piece " + std::to_string(test.piece) + ".");
            continue;
        }
        const auto one = pieceOf(definition, test.piece);
        const std::string key = isSymbolicBase(one) ? "G" : "G_increment";
        const auto expression = SymbolicExpression::parse(one.expression.at(key), variables);
        const auto referenceOf = [&](const char* name, double otherwise) { const auto it = test.reference.find(name); return it == test.reference.end() ? otherwise : it->second; };
        const auto evaluateAt = [&](double T, double P) {
            std::vector<double> v = {T, P, referenceOf("Tr", 298.15), referenceOf("Pr", 1e5), R_CONSTANT, referenceOf("Gr", 0.0), referenceOf("Hr", 0.0), referenceOf("Sr", 0.0), referenceOf("Cpr", 0.0), referenceOf("Vr", 0.0)};
            for (const auto& p : definition.parameter_definitions)
            {
                const auto it = test.parameters.find(p.name);
                v.push_back(it != test.parameters.end() ? it->second : p.default_value.value_or(0.0));
            }
            return v;
        };
        const auto v = evaluateAt(test.T, test.P);
        const auto dT = expression.diff("T");
        const double G = expression.evaluate<double>(v);
        const double S = -dT.evaluate<double>(v);
        const std::map<std::string, double> got = {{"G", G}, {"S", S}, {"H", G + test.T * S}, {"V", expression.diff("P").evaluate<double>(v)}, {"Cp", -test.T * dT.diff("T").evaluate<double>(v)}};
        for (const auto& [name, want] : test.expected)
            if (const auto it = got.find(name); it != got.end() && std::fabs(it->second - want) > test.relative_tolerance * std::max(std::fabs(want), 1e-30))
                report(diagnostics, "V2-SYM-004", Severity::Error, definition, "Test case " + std::to_string(i) + ": " + name + " is " + std::to_string(it->second) + ", expected " + std::to_string(want) + ".");
    }
}

auto symbolicMethod(const MethodDefinition& definition, const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>
{
    if (!SymbolicExpression::available())
        funError("Cannot run the symbolic method " + definition.symbol, "V2-MTH-008: this build of ThermoFun has no symbolic methods. Build with TFUN_USE_SYMENGINE=ON.", __LINE__, __FILE__);
    return std::make_shared<const SymbolicSubstanceMethod>(definition, application);
}

} // namespace ThermoFun::v2
