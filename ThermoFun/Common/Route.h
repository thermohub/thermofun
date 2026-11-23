// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_ROUTE_H
#define THERMOFUN_COMMON_ROUTE_H

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Common/Notes.h>

namespace ThermoFun {

/// G, H, S, Cp, V of a substance; logK, dG, dH, dS, dCp, dV of a reaction (the words of the format), SI: J/mol, J/(mol K), m3/mol.
/// For a reaction, G holds dG, ... and logK its part of log K; the parts of log K add up like the others.
struct RouteValues
{
    double G = 0.0, H = 0.0, S = 0.0, Cp = 0.0, V = 0.0;
    double logK = 0.0;      ///< a reaction only
    bool reaction = false;  ///< written with the words of a reaction: logK, dG, dH, dS, dCp, dV

    auto operator+=(const RouteValues& o) -> RouteValues& { G += o.G; H += o.H; S += o.S; Cp += o.Cp; V += o.V; logK += o.logK; reaction = reaction || o.reaction; return *this; }
    auto operator-(const RouteValues& o) const -> RouteValues { return {G - o.G, H - o.H, S - o.S, Cp - o.Cp, V - o.V, logK - o.logK, reaction || o.reaction}; }
    auto isZero() const -> bool { return G == 0.0 && H == 0.0 && S == 0.0 && Cp == 0.0 && V == 0.0 && logK == 0.0; }
    auto operator==(const RouteValues&) const -> bool = default;
};

/// A named number of a method: a term (a part of the contribution of the method to one property) or a variable (a quantity of
/// the model that the terms come from, not summed). SI. The names, units and meaning are in the catalog of the method.
struct RouteNumber
{
    std::string name;
    double value = 0.0;
};

/// One method on the route of a record.
struct RouteMethod
{
    std::string method;           ///< the method id, for example "HKF88"
    std::string implementation;   ///< empty: the default implementation
    std::string region;           ///< water and dielectric methods: the region of the method used, as text; empty: everywhere
    RouteValues contribution;     ///< what the method adds to (or replaces in) the properties: the state after it minus the state before it;
                                  ///< G NaN: not calculated (the solvent of a result: only the values that its methods use)
    std::vector<RouteNumber> terms;      ///< of the methods that have them (catalog: terms)
    std::vector<RouteNumber> variables;  ///< the same
};

/// A change after the source of the values: a conversion of convention ("convention": a method in another convention to the
/// canonical one, the canonical one to the conventions of the results), the options of gases ("gas_options"), the reference values
/// used for a value not defined ("fallback"), the rounding to the uncertainty ("rounding").
struct RouteConversion
{
    std::string what;     ///< "convention", "gas_options", "fallback", "rounding"
    std::string detail;   ///< for example "berman_brown", "1atm", "steam_tables", "HKF88 to canonical", "fallback_to_reference_properties"
    RouteValues change;
};

struct Route;

/// Another record that a route needs.
struct RouteNeed
{
    std::string role;            ///< "solvent", "defining_reaction", "reactant"
    double coefficient = 1.0;    ///< in the reaction, for a reactant
    std::shared_ptr<const Route> route;
};

/// How a result was calculated (decision 0030). Every result has one (`result.info.route`), and engine.route(symbol) gives the
/// same without numbers. For each property, the result is the sum of `contribution`, of the contributions of the methods and of
/// the changes of the conversions.
struct Route
{
    std::string symbol;
    std::string kind;     ///< "substance" or "reaction"
    double T = std::numeric_limits<double>::quiet_NaN();  ///< K; NaN in engine.route
    double P = std::numeric_limits<double>::quiet_NaN();  ///< Pa, as used (the saturation pressure for P = 0); NaN in engine.route
    /// Where the values come from: "methods", "reaction" (the defining reaction), "reactants", "water", "reference" (the values
    /// of the record), "hydrogen_ion", "hydrogen_electrode"; empty: nothing gave values
    std::string by;
    RouteValues contribution;            ///< what the source gives, if it is not the methods (by reaction, reactants, reference, hydrogen_electrode)
    std::vector<RouteMethod> methods;
    std::vector<RouteNeed> needs;
    std::vector<RouteConversion> conversions;
    Notes notes;                         ///< engine.route: what cannot be calculated (undefined, hard notes). The notes of a result are in result.info

    /// The sum of the contribution of the source, of the methods and of the conversions: the result
    auto total() const -> RouteValues;
};

/// The route in full as YAML text (numbers with 17 significant digits; T, P only if known)
auto toYaml(const Route& route) -> std::string;

/// One row of routeTable for each point.
struct RouteTable
{
    std::vector<std::string> columns;            ///< "<method>:<name>", "<by>:<property>", "<what>(<detail>):<property>"
    std::vector<double> T, P;                    ///< K, Pa as used
    std::vector<std::vector<double>> values;     ///< values[row][column], SI; NaN where the route of a point has no such column
};

} // namespace ThermoFun

#endif
