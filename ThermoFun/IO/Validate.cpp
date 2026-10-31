// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Validate.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::v2 {

namespace {

// The severity and the fix come from the catalog of diagnostics
void report(Diagnostics& diag, const char* code, const std::string& record, const std::string& path, const std::string& message)
{
    Diagnostic d;
    d.severity = diagnosticInfo(code)->severity;
    d.code = code;
    d.record = record;
    d.path = path;
    d.message = message;
    diag.add(d);
}

void checkRange(Diagnostics& diag, const Applicability& a, const std::string& record, const std::string& path)
{
    if (a.t_min && a.t_max && !(*a.t_min < *a.t_max))
        report(diag, "V2-APP-001", record, path, "t_min is not below t_max.");
    if (a.p_min && a.p_max && !(*a.p_min < *a.p_max))
        report(diag, "V2-APP-001", record, path, "p_min is not below p_max.");
}

// The transformation methods of a substance: at most one, and it runs before the setters and adders whatever the list says
void checkTransformations(Diagnostics& diag, const std::vector<MethodApplication>& methods, const std::string& record, const std::string& path)
{
    bool found = false, afterOther = false;
    for (size_t i = 0; i < methods.size(); ++i)
    {
        const auto* spec = MethodCatalog::instance().find(methods[i].method, OperatesOn::Substance);
        if (!spec || spec->effects.base)
            continue;
        const std::string where = path + "/methods/" + std::to_string(i);
        if (spec->effects.transform)
        {
            if (found)
                report(diag, "V2-MTH-017", record, where, "There is more than one transformation method.");
            if (afterOther)
                report(diag, "V2-MTH-015", record, where, "The transformation " + spec->id + " is listed after other methods. It runs before them.");
            found = true;
        }
        else
            afterOther = true;
    }
}

void checkMethods(Diagnostics& diag, const V2Records& records, const std::vector<MethodApplication>& methods, OperatesOn operatesOn,
                  const std::string& record, const std::string& path)
{
    const auto findSet = [&](const std::string& symbol) -> const MethodParameterSet* {
        for (const auto& set : records.parameter_sets)
            if (set.symbol == symbol)
                return &set;
        return nullptr;
    };
    int index = 0;
    for (const auto& app : methods)
    {
        const std::string where = path + "/methods/" + std::to_string(index++);
        const MethodSpec* spec = MethodCatalog::instance().find(app.method, operatesOn);
        if (!spec)
        {
            const bool defined = std::any_of(records.methods.begin(), records.methods.end(), [&](const auto& m) { return m.symbol == app.method; });
            if (!defined)
                report(diag, "V2-REF-003", record, where, "The method " + app.method + " is not known.");
            continue;
        }
        if (!spec->implemented)
            report(diag, "V2-MTH-008", record, where, "The method " + spec->id + " is known but does nothing.");
        checkRange(diag, app.applicability, record, where + "/applicability");

        const MethodParameterSet* set = nullptr;
        if (!app.parameter_set.empty())
        {
            set = findSet(app.parameter_set);
            if (!set || set->method != app.method)
                report(diag, "V2-REF-004", record, where, "The parameter set " + app.parameter_set + " is not found, or it belongs to another method.");
            if (set)
                for (const auto& p : app.parameters)
                    if (set->parameters.count(p.first))
                        report(diag, "V2-MTH-016", record, where + "/parameters/" + p.first,
                               "The parameter " + p.first + " replaces the value of the set " + app.parameter_set + ".");
        }
        if (spec->id == "LogKTable" && !app.table)
            report(diag, "V2-MTH-004", record, where, "The method " + spec->id + " needs a table of log K.");
        if (spec->id == "TPTable" && !app.table)
            report(diag, "V2-MTH-004", record, where, "The method " + spec->id + " needs a table.");

        const auto has = [&](const std::map<std::string, Quantity>& values, const std::string& name) {
            return values.count(name) > 0 || (set && set->parameters.count(name) > 0);
        };
        if (spec->piecewise)
        {
            for (size_t i = 0; i < app.segments.size(); ++i)
                if (std::isfinite(app.segments[i].t_min) && std::isfinite(app.segments[i].t_max) && !(app.segments[i].t_min < app.segments[i].t_max))
                    report(diag, "V2-APP-001", record, where + "/segments/" + std::to_string(i), "t_min is not below t_max.");
            for (const auto& segment : app.segments)
                for (const auto& p : spec->parameters)
                    if (p.required && !p.default_si && !has(segment.parameters, p.name))
                        report(diag, "V2-MTH-004", record, where, "The parameter " + p.name + " is missing in a segment.");
            for (size_t i = 1; i < app.segments.size(); ++i)
                if (std::isfinite(app.segments[i].t_min) && std::isfinite(app.segments[i - 1].t_max) &&
                    std::fabs(app.segments[i].t_min - app.segments[i - 1].t_max) > 1e-9)
                    report(diag, "V2-MTH-006", record, where, "The segments overlap or leave a gap.");
        }
        else if (!app.table)
            for (const auto& p : spec->parameters)
                if (p.required && !p.default_si && !has(app.parameters, p.name))
                    report(diag, "V2-MTH-004", record, where, "The parameter " + p.name + " is missing.");
    }
}

// True if a path of a covariance block names a value that the record has
template <class Record, class Properties>
auto valueExists(const Record& record, const Properties& properties, const char* section, const std::string& path) -> bool
{
    if (path.rfind(std::string("/") + section + "/", 0) == 0)
    {
        const std::string name = path.substr(std::string(section).size() + 2);
        return std::any_of(properties.begin(), properties.end(), [&](const auto& p) { return toString(p.first) == name; });
    }
    // /methods/<index>/parameters/<name> or /methods/<index>/segments/<index>/parameters/<name>
    if (path.rfind("/methods/", 0) != 0)
        return false;
    const auto next = [&](size_t at) { return path.find('/', at); };
    const size_t afterIndex = next(9);
    if (afterIndex == std::string::npos)
        return false;
    const std::string text = path.substr(9, afterIndex - 9);
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        return false;
    const size_t index = std::stoul(text);
    if (index >= record.methods.size())
        return false;
    const auto& app = record.methods[index];
    const std::string rest = path.substr(afterIndex);
    if (rest.rfind("/parameters/", 0) == 0)
        return app.parameters.count(rest.substr(12)) > 0;
    if (rest.rfind("/segments/", 0) == 0)
    {
        const size_t slash = rest.find('/', 10);
        if (slash == std::string::npos || rest.rfind("/parameters/", slash) != slash)
            return false;
        const std::string number = rest.substr(10, slash - 10);
        if (number.empty() || number.find_first_not_of("0123456789") != std::string::npos)
            return false;
        const size_t segment = std::stoul(number);
        return segment < app.segments.size() && app.segments[segment].parameters.count(rest.substr(slash + 12)) > 0;
    }
    return false;
}

// Positive semi-definite: a Cholesky factorization that tolerates zero pivots, within the rounding of the largest diagonal value
auto positiveSemiDefinite(const std::vector<double>& m, size_t n) -> bool
{
    double largest = 0.0;
    for (size_t i = 0; i < n; ++i)
        largest = std::max(largest, std::fabs(m[i * n + i]));
    const double tolerance = 1e-12 * std::max(largest, 1e-300);
    std::vector<double> l(n * n, 0.0);
    for (size_t j = 0; j < n; ++j)
    {
        double diagonal = m[j * n + j];
        for (size_t k = 0; k < j; ++k)
            diagonal -= l[j * n + k] * l[j * n + k];
        if (diagonal < -tolerance)
            return false;
        const double root = diagonal > tolerance ? std::sqrt(diagonal) : 0.0;
        l[j * n + j] = root;
        for (size_t i = j + 1; i < n; ++i)
        {
            double value = m[i * n + j];
            for (size_t k = 0; k < j; ++k)
                value -= l[i * n + k] * l[j * n + k];
            if (root > 0.0)
                l[i * n + j] = value / root;
            else if (std::fabs(value) > std::sqrt(tolerance * std::max(largest, 1e-300)))
                return false;
        }
    }
    return true;
}

template <class Record, class Properties>
void checkCovariances(Diagnostics& diag, const Record& record, const Properties& properties, const char* section, const std::string& path)
{
    int index = 0;
    for (const auto& block : record.covariance)
    {
        const std::string where = path + "/covariances/" + std::to_string(index++);
        const size_t n = block.paths.size();
        if (block.matrix.size() != n * n)
        {
            report(diag, "V2-COV-003", record.symbol, where, "The matrix has " + std::to_string(block.matrix.size()) + " values for " + std::to_string(n) + " paths.");
            continue;
        }
        for (const auto& value : block.paths)
            if (!valueExists(record, properties, section, value))
                report(diag, "V2-COV-004", record.symbol, where, "The record has no value at " + value + ".");
        bool symmetric = true;
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < i; ++j)
            {
                const double a = block.matrix[i * n + j], b = block.matrix[j * n + i];
                symmetric = symmetric && std::fabs(a - b) <= 1e-12 * std::max(std::fabs(a), std::fabs(b));
            }
        if (!symmetric)
            report(diag, "V2-COV-001", record.symbol, where, "The matrix is not symmetric.");
        else if (!positiveSemiDefinite(block.matrix, n))
            report(diag, "V2-COV-002", record.symbol, where, "The matrix is not positive semi-definite.");
    }
}

} // namespace

auto validateRecords(const V2Records& records, const FormulaParser& parseFormula) -> Diagnostics
{
    Diagnostics diag;
    std::map<std::string, const SubstanceRecord*> substances;
    std::map<std::string, const ReactionRecord*> reactions;
    for (const auto& r : records.substances)
        substances[r.symbol] = &r;
    for (const auto& r : records.reactions)
        reactions[r.symbol] = &r;
    std::set<std::string> sources;
    for (const auto& d : records.data_sources)
        sources.insert(d.symbol);

    if (records.dataset.status == DatasetStatus::Published && records.dataset.version.empty())
        report(diag, "V2-DS-001", records.dataset.symbol, "/thermodataset", "A published dataset needs a version.");

    for (const auto& r : records.substances)
    {
        const std::string path = "/substances/" + r.symbol;
        checkRange(diag, r.applicability, r.symbol, path + "/applicability");
        checkMethods(diag, records, r.methods, OperatesOn::Substance, r.symbol, path);
        checkTransformations(diag, r.methods, r.symbol, path);
        checkCovariances(diag, r, r.standard_state_properties, "standard_state_properties", path);
        if (!r.defining_reaction.empty())
        {
            const auto it = reactions.find(r.defining_reaction);
            if (it == reactions.end())
                report(diag, "V2-REF-001", r.symbol, path + "/defining_reaction", "The reaction " + r.defining_reaction + " is not in the database.");
            else if (std::none_of(it->second->stoichiometry.begin(), it->second->stoichiometry.end(), [&](const auto& s) { return s.first == r.symbol; }))
                report(diag, "V2-REF-001", r.symbol, path + "/defining_reaction", "The reaction " + r.defining_reaction + " does not name the substance.");
        }
        if (!r.data_source.empty() && !sources.empty() && !sources.count(r.data_source))
            report(diag, "V2-REF-005", r.symbol, path + "/data_source", "The data source " + r.data_source + " is not in the database.");
    }

    // cycles through the defining reactions
    std::map<std::string, int> state;   // 1 on the path, 2 done
    std::vector<std::string> trail;
    std::set<std::string> reported;
    const std::function<void(const std::string&)> visit = [&](const std::string& symbol) {
        const auto substance = substances.find(symbol);
        if (substance == substances.end() || state[symbol] == 2)
            return;
        if (state[symbol] == 1)
        {
            std::string cycle;
            for (auto it = std::find(trail.begin(), trail.end(), symbol); it != trail.end(); ++it)
                cycle += *it + " -> ";
            if (reported.insert(symbol).second)
                report(diag, "V2-REF-010", symbol, "/substances/" + symbol + "/defining_reaction", "Cycle of defining reactions: " + cycle + symbol);
            return;
        }
        state[symbol] = 1;
        trail.push_back(symbol);
        // only a substance without methods takes its properties from the reaction
        const auto reaction = reactions.find(substance->second->defining_reaction);
        if (reaction != reactions.end() && substance->second->methods.empty())
            for (const auto& s : reaction->second->stoichiometry)
                if (s.first != symbol)
                    visit(s.first);
        trail.pop_back();
        state[symbol] = 2;
    };
    for (const auto& r : records.substances)
        visit(r.symbol);

    for (const auto& r : records.reactions)
    {
        const std::string path = "/reactions/" + r.symbol;
        checkRange(diag, r.applicability, r.symbol, path + "/applicability");
        checkMethods(diag, records, r.methods, OperatesOn::Reaction, r.symbol, path);
        checkCovariances(diag, r, r.reaction_properties, "reaction_properties", path);
        std::map<std::string, double> balance;
        bool complete = !r.stoichiometry.empty();
        for (const auto& s : r.stoichiometry)
        {
            const auto substance = substances.find(s.first);
            if (substance == substances.end())
            {
                report(diag, "V2-REF-002", r.symbol, path + "/stoichiometry", "The substance " + s.first + " is not in the database.");
                complete = false;
                continue;
            }
            try
            {
                for (const auto& element : parseFormula(substance->second->formula))
                    balance[element.first] += s.second * element.second;
            }
            catch (const std::exception&)
            {
                complete = false;
            }
        }
        if (complete)
            for (const auto& b : balance)
                if (std::fabs(b.second) > 1e-6)
                {
                    report(diag, "V2-RXN-002", r.symbol, path + "/stoichiometry", "The reaction is not balanced in " + b.first + " (" + std::to_string(b.second) + ").");
                    break;
                }
        if (!r.data_source.empty() && !sources.empty() && !sources.count(r.data_source))
            report(diag, "V2-REF-005", r.symbol, path + "/data_source", "The data source " + r.data_source + " is not in the database.");
    }
    return diag;
}

} // namespace ThermoFun::v2
