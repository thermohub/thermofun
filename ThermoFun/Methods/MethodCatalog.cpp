// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodCatalog.h"

#include <algorithm>
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
    auto piecewise() -> Spec& { s.piecewise = true; return *this; }
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
                    .alias({"HKFReaktoro", "HKFGems"}).base().classes(solutes).solvent().impl({"reaktoro", "gems"}).add(hkfParameters()));
    {
        Spec s("AkinfievDiamond03", "Model of Akinfiev and Diamond", "Calculates the properties of a dissolved gas or neutral species from those of the gas and of water.",
               "Akinfiev and Diamond (2003)");
        s.effects(E::Read, E::Read, E::Read, E::Read, E::Set, 300, true).classes(solutes).solvent(true).impl({"gems"});
        s.s.needs.ideal_gas_water = true;
        s.add({par("xi", "1", "1", "parameter xi"), par("a", "m3/kg", "cm3/g", "parameter a"),
               par("b", "m3.K^0.5/kg", "cm3.K^0.5/g", "parameter b")});
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
    m.push_back(Spec("IdealGas", "Ideal gas", "Sets V = RT/P and adds RT ln(P/P0) to G.")
                    .alias({"IdealGasVol"}).effects(E::Add, E::Add, E::None, E::None, E::Set, 200).classes({SC::GasFluid, SC::Component}));
    auto fluid = [](Spec s) -> Spec { s.effects(E::Add, E::Add, E::Add, E::None, E::Set, 200).classes(fluids); return s; };
    m.push_back(fluid(Spec("PRSV86", "Peng-Robinson-Stryjek-Vera equation of state", "Adds the pressure terms of the PRSV equation of state to G, H and S, and sets V.",
                           "Stryjek and Vera (1986); Proust and Vera (1989)")
                          .alias({"PRSV"}).add(interactionParameters())));
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
                          .alias({"SRK"}).add(cubicParameters())));
    m.push_back(fluid(Spec("SternerPitzer94", "Equation of state of Sterner and Pitzer", "Adds the pressure terms of the Sterner-Pitzer equation of state to G, H and S, and sets V.", "Sterner and Pitzer (1994)")
                          .alias({"SternerPitzer"}).add({par("Tc", "K", "K", "critical temperature"), par("Pc", "Pa", "bar", "critical pressure")})));
    m.push_back(fluid(Spec("PengRobinson78", "Peng-Robinson equation of state", "Adds the pressure terms of the Peng-Robinson equation of state (1978 form) to G, H and S, and sets V.", "Peng and Robinson (1976, 1978)")
                          .alias({"PR78"}).add(cubicParameters())));
    m.push_back(fluid(Spec("CorkHP91", "Compensated Redlich-Kwong equation of state", "Adds the pressure terms of the CORK equation of state to G, H and S, and sets V.", "Holland and Powell (1991)")
                          .alias({"CORK"}).add(interactionParameters())));

    // water
    auto waterEoS = [](Spec s) -> Spec
    {
        auto state = par("state", "1", "1", "0: liquid, 1: gas", false);
        state.default_si = 0.0;
        state.fittable = false;
        s.base().role(MethodRole::SolventProperties).add(state);
        return s;
    };
    m.push_back(waterEoS(Spec("WaterHGK84", "Water, Haar-Gallagher-Kell", "Calculates the properties of water from the HGK equation of state.", "Haar, Gallagher and Kell (1984); Levelt Sengers et al. (1983)")
                             .alias({"WaterHGK", "WaterHGKGems"}).classes({SC::WaterSolvent, SC::GasFluid}).impl({"reaktoro", "gems"})));
    m.push_back(waterEoS(Spec("WaterIAPWS95", "Water, IAPWS-95", "Calculates the properties of water from the IAPWS-95 formulation.", "Wagner and Pruss (2002)")
                             .alias({"WaterWP95", "WaterIAPWS95Gems"}).classes({SC::WaterSolvent, SC::GasFluid}).impl({"reaktoro"})));
    m.push_back(waterEoS(Spec("WaterZhangDuan05", "Water, Zhang and Duan", "Calculates the properties of water at high pressure from the equation of Zhang and Duan.", "Zhang and Duan (2005)")
                             .classes(water)));
    auto dielectric = [](Spec s) -> Spec { s.role(MethodRole::SolventElectro).classes(water); return s; };
    m.push_back(dielectric(Spec("JohnsonNorton91", "Dielectric constant of Johnson and Norton", "Calculates the dielectric constant of water and the Born functions.", "Johnson and Norton (1991)")
                               .alias({"JN91Reaktoro", "JN91Gems"}).impl({"reaktoro", "gems"})));
    m.push_back(dielectric(Spec("SverjenskyEtAl14", "Dielectric constant of Sverjensky et al.", "Calculates the dielectric constant of water at high pressure (DEW model).", "Sverjensky et al. (2014)")
                               .alias({"Sverjensky14Diel"})));
    m.push_back(dielectric(Spec("FernandezEtAl97", "Dielectric constant of Fernandez et al.", "Calculates the dielectric constant of water from the IAPWS formulation.", "Fernandez et al. (1997)")
                               .alias({"Fernandez97Diel"})));

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
                    .alias({"LogKLagrange"}).logK().notImplemented());
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
