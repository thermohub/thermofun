// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodCatalog.h"

#include <ThermoFun/Methods/Water/WaterZhangDuan2005.h>

#include <algorithm>
#include <map>
#include <cctype>

namespace ThermoFun::v2 {

namespace {

using E = Effect;
using SC = SubstanceClass;

auto par(const char* name, const char* si, const char* source, const char* description, bool required = true) -> ParameterSpec
{
    ParameterSpec p;
    p.name = name;
    p.si_unit = si;
    p.source_unit = source;
    p.description = description;
    p.required = required;
    return p;
}

// Builds one specification. The functions return the builder, so the entries below read as one sentence.
struct Spec
{
    MethodSpec s;

    Spec(const char* id, const char* title, const char* summary, const char* citation = "")
    {
        s.id = id;
        s.title = title;
        s.summary = summary;
        s.citation = citation;
    }

    auto alias(std::vector<std::string> names) -> Spec& { s.aliases = std::move(names); return *this; }
    auto classes(std::vector<SC> c) -> Spec& { s.applies_to_classes = std::move(c); return *this; }
    auto role(MethodRole r) -> Spec& { s.role = r; return *this; }
    auto impl(std::vector<std::string> i) -> Spec& { s.implementations = std::move(i); return *this; }
    auto solvent(bool atReference = false) -> Spec&
    {
        s.needs.solvent = true;
        s.needs.solvent_at_reference = atReference;
        return *this;
    }
    auto dielectric() -> Spec& { s.needs.dielectric = true; return *this; }
    auto piecewise() -> Spec& { s.piecewise = true; return *this; }
    auto convention(EnergyReference reference) -> Spec& { s.native_convention = reference; return *this; }
    auto notImplemented() -> Spec& { s.implemented = false; return *this; }
    auto add(ParameterSpec p) -> Spec& { s.parameters.push_back(std::move(p)); return *this; }
    auto add(std::vector<ParameterSpec> ps) -> Spec&
    {
        for (auto& p : ps)
            s.parameters.push_back(std::move(p));
        return *this;
    }

    // A base method of a substance: it sets all properties.
    auto base() -> Spec&
    {
        s.effects = {E::Set, E::Set, E::Set, E::Set, E::Set, true, false, 0};
        return *this;
    }
    auto effects(E g, E h, E sEntropy, E cp, E v, int rank, bool transform = false) -> Spec&
    {
        s.effects = {g, h, sEntropy, cp, v, false, transform, rank};
        return *this;
    }

    // A reaction method for log K: it sets logK, dG, dH, dS and dCp.
    auto logK(bool final = false) -> Spec&
    {
        reaction();
        s.reaction_effects = {E::Set, E::Set, E::Set, E::Set, E::Set, E::None, true, final, 0};
        return *this;
    }
    // A reaction method for the volume of reaction: it adds to logK, dG and dH and sets dV.
    auto volume() -> Spec&
    {
        reaction();
        s.reaction_effects = {E::Add, E::Add, E::Add, E::None, E::None, E::Set, false, false, 200};
        return *this;
    }
    auto reaction() -> Spec&
    {
        s.operates_on = OperatesOn::Reaction;
        s.role = MethodRole::ReactionProperties;
        return *this;
    }
};

const std::vector<SC> minerals = {SC::Component};
const std::vector<SC> fluids = {SC::GasFluid};
const std::vector<SC> solutes = {SC::AqueousSolute};
const std::vector<SC> water = {SC::WaterSolvent};

auto hkfParameters() -> std::vector<ParameterSpec>
{
    return {par("a1", "J/(mol.Pa)", "cal/(mol.bar)", "pressure coefficient a1"),
            par("a2", "J/mol", "cal/mol", "pressure coefficient a2"),
            par("a3", "J.K/(mol.Pa)", "cal.K/(mol.bar)", "pressure coefficient a3"),
            par("a4", "J.K/mol", "cal.K/mol", "pressure coefficient a4"),
            par("c1", "J/(mol.K)", "cal/(mol.K)", "temperature coefficient c1"),
            par("c2", "J.K/mol", "cal.K/mol", "temperature coefficient c2"),
            par("omega", "J/mol", "cal/mol", "conventional Born coefficient")};
}

auto expansionAndBulkModulus(bool required) -> std::vector<ParameterSpec>
{
    return {par("alpha0", "1/K", "1/K", "thermal expansion at the reference state", required),
            par("kappa0", "Pa", "kbar", "bulk modulus at 298 K", required)};
}

auto volumeParameters() -> std::vector<ParameterSpec>
{
    return {par("a1", "1/K", "1/K", "thermal expansion, first coefficient"),
            par("a2", "1/K2", "1/K2", "thermal expansion, second coefficient"),
            par("a3", "1/K3", "1/K3", "thermal expansion, third coefficient"),
            par("b1", "1/Pa", "1/bar", "compressibility, first coefficient"),
            par("b2", "1/Pa2", "1/bar2", "compressibility, second coefficient")};
}

auto criticalData() -> std::vector<ParameterSpec>
{
    return {par("Tc", "K", "K", "critical temperature"), par("Pc", "Pa", "bar", "critical pressure"),
            par("omega", "1", "1", "acentric factor")};
}

auto cubicParameters() -> std::vector<ParameterSpec>
{
    auto out = criticalData();
    out.push_back(par("N", "1", "1", "empirical parameter of the attraction term", false));
    return out;
}

auto interactionParameters() -> std::vector<ParameterSpec>
{
    auto out = criticalData();
    out.push_back(par("k1", "1", "1", "interaction parameter k1", false));
    out.push_back(par("k2", "1", "1", "interaction parameter k2", false));
    out.push_back(par("k3", "1", "1", "interaction parameter k3", false));
    return out;
}

// log K = a0 + a1 T + a2/T + a3 ln T + a4/T2 + a5 T2 + a6/sqrt(T)
auto logKCoefficients(bool required) -> std::vector<ParameterSpec>
{
    return {par("a0", "1", "1", "constant term", required), par("a1", "1/K", "1/K", "term a1 T", false),
            par("a2", "K", "K", "term a2 / T", false), par("a3", "1", "1", "term a3 ln T", false),
            par("a4", "K2", "K2", "term a4 / T2", false), par("a5", "1/K2", "1/K2", "term a5 T2", false),
            par("a6", "K^0.5", "K^0.5", "term a6 / sqrt(T)", false)};
}

// dCp = a + b T + c / T2 + d / sqrt(T) + e T2
auto reactionCpCoefficients(bool required) -> std::vector<ParameterSpec>
{
    return {par("a", "J/(mol.K)", "J/(mol.K)", "constant term", required), par("b", "J/(mol.K2)", "J/(mol.K2)", "term b T", false),
            par("c", "J.K/mol", "J.K/mol", "term c / T2", false), par("d", "J/(mol.K^0.5)", "J/(mol.K^0.5)", "term d / sqrt(T)", false),
            par("e", "J/(mol.K3)", "J/(mol.K3)", "term e T2", false)};
}

/// The constants of a model: built-in, so a record does not have to give them (see Methods/ParameterSets).
auto constants(std::vector<std::pair<const char*, const char*>> rows) -> std::vector<ParameterSpec>
{
    std::vector<ParameterSpec> out;
    for (const auto& [name, description] : rows)
    {
        out.push_back(par(name, "1", "1", description, false));
        out.back().constant = true;
    }
    return out;
}

auto numbered(const char* prefix, int count, const char* description) -> std::vector<ParameterSpec>
{
    std::vector<ParameterSpec> out;
    for (int i = 1; i <= count; ++i)
    {
        out.push_back(par((prefix + std::to_string(i)).c_str(), "1", "1", description, false));
        out.back().constant = true;
    }
    return out;
}

/// name_1, name_2, ...: an array of the formulas, built in
auto listed(const std::string& name, int count) -> std::vector<ParameterSpec>
{
    return numbered((name + "_").c_str(), count, "see the file of the method in Methods/ParameterSets");
}

/// Constants named one by one, explained by the description of their file.
auto builtIn(std::initializer_list<const char*> names) -> std::vector<ParameterSpec>
{
    std::vector<std::pair<const char*, const char*>> rows;
    for (const auto* name : names)
        rows.emplace_back(name, "see the file of the method in Methods/ParameterSets");
    return constants(rows);
}

/// What the Reaktoro implementations of water share, in WaterSaturation.yaml, and their T of the triple point
auto saturation() -> std::vector<ParameterSpec>
{
    auto out = numbered("sat_a", 6, "of the saturation pressure");
    for (auto& c : numbered("sat_b", 6, "of the density of the saturated liquid"))
        out.push_back(c);
    for (auto& c : numbered("sat_c", 6, "of the density of the saturated vapour"))
        out.push_back(c);
    for (auto& c : constants({{"Tcr", "critical temperature, K"}, {"Pcr", "critical pressure, Pa"}, {"Dcr", "critical density, kg/m3"}, {"M", "molar mass, kg/mol"}, {"Ttr", "T of the triple point of the model, K"}}))
        out.push_back(c);
    return out;
}

auto substanceMethods() -> std::vector<MethodSpec>
{
    std::vector<Spec> m;
    m.push_back(Spec("ConstantProperties", "Constant properties", "Gives the reference properties at every T and P.").base());

    // heat capacity
    {
        Spec s("CpPolynomial", "Heat capacity polynomial",
               "Integrates Cp = a0 + a1 T + a2/T2 + a3/sqrt(T) + a4 T2 + a5 T3 + a6 T4 + a7/T3 + a8/T + a9 sqrt(T) over T segments, with phase transitions.");
        s.alias({"CpIntegration"}).base().piecewise();
        s.add({par("a0", "J/(mol.K)", "J/(mol.K)", "constant term"), par("a1", "J/(mol.K2)", "J/(mol.K2)", "term a1 T", false),
               par("a2", "J.K/mol", "J.K/mol", "term a2 / T2", false), par("a3", "J/(mol.K^0.5)", "J/(mol.K^0.5)", "term a3 / sqrt(T)", false),
               par("a4", "J/(mol.K3)", "J/(mol.K3)", "term a4 T2", false), par("a5", "J/(mol.K4)", "J/(mol.K4)", "term a5 T3", false),
               par("a6", "J/(mol.K5)", "J/(mol.K5)", "term a6 T4", false), par("a7", "J.K2/mol", "J.K2/mol", "term a7 / T3", false),
               par("a8", "J/mol", "J/mol", "term a8 / T", false), par("a9", "J/(mol.K^1.5)", "J/(mol.K^1.5)", "term a9 sqrt(T)", false)});
        m.push_back(s);
    }
    m.push_back(Spec("CpFeiSaxena86", "Heat capacity of Fei and Saxena", "Heat capacity equation of Fei and Saxena.", "Fei and Saxena (1986)")
                    .alias({"CpSaxena86"}).base().classes(minerals).notImplemented());
    m.push_back(Spec("ConstantCp", "Constant heat capacity", "Integrates the constant reference Cp from the reference S, H and G.")
                    .alias({"EntropyCpIntegration"}).base());
    {
        Spec s("CpExtendedUNIQUAC", "Heat capacity of Extended UNIQUAC", "Integrates Cp = a + b T + c / (T - Theta) and adds a pressure term.", "Thomsen and Rasmussen (1999)");
        s.alias({"ExtendedUNIQUAC", "StandardThermoModelExtendedUNIQUAC"}).base().classes(solutes);
        s.convention(EnergyReference::GibbsHelmholtzFormation);
        s.add({par("a", "J/(mol.K)", "J/(mol.K)", "constant term"), par("b", "J/(mol.K2)", "J/(mol.K2)", "term b T"),
               par("c", "J/mol", "J/mol", "term c / (T - Theta)"), par("alpha", "1/Pa", "1/bar", "pressure term, first coefficient"),
               par("beta", "1/Pa2", "1/bar2", "pressure term, second coefficient")});
        auto theta = par("Theta", "K", "K", "temperature of the pole of the heat capacity", false);
        theta.default_si = 200.0;
        s.add(theta);
        m.push_back(s);
    }
    {
        Spec s("NASA9", "NASA polynomials", "Calculates the properties from NASA 9-coefficient polynomials, by T segment.", "McBride, Zehe and Gordon (2002)");
        s.alias({"StandardThermoModelNasa"}).base().piecewise();
        s.convention(EnergyReference::AssignedEnthalpy);
        for (const char* n : {"a1", "a2", "a3", "a4", "a5", "a6", "a7"})
            s.add(par(n, "1", "1", "coefficient of Cp/R"));
        s.add(par("b1", "K", "K", "integration constant of H"));
        s.add(par("b2", "1", "1", "integration constant of S"));
        m.push_back(s);
    }
    m.push_back(Spec("TPTable", "Table of T and P", "Interpolates a table of G, H, V and Cp on a grid of T and P.")
                    .alias({"TPTableInterpolation", "StandardThermoModelInterpolation"}).base());

    // aqueous solutes
    m.push_back(Spec("HKF88", "Revised HKF equation of state", "Calculates the properties of an aqueous species from the revised Helgeson-Kirkham-Flowers equation of state.",
                     "Tanger and Helgeson (1988); Shock et al. (1992)")
                    .alias({"HKFReaktoro", "HKFGems"}).base().classes(solutes).solvent().dielectric().impl({"reaktoro", "gems"}).add(hkfParameters())
                    .add(constants({{"eta", "constant of the Born term, A cal/mol"}, {"theta", "characteristic T of the solvent, K"}, {"psi", "characteristic P of the solvent, bar"},
                                    {"Tr", "reference T, K"}, {"Pr", "reference P, bar"}, {"Zr", "Born function Z at the reference"}, {"Yr", "Born function Y at the reference"},
                                    {"zeta", "constant of the effective radius, A"}, {"gr", "g function at the reference"},
                                    {"ag1", "coefficient of a_g"}, {"ag2", "coefficient of a_g"}, {"ag3", "coefficient of a_g"}, {"bg1", "coefficient of b_g"}, {"bg2", "coefficient of b_g"}, {"bg3", "coefficient of b_g"},
                                    {"af1", "coefficient of f(T)"}, {"af2", "coefficient of f(P)"}, {"af3", "coefficient of f(P)"}})));
    {
        Spec s("AkinfievDiamond03", "Model of Akinfiev and Diamond", "Calculates the properties of a dissolved gas or neutral species from those of the gas and of water.",
               "Akinfiev and Diamond (2003)");
        s.effects(E::Read, E::Read, E::Read, E::Read, E::Set, 300, true).classes(solutes).solvent(true).impl({"gems"});
        s.s.needs.ideal_gas_water = true;
        s.add({par("xi", "1", "1", "parameter xi"), par("a", "m3/kg", "cm3/g", "parameter a"),
               par("b", "m3.K^0.5/kg", "cm3.K^0.5/g", "parameter b")});
        s.add(constants({{"MW", "molar mass of water of the model, g/mol"}}));
        m.push_back(s);
    }
    m.push_back(Spec("SoluteHP98", "Aqueous solute of Holland and Powell", "Calculates the properties of an aqueous species from the density model of Holland and Powell.",
                     "Holland and Powell (1998)")
                    .alias({"HP98Sol"}).base().classes(solutes).solvent(true)
                    .add(par("b", "J/(mol.K2)", "kJ/(mol.K2)", "coefficient b")));
    m.push_back(Spec("AndersonEtAl91", "Aqueous solute of Anderson et al.", "Calculates the properties of an aqueous species from the density model of Anderson et al.",
                     "Anderson et al. (1991)")
                    .alias({"Anderson91"}).base().classes(solutes).solvent(true));

    // minerals
    m.push_back(Spec("LandauHP98", "Landau transition of Holland and Powell", "Adds the Landau term of a phase transition to G, H, S and Cp, and sets V.", "Holland and Powell (1998)")
                    .alias({"HP98Landau"}).effects(E::Add, E::Add, E::Add, E::Add, E::Set, 100).classes(minerals)
                    .add({par("Tc0", "K", "Cel", "critical temperature at 1 bar"), par("Smax", "J/(mol.K)", "J/(mol.K)", "maximum entropy of the transition"),
                          par("Vmax", "m3/mol", "J/(mol.bar)", "maximum volume of the transition")})
                    .add(expansionAndBulkModulus(false)));
    m.push_back(Spec("LandauHP11", "Landau transition of Holland and Powell (2011)", "Adds the Landau term of a phase transition to G, H, S, Cp and V, with the order parameter at the reference state.", "Holland and Powell (2011)")
                    .alias({"HP11Landau"}).effects(E::Add, E::Add, E::Add, E::Add, E::Add, 150).classes(minerals)
                    .add({par("Tc0", "K", "Cel", "critical temperature at the reference pressure"), par("Smax", "J/(mol.K)", "J/(mol.K)", "maximum entropy of the transition"),
                          par("Vmax", "m3/mol", "J/(mol.bar)", "maximum volume of the transition")}));
    m.push_back(Spec("LambdaBerman88", "Lambda transition of Berman", "Adds the lambda transition of Berman.", "Berman (1988)")
                    .alias({"Berman88Landau"}).effects(E::Add, E::Add, E::Add, E::Add, E::None, 100).classes(minerals).notImplemented());
    m.push_back(Spec("ConstantVolume", "Constant molar volume", "Adds V dP to G and H, with the reference volume.")
                    .alias({"ConMolVol"}).effects(E::Add, E::Add, E::None, E::None, E::Set, 200));
    m.push_back(Spec("VolumeBerman88", "Molar volume of Berman", "Calculates V(T, P) from expansion and compressibility terms, and adds the terms to G, H and S.", "Berman (1988)")
                    .alias({"Berman88Vol"}).effects(E::Add, E::Add, E::Add, E::None, E::Set, 200).classes(minerals).add(volumeParameters()));
    m.push_back(Spec("VolumeDorogokupets88", "Molar volume of Dorogokupets et al.", "Calculates V(T, P) as VolumeBerman88, with another form of the coefficients.", "Dorogokupets et al. (1988)")
                    .alias({"Dorogokupets88Vol"}).effects(E::Add, E::Add, E::Add, E::None, E::Set, 200).classes(minerals).add(volumeParameters()));
    m.push_back(Spec("BirchMurnaghan47", "Birch-Murnaghan equation of state", "Calculates V(T, P) from the third-order Birch-Murnaghan equation, with alpha(T) = a1 + a2 T + a3/T2 and K(T) linear in T.",
                     "Birch (1947); code of M. Gottschalk")
                    .alias({"BirchMurnaghanGottschalk97", "BirchMurnaghanGott97"}).effects(E::Add, E::Add, E::Add, E::None, E::Add, 200).classes(minerals).impl({"gems"})
                    .add({par("a1", "1/K", "1/K", "thermal expansion, constant term"), par("a2", "1/K2", "1/K2", "thermal expansion, term a2 T", false),
                          par("a3", "K", "K", "thermal expansion, term a3 / T2", false), par("kt00", "Pa", "MPa", "bulk modulus at the reference temperature"),
                          par("dkdt", "Pa/K", "MPa/K", "temperature derivative of the bulk modulus", false), par("kp", "1", "1", "pressure derivative of the bulk modulus"),
                          par("kpp", "1/Pa", "1/MPa", "second pressure derivative of the bulk modulus; 0: computed from kp", false)}));
    m.push_back(Spec("MurnaghanHP98", "Murnaghan equation of Holland and Powell", "Calculates V(T, P) from the Murnaghan equation, and adds the terms to G, H, S and Cp.", "Holland and Powell (1998)")
                    .alias({"HP98Min"}).effects(E::Add, E::Add, E::Add, E::Add, E::Add, 200).classes(minerals).add(expansionAndBulkModulus(true)));
    {
        Spec s("TaitHP11", "Tait equation of Holland and Powell", "Calculates V(T, P) from the modified Tait equation with a thermal pressure, and adds the terms to G, H, S and Cp.",
               "Holland and Powell (2011)");
        s.alias({"HP11Tait"}).effects(E::Add, E::Add, E::Add, E::Add, E::Set, 200).classes(minerals);
        s.add({par("kappa0p", "1", "1", "pressure derivative of the bulk modulus"), par("kappa0pp", "1/Pa", "1/kbar", "second pressure derivative of the bulk modulus")});
        auto atoms = par("n_atoms", "1", "1", "number of atoms in the formula");
        atoms.fittable = false;
        s.add(atoms);
        s.add(expansionAndBulkModulus(true));
        m.push_back(s);
    }

    // gases and fluids
    m.push_back(Spec("IdealGas", "Ideal gas", "Sets V = RT/P, adds RT ln(P/P0) to G and subtracts R ln(P/P0) from S.")
                    .alias({"IdealGasVol"}).effects(E::Add, E::None, E::Add, E::None, E::Set, 200).classes({SC::GasFluid, SC::Component}));
    auto fluid = [](Spec s) -> Spec { s.effects(E::Add, E::Add, E::Add, E::None, E::Set, 200).classes(fluids); return s; };
    m.push_back(fluid(Spec("PRSV86", "Peng-Robinson-Stryjek-Vera equation of state", "Adds the pressure terms of the PRSV equation of state to G, H and S, and sets V.",
                           "Stryjek and Vera (1986); Proust and Vera (1989)")
                          .alias({"PRSV"}).add(interactionParameters()).add(builtIn({"k0_0", "k0_1", "k0_2", "k0_3", "Oa", "Ob"}))));
    {
        Spec s("ChurakovGottschalk03", "Equation of state of Churakov and Gottschalk", "Adds the pressure terms of the perturbation equation of state to G, H and S, and sets V.",
               "Churakov and Gottschalk (2003a, b)");
        s.alias({"CGF"});
        // the coefficients stay as published: their units are in the descriptions
        for (const auto& [n, d] : std::vector<std::pair<const char*, const char*>>{
                 {"sigma0", "sigma = sigma0 + sigma1 exp(sigma2 T), angstrom"}, {"eps0", "epsilon = eps0 + eps1 exp(eps2 T), K"},
                 {"mu0", "dipole = mu0 + mu1 / (T + mu2), debye"}, {"pol0", "polarizability = pol0 + pol1 / (T + pol2), angstrom3"},
                 {"sigma1", "see sigma0, angstrom"}, {"sigma2", "see sigma0, 1/K"}, {"eps1", "see eps0, K"}, {"eps2", "see eps0, 1/K"},
                 {"mu1", "see mu0, debye K"}, {"mu2", "see mu0, K"}, {"pol1", "see pol0, angstrom3 K"}, {"pol2", "see pol0, K"}})
            s.add(par(n, "1", "1", d));
        m.push_back(fluid(s));
    }
    m.push_back(fluid(Spec("SoaveRedlichKwong72", "Soave-Redlich-Kwong equation of state", "Adds the pressure terms of the SRK equation of state to G, H and S, and sets V.", "Soave (1972); Soave (1993)")
                          .alias({"SRK"}).add(cubicParameters()).add(builtIn({"m0", "m1", "m2", "Oa", "Ob"}))));
    m.push_back(fluid(Spec("SternerPitzer94", "Equation of state of Sterner and Pitzer", "Adds the pressure terms of the Sterner-Pitzer equation of state to G, H and S, and sets V.", "Sterner and Pitzer (1994)")
                          .alias({"SternerPitzer"}).add({par("Tc", "K", "K", "critical temperature"), par("Pc", "Pa", "bar", "critical pressure")}).add(builtIn({"TMIN", "TMAX", "PMIN", "PMAX", "cfh_1", "cfh_2", "cfh_3", "cfh_4", "cfh_5", "cfh_6", "cfh_7", "cfh_8", "cfh_9", "cfh_10", "cfh_11", "cfh_12", "cfh_13", "cfh_14", "cfh_15", "cfh_16", "cfh_17", "cfh_18", "cfh_19", "cfh_20", "cfh_21", "cfh_22", "cfh_23", "cfh_24", "cfh_25", "cfh_26", "cfh_27", "cfh_28", "cfh_29", "cfh_30", "cfh_31", "cfh_32", "cfh_33", "cfh_34", "cfh_35", "cfh_36", "cfh_37", "cfh_38", "cfh_39", "cfh_40", "cfh_41", "cfh_42", "cfh_43", "cfh_44", "cfh_45", "cfh_46", "cfh_47", "cfh_48", "cfh_49", "cfh_50", "cfh_51", "cfh_52", "cfh_53", "cfh_54", "cfh_55", "cfh_56", "cfh_57", "cfh_58", "cfh_59", "cfh_60", "cfc_1", "cfc_2", "cfc_3", "cfc_4", "cfc_5", "cfc_6", "cfc_7", "cfc_8", "cfc_9", "cfc_10", "cfc_11", "cfc_12", "cfc_13", "cfc_14", "cfc_15", "cfc_16", "cfc_17", "cfc_18", "cfc_19", "cfc_20", "cfc_21", "cfc_22", "cfc_23", "cfc_24", "cfc_25", "cfc_26", "cfc_27", "cfc_28", "cfc_29", "cfc_30", "cfc_31", "cfc_32", "cfc_33", "cfc_34", "cfc_35", "cfc_36", "cfc_37", "cfc_38", "cfc_39", "cfc_40", "cfc_41", "cfc_42", "cfc_43", "cfc_44", "cfc_45", "cfc_46", "cfc_47", "cfc_48", "cfc_49", "cfc_50", "cfc_51", "cfc_52", "cfc_53", "cfc_54", "cfc_55", "cfc_56", "cfc_57", "cfc_58", "cfc_59", "cfc_60", "cs_a0", "cs_a1", "cs_b0", "cs_c0", "cs_c1", "cs_d0", "cs_d1", "psat_co2_Rhoc", "psat_co2_a1", "psat_co2_a2", "psat_co2_a3", "psat_co2_a4", "psat_co2_b1", "psat_co2_b2", "psat_co2_b3", "psat_co2_b4", "psat_co2_c1", "psat_co2_c2", "psat_co2_c3", "psat_co2_c4", "psat_co2_c5", "psat_h2o_Rhoc", "psat_h2o_a1", "psat_h2o_a2", "psat_h2o_a3", "psat_h2o_a4", "psat_h2o_a5", "psat_h2o_a6", "psat_h2o_b1", "psat_h2o_b2", "psat_h2o_b3", "psat_h2o_b4", "psat_h2o_b5", "psat_h2o_b6", "psat_h2o_c1", "psat_h2o_c2", "psat_h2o_c3", "psat_h2o_c4", "psat_h2o_c5", "psat_h2o_c6", "Mw_h2o", "Mw_co2"}))));
    m.push_back(fluid(Spec("PengRobinson78", "Peng-Robinson equation of state", "Adds the pressure terms of the Peng-Robinson equation of state (1978 form) to G, H and S, and sets V.", "Peng and Robinson (1976, 1978)")
                          .alias({"PR78"}).add(cubicParameters()).add(builtIn({"k1_0", "k1_1", "k1_2", "k2_0", "k2_1", "k2_2", "k2_3", "omega_switch", "Oa", "Ob"}))));
    m.push_back(fluid(Spec("CorkHP91", "Compensated Redlich-Kwong equation of state", "Adds the pressure terms of the CORK equation of state to G, H and S, and sets V.", "Holland and Powell (1991)")
                          .alias({"CORK"}).add(interactionParameters()).add(builtIn({"cs_a0", "cs_a1", "cs_b0", "cs_c0", "cs_c1", "cs_d0", "cs_d1", "co2_a0", "co2_a1", "co2_a2", "co2_b", "co2_c0", "co2_c1", "co2_d0", "co2_d1", "co2_e0", "co2_p0", "h2o_a0", "h2o_a1", "h2o_a2", "h2o_a3", "h2o_a4", "h2o_a5", "h2o_a6", "h2o_a7", "h2o_a8", "h2o_a9", "h2o_b", "h2o_c0", "h2o_c1", "h2o_d0", "h2o_d1", "h2o_e0", "h2o_p0", "h2o_psat0", "h2o_psat2", "h2o_psat3", "h2o_psat5", "h2o_T0", "h2o_Tsuper"}))));

    // water
    // the option of the model DEW: the Gibbs energy of water from the volume of Zhang and Duan
    auto zhangDuanGibbs = [] {
        std::vector<ParameterSpec> out;
        auto dew = par("dew_gibbs", "1", "1", "1: the Gibbs energy of the model DEW (P >= 1 kbar)", false);
        dew.default_si = 0.0;
        dew.fittable = false;
        out.push_back(dew);
        for (auto& c : numbered("a", 14, "coefficient of the equation of state"))
            out.push_back(c);
        for (auto& c : constants({{"gamma", "coefficient of the exponential term"}, {"Tc", "critical temperature, K"}, {"Vc", "critical volume, cm3/mol"}, {"R", "gas constant, cm3 bar/(K mol)"},
                                  {"g1kb_0", "coefficient of G(T, 1 kbar) of DEW, cal/mol, the highest power of T (degC) first"}, {"g1kb_1", "coefficient of G(T, 1 kbar) of DEW"},
                                  {"g1kb_2", "coefficient of G(T, 1 kbar) of DEW"}, {"g1kb_3", "coefficient of G(T, 1 kbar) of DEW"}, {"g1kb_4", "coefficient of G(T, 1 kbar) of DEW"}}))
            out.push_back(c);
        return out;
    };
    auto waterEoS = [](Spec s) -> Spec
    {
        auto state = par("state", "1", "1", "0: liquid, 1: gas", false);
        state.default_si = 0.0;
        state.fittable = false;
        s.base().role(MethodRole::SolventProperties).add(state);
        return s;
    };
    m.push_back(waterEoS(Spec("WaterHGK84", "Water, Haar-Gallagher-Kell", "Calculates the properties of water from the HGK equation of state.", "Haar, Gallagher and Kell (1984); Levelt Sengers et al. (1983)")
                             .alias({"WaterHGK", "WaterHGKGems"}).classes({SC::WaterSolvent, SC::GasFluid}).impl({"reaktoro", "gems"}).add(listed("A0", 18)).add(listed("A1", 5)).add(listed("yc", 4)).add(listed("ki", 36)).add(listed("li", 36)).add(listed("A3", 36)).add(listed("mi", 4)).add(listed("ni", 4)).add(listed("alpha", 4)).add(listed("beta", 4)).add(listed("ri", 4)).add(listed("ti", 4)).add(listed("A4", 4))
                             .add(saturation()).add(constants({{"referenceTemperature", "K"}, {"referenceDensity", "kg/m3"}, {"referenceHelmholtz", "J/kg"}, {"A20", "coefficient"}, {"z0", "coefficient"}}))
                             .add(builtIn({"ac_wm", "ac_gascon", "ac_tz", "ac_aa", "ac_uref", "ac_sref", "ac_zb", "ac_dzb", "ac_yb", "nc_nc", "el_g1", "el_g2", "el_gf", "el_b1", "el_b2", "el_b1t", "el_b2t", "el_b1tt", "el_b2tt", "hc_tcHGK", "hc_dcHGK", "hc_pcHGK", "hb_Ttop", "hb_Tbtm", "hb_Ptop", "hb_Pbtm", "hb_Dtop", "hb_Dbtm", "li_sDli1", "li_sPli1", "li_sDli37", "li_sPli37", "li_sDIB30", "li_Tli13", "li_Pli13", "li_Dli13", "li_TnIB30", "li_DnIB30", "tt_Utri", "tt_Stri", "tt_Htri", "tt_Atri", "tt_Gtri", "tt_Ttr", "tt_Ptripl", "tt_Dltrip", "tt_Dvtrip", "cr_Tc", "cr_rhoC", "cr_Pc", "cr_Pcon", "cr_Ucon", "cr_Scon", "cr_dPcon"})).add(listed("nc_g", 40)).add(listed("nc_ii", 40)).add(listed("nc_jj", 40)).add(listed("bc_bp", 10)).add(listed("bc_bq", 10)).add(listed("ad_atz", 4)).add(listed("ad_adz", 4)).add(listed("ad_aat", 4)).add(listed("ad_aad", 4)).add(listed("co_a", 20)).add(listed("co_q", 20)).add(listed("co_x", 11)).add(listed("psat_a", 8)).add(listed("ideal_c", 18)).add(listed("visc_a", 4)).add(listed("visc_b", 42)).add(listed("cond_aL", 4)).add(listed("cond_au", 4)).add(listed("cond_bL", 30)).add(listed("cond_bu", 30))));
    m.push_back(waterEoS(Spec("WaterIAPWS95", "Water, IAPWS-95", "Calculates the properties of water from the IAPWS-95 formulation.", "Wagner and Pruss (2002)")
                             .alias({"WaterWP95", "WaterIAPWS95Gems"}).classes({SC::WaterSolvent, SC::GasFluid}).impl({"reaktoro"}).add(saturation()).add(listed("no", 9)).add(listed("gammao", 5)).add(listed("c", 55)).add(listed("d", 55)).add(listed("t", 55)).add(listed("n", 57)).add(listed("alpha", 3)).add(listed("beta", 3)).add(listed("gamma", 3)).add(listed("epsilon", 3)).add(listed("a", 2)).add(listed("b", 2)).add(listed("A", 2)).add(listed("B", 2)).add(listed("C", 2)).add(listed("F", 2)).add(listed("E", 2))));
    m.push_back(waterEoS(Spec("WaterZhangDuan05", "Water, Zhang and Duan", "Calculates the properties of water at high pressure from the equation of Zhang and Duan.", "Zhang and Duan (2005)")
                             .classes(water)
                             .add(zhangDuanGibbs())));
    auto dielectric = [](Spec s) -> Spec { s.role(MethodRole::SolventElectro).classes(water); return s; };
    m.push_back(dielectric(Spec("JohnsonNorton91", "Dielectric constant of Johnson and Norton", "Calculates the dielectric constant of water and the Born functions.", "Johnson and Norton (1991)")
                               .alias({"JN91Reaktoro", "JN91Gems"}).impl({"reaktoro", "gems"})
                               .add(numbered("a", 10, "coefficient of k_i(t)")).add(constants({{"Tr", "reference temperature, K"}, {"Dr", "reference density, kg/m3"}}))));
    m.push_back(dielectric(Spec("SverjenskyEtAl14", "Dielectric constant of Sverjensky et al.", "Calculates the dielectric constant of water at high pressure (DEW model). The published coefficients are for t in degC and the density in g/cm3.", "Sverjensky D.A., Harrison B., Azzolini D. (2014) Geochim. Cosmochim. Acta 129, 125-145")
                               .alias({"Sverjensky14Diel"})
                               .add(constants({{"a1", "coefficient of t in w; t in degC"}, {"a2", "coefficient of sqrt(t) in w"}, {"a3", "constant of w"},
                                               {"b1", "coefficient of t in u"}, {"b2", "coefficient of sqrt(t) in u"}, {"b3", "constant of u"}}))));
    m.push_back(dielectric(Spec("FernandezEtAl97", "Dielectric constant of Fernandez et al.", "Calculates the dielectric constant of water from the IAPWS formulation.", "Fernandez D.P., Goodwin A.R.H., Lemmon E.W., Levelt Sengers J.M.H., Williams R.C. (1997) J. Phys. Chem. Ref. Data 26, 1125-1166")
                               .alias({"Fernandez97Diel"})
                               .add(numbered("n", 12, "coefficient of g")).add(numbered("I", 11, "exponent of the density")).add(numbered("J", 11, "exponent of the temperature"))
                               .add(constants({{"Tc", "critical temperature, K"}, {"rhoc", "critical density, kg/m3"}, {"k", "constant of Boltzmann, SI"}, {"Na", "number of Avogadro, 1/mol"},
                                               {"alpha", "mean molecular polarizability, SI"}, {"epsilon0", "permittivity of vacuum, SI"}, {"mu", "dipole moment, SI"}, {"M", "molar mass, kg/mol"}}))));

    std::vector<MethodSpec> out;
    for (auto& s : m)
        out.push_back(std::move(s.s));
    return out;
}

auto reactionMethods() -> std::vector<MethodSpec>
{
    std::vector<Spec> m;
    {
        Spec s("ConstantReactionProperties", "Constant reaction properties", "Gives the reference reaction properties at every T and P.");
        s.reaction().s.reaction_effects = {E::Set, E::Set, E::Set, E::Set, E::Set, E::Set, true, false, 0};
        m.push_back(s);
    }
    m.push_back(Spec("LogKPolynomial", "log K polynomial", "Calculates log K = a0 + a1 T + a2/T + a3 ln T + a4/T2 + a5 T2 + a6/sqrt(T).")
                    .alias({"LogKAnalytic"}).logK().add(logKCoefficients(true)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKNordstromMunoz88", "log K of Nordstrom and Munoz", "Calculates log K from the same equation as LogKPolynomial, in the form of Nordstrom and Munoz.", "Nordstrom and Munoz (1988)")
                    .logK().add(logKCoefficients(true)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKConstant", "Constant log K", "Keeps log K at the reference value at every T. The coefficients of log K(T), if given, only check the reference value.")
                    .alias({"LogKExtrap1T0"}).logK().add(logKCoefficients(false)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKZeroEntropy", "log K with zero entropy of reaction", "Calculates log K with dS = 0, so dH = dG. The coefficients of log K(T), if given, only check the reference value.")
                    .alias({"LogKExtrap1T1"}).logK().add(logKCoefficients(false)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKVantHoff", "log K of van 't Hoff", "Calculates log K with dH and dS that do not change with T (dCp = 0). The coefficients of log K(T), if given, only check the reference value.")
                    .alias({"LogKExtrap2T"}).logK().add(logKCoefficients(false)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKConstantCp", "log K with constant heat capacity of reaction", "Calculates log K with a dCp that does not change with T; without dCp, dH or dS it uses the simpler form that the data allow. The coefficients of log K(T), if given, only check the reference value.")
                    .alias({"LogKExtrap3T"}).logK().add(logKCoefficients(false)).add(reactionCpCoefficients(false)));
    m.push_back(Spec("LogKTable", "Table of log K", "Interpolates a table of log K(T), with Lagrange polynomials or a cubic spline.")
                    .alias({"LogKLagrange"}).logK());
    m.push_back(Spec("MarshallFranck81", "Ion product of water of Marshall and Franck", "Calculates log K = a + b/T + c/T2 + d/T3 + (e + f/T + g/T2) log10(density of water).", "Marshall and Franck (1981)")
                    .alias({"FrantzMarshall", "MarshallFranck78"}).logK().solvent()
                    .add({par("a", "1", "1", "constant term"), par("b", "K", "K", "term b / T"), par("c", "K2", "K2", "term c / T2"), par("d", "K3", "K3", "term d / T3"),
                          par("e", "1", "1", "density term, constant"), par("f", "K", "K", "density term, f / T"), par("g", "K2", "K2", "density term, g / T2")}));
    m.push_back(Spec("RyzhenkoBryzgalin", "Model of Ryzhenko and Bryzgalin", "Calculates log K of a complex from an electrostatic model.", "Ryzhenko and Bryzgalin")
                    .logK(true).solvent()
                    .add({par("pK298", "1", "1", "pK at 298.15 K"), par("A", "1", "1", "coefficient A"), par("B", "1", "1", "coefficient B")}));
    m.push_back(Spec("DolejsManning10", "Density model of Dolejs and Manning", "Calculates dG = a + b T + c T ln T + d T2 + e T ln(density of water), and log K from it.", "Dolejs and Manning (2010)")
                    .logK().solvent()
                    .add({par("a", "J/mol", "kJ/mol", "enthalpy term"), par("b", "J/(mol.K)", "J/(mol.K)", "entropy term"),
                          par("c", "J/(mol.K)", "J/(mol.K)", "heat capacity, constant term", false), par("d", "J/(mol.K2)", "J/(mol.K2)", "heat capacity, linear term", false),
                          par("e", "J/(mol.K)", "J/(mol.K)", "density term")}));
    m.push_back(Spec("ReactionCpPolynomial", "Heat capacity of reaction", "Gives dCp = a + b T + c/T2 + d/sqrt(T) + e T2; the log K methods use it.")
                    .alias({"DrCpIntegration"}).reaction().add(reactionCpCoefficients(true)));
    m.push_back(Spec("ReactionVolumePolynomial", "Volume of reaction, T and P", "Calculates dV(T, P) from expansion and compressibility terms, and adds the terms to log K, dG and dH.")
                    .alias({"DrVolumeFT"}).volume().add(volumeParameters()));
    m.push_back(Spec("ReactionConstantVolume", "Constant volume of reaction", "Adds dV dP to dG and dH, and the same term to log K.")
                    .alias({"DrVolumeConst"}).volume());

    std::vector<MethodSpec> out;
    for (auto& s : m)
        out.push_back(std::move(s.s));
    return out;
}

auto lower(std::string s) -> std::string
{
    for (auto& c : s)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

auto editDistance(const std::string& a, const std::string& b) -> size_t
{
    std::vector<size_t> row(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j)
        row[j] = j;
    for (size_t i = 1; i <= a.size(); ++i)
    {
        size_t diagonal = row[0];
        row[0] = i;
        for (size_t j = 1; j <= b.size(); ++j)
        {
            const size_t up = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diagonal = up;
        }
    }
    return row[b.size()];
}

} // namespace

MethodCatalog::MethodCatalog()
{
    specs_ = substanceMethods();
    auto reactions = reactionMethods();
    specs_.insert(specs_.end(), reactions.begin(), reactions.end());
    using D = Dependence;
    static const std::map<std::string, Dependence> depends = {
        {"ConstantProperties", D::None}, {"ConstantReactionProperties", D::None}, {"LogKConstant", D::None},
        {"ConstantCp", D::T}, {"CpPolynomial", D::T}, {"CpFeiSaxena86", D::T}, {"NASA9", D::T}, {"LambdaBerman88", D::T},
        {"LogKZeroEntropy", D::T}, {"LogKVantHoff", D::T}, {"LogKConstantCp", D::T}, {"LogKPolynomial", D::T}, {"LogKNordstromMunoz88", D::T},
        {"LogKTable", D::T}, {"ReactionCpPolynomial", D::T},
        {"ConstantVolume", D::P}, {"IdealGas", D::P}, {"ReactionConstantVolume", D::P},
        {"TPTable", D::TP}, {"CpExtendedUNIQUAC", D::TP}, {"HKF88", D::TP}, {"AkinfievDiamond03", D::TP}, {"SoluteHP98", D::TP}, {"AndersonEtAl91", D::TP},
        {"LandauHP98", D::TP}, {"LandauHP11", D::TP}, {"MurnaghanHP98", D::TP}, {"TaitHP11", D::TP}, {"VolumeBerman88", D::TP}, {"VolumeDorogokupets88", D::TP}, {"BirchMurnaghan47", D::TP},
        {"PRSV86", D::TP}, {"SoaveRedlichKwong72", D::TP}, {"PengRobinson78", D::TP}, {"SternerPitzer94", D::TP}, {"CorkHP91", D::TP}, {"ChurakovGottschalk03", D::TP},
        {"WaterHGK84", D::TP}, {"WaterIAPWS95", D::TP}, {"WaterZhangDuan05", D::TP}, {"JohnsonNorton91", D::TP}, {"SverjenskyEtAl14", D::TP}, {"FernandezEtAl97", D::TP},
        {"DolejsManning10", D::TP}, {"MarshallFranck81", D::TP}, {"RyzhenkoBryzgalin", D::TP}, {"ReactionVolumePolynomial", D::TP}};
    for (auto& spec : specs_)
        if (const auto it = depends.find(spec.id); it != depends.end())
            spec.depends_on = it->second;
}

auto MethodCatalog::instance() -> const MethodCatalog&
{
    static const MethodCatalog catalog;
    return catalog;
}

auto MethodCatalog::specs() const -> const std::vector<MethodSpec>&
{
    return specs_;
}

auto MethodCatalog::find(const std::string& idOrAlias, std::optional<OperatesOn> operatesOn) const -> const MethodSpec*
{
    auto suits = [&](const MethodSpec& s) { return !operatesOn || s.operates_on == *operatesOn; };
    for (const auto& s : specs_)  // an id first, then an alias
        if (s.id == idOrAlias && suits(s))
            return &s;
    for (const auto& s : specs_)
        if (suits(s) && std::find(s.aliases.begin(), s.aliases.end(), idOrAlias) != s.aliases.end())
            return &s;
    return nullptr;
}

auto MethodCatalog::suggest(const std::string& unknown) const -> std::string
{
    const auto word = lower(unknown);
    const size_t limit = std::max<size_t>(2, word.size() / 3);  // a few typing errors, not another word
    std::string best;
    size_t bestDistance = limit + 1;
    for (const auto& s : specs_)
    {
        std::vector<std::string> names = s.aliases;
        names.push_back(s.id);
        for (const auto& name : names)
        {
            const auto d = editDistance(word, lower(name));
            if (d < bestDistance)
            {
                bestDistance = d;
                best = s.id;
            }
        }
    }
    return best;
}

} // namespace ThermoFun::v2
