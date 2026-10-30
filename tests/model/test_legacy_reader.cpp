// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of IO/Legacy/LegacyReader.h: small hand-written records, the quirks, and every record of the golden databases.

#include <cmath>
#include <fstream>
#include <cstdlib>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "Common/Ucum.h"
#include "Database.h"
#include "Element.h"
#include "Reaction.h"
#include "Substance.h"
#include "IO/Legacy/LegacyMethodMap.h"
#include "IO/Legacy/LegacyReader.h"
#include "Methods/MethodCatalog.h"
#include "check.h"

using namespace ThermoFun::v2;
using ThermoFun::Diagnostics;
using ThermoFun::Severity;
using nlohmann::json;

namespace {

bool near(double a, double b, double rel = 1e-12)
{
    return std::fabs(a - b) <= rel * std::fmax(std::fabs(a), std::fabs(b));
}

size_t count(const Diagnostics& d, const std::string& code)
{
    size_t n = 0;
    for (const auto& x : d.all())
        n += x.code == code;
    return n;
}

// A solute with HKF coefficients (Al+3 of Slop98): 8 values, 6 units, the third unit is wrong
const char* const hkfRecord = R"JSON({
  "name": "Al+3 ion", "symbol": "Al+3", "formula": "Al+3", "formula_charge": 3,
  "aggregate_state": {"4": "AS_AQUEOUS"}, "class_": {"2": "SC_AQSOLUTE"}, "Tst": 298.15, "Pst": 100000,
  "TPMethods": [{"method": {"3": "solute_hkf88_reaktoro"},
     "eos_hkf_coeffs": {"values": [-0.33802, -1700.71, 14.5185, -20758, 10.7, -80600, 275300, 0],
        "units": ["cal/(mol*bar)", "cal/mol", "(cal*K)/mol", "cal/(mol*K)", "(cal*K)/mol", "cal/mol"]}}],
  "sm_heat_capacity_p": {"values": [-128.696], "errors": [0.5], "units": ["J/(mol*K)"]},
  "sm_gibbs_energy": {"values": [-483708], "units": ["J/mol"]},
  "sm_enthalpy": {"values": [-530673], "units": ["J/mol"]},
  "sm_entropy_abs": {"values": [-325.097], "units": ["J/(mol*K)"]},
  "sm_volume": {"values": [-4.5243], "errors": [0], "units": ["J/bar"]},
  "_label": "substance", "references": ["Slop98_dat:1998:data:"]
})JSON";

void testSolute()
{
    Diagnostics d;
    const SubstanceRecord s = readLegacySubstance(hkfRecord, {}, &d);
    CHECK(s.symbol == "Al+3");
    CHECK(s.name == "Al+3 ion");
    CHECK(s.charge == 3);
    CHECK(s.class_ == SubstanceClass::AqueousSolute);
    CHECK(s.aggregate_state == AggregateState::Aqueous);
    CHECK(s.source_format == DataFormat::Legacy);
    CHECK(s.original_text == hkfRecord);
    CHECK(s.reference_conditions.T == 298.15 && s.reference_conditions.P == 1e5);
    CHECK(!s.molar_mass.has_value());

    // values in SI; a volume in J/bar is m3/mol
    CHECK(s.standard_state_properties.at(PropertyName::G).value == -483708.0);
    CHECK(s.standard_state_properties.at(PropertyName::S).value == -325.097);
    CHECK(near(s.standard_state_properties.at(PropertyName::V).value, -4.5243e-5));
    CHECK(s.standard_state_properties.at(PropertyName::G).unit_as_written == "J/mol");
    CHECK(!s.standard_state_properties.at(PropertyName::G).uncertainty.has_value());
    // errors are standard uncertainties; an error of 0 is an uncertainty of 0
    const auto& cp = s.standard_state_properties.at(PropertyName::Cp);
    CHECK(cp.uncertainty && cp.uncertainty->standard == 0.5 && cp.uncertainty->convention_unknown);
    const auto& v = s.standard_state_properties.at(PropertyName::V);
    CHECK(v.uncertainty && v.uncertainty->standard == 0.0);

    // HKF: 7 named parameters in SI; the 8th value is kept
    CHECK(s.methods.size() == 1);
    const auto& m = s.methods[0];
    CHECK(m.method == "HKF88" && m.implementation == "reaktoro");
    CHECK(m.parameters.size() == 7);
    CHECK(near(m.parameters.at("a1").value, -0.33802 * 4.184 / 1e5));   // cal/(mol bar) in J/(mol Pa)
    CHECK(near(m.parameters.at("a2").value, -1700.71 * 4.184));
    CHECK(near(m.parameters.at("a3").value, 14.5185 * 4.184 / 1e5));    // the wrong third unit is replaced
    CHECK(near(m.parameters.at("omega").value, 275300 * 4.184));
    CHECK(m.extra_json.empty());  // the extra value is reported, not kept
    CHECK(count(d, "LEG-QRK-003") == 1);
    CHECK(count(d, "LEG-QRK-010") == 1);
    CHECK(d.ok());
    CHECK(s.data_sources.size() == 1);
    CHECK(s.extra_json.empty());
}

void testCpMineral()
{
    const char* record = R"JSON({
      "symbol": "Mgs", "name": "Magnesite", "formula": "MgCO3", "formula_charge": 0, "mass_per_mole": {"values": [84.314], "units": ["g/mol"]},
      "aggregate_state": {"3": "AS_CRYSTAL"}, "class_": {"0": "SC_COMPONENT"},
      "TPMethods": [
        {"method": {"0": "cp_ft_equation"}, "limitsTP": {"lowerT": 273.15, "upperT": 500.0},
         "m_heat_capacity_ft_coeffs": {"values": [100, 0.01, 0, 0, 0, 0, 0, 0, 0, 0, 0]},
         "m_phase_trans_props": {"values": [500, 1.5, 750, 0.0002, 0.0001]}},
        {"method": {"0": "cp_ft_equation"}, "limitsTP": {"lowerT": 500.0, "upperT": 1000.0},
         "m_heat_capacity_ft_coeffs": {"values": [120, 0.02, 0, 0, 0, 0, 0, 0, 0, 0, 0]}},
        {"method": {"36": "mv_equation_berman88"},
         "m_volume_fpt_coeffs": {"values": [1e-6, 2e-12, 0, 1.8e-5, 4e-8]}}],
      "m_expansivity": {"values": [4.19e-5], "units": ["kbar"]},
      "m_compressibility": {"values": [2520], "units": ["1e-05/K"]},
      "sm_gibbs_energy": {"values": [-1000000]}
    })JSON";
    Diagnostics d;
    const SubstanceRecord s = readLegacySubstance(record, {}, &d);
    CHECK(s.molar_mass && near(s.molar_mass->value, 0.084314));  // kg/mol
    CHECK(s.methods.size() == 2);  // the two Cp entries are one application with two segments

    const auto& cpi = s.methods[0];
    CHECK(cpi.method == "CpPolynomial");
    CHECK(cpi.segments.size() == 2);
    CHECK(cpi.segments[0].t_min == 273.15 && cpi.segments[0].t_max == 500.0);
    CHECK(cpi.segments[1].t_min == 500.0 && cpi.segments[1].t_max == 1000.0);
    CHECK(cpi.segments[0].parameters.at("a0").value == 100.0);
    CHECK(cpi.segments[1].parameters.at("a1").value == 0.02);
    CHECK(cpi.segments[0].parameters.size() == 10);
    CHECK(cpi.transitions.size() == 1);
    CHECK(cpi.transitions[0].at("T").value == 500.0);
    CHECK(near(cpi.transitions[0].at("dV").value, 0.0002e-5));  // J/bar in m3
    CHECK(near(cpi.transitions[0].at("dTdP").value, 0.0001 / 1e5));  // K/bar in K/Pa

    const auto& vol = s.methods[1];
    CHECK(vol.method == "VolumeBerman88");
    CHECK(near(vol.parameters.at("b1").value, 1.8e-5 / 1e5));  // 1/bar in 1/Pa
    CHECK(near(vol.parameters.at("a2").value, 2e-12));
    // m_expansivity and m_compressibility are record-level in the file; here no method declares them
    CHECK(s.structural.at("alpha0").value == 4.19e-5);
    CHECK(near(s.structural.at("kappa0").value, 2520e8));
    CHECK(count(d, "LEG-QRK-004") == 1 && count(d, "LEG-QRK-005") == 1);
}

void testSolventAndGas()
{
    const char* water = R"JSON({
      "symbol": "H2O@", "aggregate_state": {"4": "AS_AQUEOUS"}, "class_": {"3": "SC_AQSOLVENT"},
      "TPMethods": [{"method": {"31": "water_eos_hgk84_reaktoro"}}, {"method": {"25": "water_diel_jnort91_reaktoro"}}]})JSON";
    const SubstanceRecord w = readLegacySubstance(water);
    CHECK(w.class_ == SubstanceClass::WaterSolvent);
    CHECK(w.methods.size() == 2);
    CHECK(w.methods[0].method == "WaterHGK84" && w.methods[1].method == "JohnsonNorton91");
    CHECK(w.methods[0].method_as_written == "water_eos_hgk84_reaktoro");

    const char* gas = R"JSON({
      "symbol": "CO", "aggregate_state": {"0": "AS_GAS"}, "class_": {"1": "SC_GASFLUID"},
      "TPMethods": [{"method": {"9": "fluid_prsv"},
        "eos_gas_crit_props": {"values": [132.91, 34.96, 0.0483, 0.04279, 0, 0, 0], "units": ["K", "bar", "", ""]}}]})JSON";
    const SubstanceRecord g = readLegacySubstance(gas);
    CHECK(g.class_ == SubstanceClass::GasFluid && g.aggregate_state == AggregateState::Gas);
    const auto& prsv = g.methods.at(0);
    CHECK(prsv.method == "PRSV86");
    CHECK(prsv.parameters.size() == 6);
    CHECK(prsv.parameters.at("Tc").value == 132.91);
    CHECK(near(prsv.parameters.at("Pc").value, 34.96e5));  // bar in Pa
    CHECK(prsv.parameters.at("omega").value == 0.0483);
}

void testReaction()
{
    const char* record = R"JSON({
      "symbol": "CoCl4-2", "equation": "CoCl4-2 = Co+2 + 4Cl-", "Tst": 298.15, "Pst": 100000,
      "limitsTP": {"lowerT": 273.15, "upperT": 423.15, "lowerP": 0, "upperP": 10000000},
      "reactants": [{"coefficient": 1, "symbol": "Co+2"}, {"coefficient": 4, "symbol": "Cl-"}, {"coefficient": -1, "symbol": "CoCl4-2"}],
      "logKr": {"values": [1.1], "errors": [0.01]},
      "drsm_gibbs_energy": {"values": [-6.3], "units": ["kJ/mol"]},
      "drsm_volume": {"values": [2.9461600780487], "units": ["J/bar"]},
      "datasources": "Migdisov et al., 2011",
      "TPMethods": [
        {"method": {"0": "logk_fpt_function"}, "logk_ft_coeffs": {"values": [-3.98, 0, 1514.77, 0, 0, 0, 0, 0, 0, 0, 0, 0]}},
        {"method": {"7": "logk_3_term_extrap"}},
        {"method": {"13": "dr_volume_constant"}}]})JSON";
    Diagnostics d;
    const ReactionRecord r = readLegacyReaction(record, {}, &d);
    CHECK(r.symbol == "CoCl4-2");
    CHECK(r.stoichiometry.size() == 3);
    CHECK(r.stoichiometry[0].first == "Co+2" && r.stoichiometry[0].second == 1.0);  // the order of the file
    CHECK(r.stoichiometry[2].first == "CoCl4-2" && r.stoichiometry[2].second == -1.0);
    CHECK(r.reaction_properties.at(ReactionPropertyName::logK).value == 1.1);
    CHECK(r.reaction_properties.at(ReactionPropertyName::logK).uncertainty->standard == 0.01);
    CHECK(r.reaction_properties.at(ReactionPropertyName::dG).value == -6300.0);  // kJ/mol in J/mol
    CHECK(near(r.reaction_properties.at(ReactionPropertyName::dV).value, 2.9461600780487e-5));
    CHECK(r.applicability.t_min == 273.15 && r.applicability.p_max == 1e7);
    CHECK(r.methods.size() == 2);
    CHECK(r.methods[0].method == "LogKConstantCp");
    // logk_fpt_function holds coefficients for the log K method beside it: one base method remains
    CHECK(r.methods[0].parameters.size() == 7);  // the 12 values: 7 are used
    CHECK(r.methods[0].parameters.at("a2").value == 1514.77);
    CHECK(r.methods.size() == 2);
    CHECK(r.methods[1].method == "ReactionConstantVolume");
    CHECK(r.data_sources.size() == 1);
    CHECK(d.ok());
}

void testQuirks()
{
    // LEG-QRK-001: the id decides; LEG-QRK-002: spaces; LEG-QRK-006: no aggregate state; LEG-QRK-007: wrapper
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance(
            R"JSON({"properties": {"symbol": "X", "TPMethods": [{"method": {"3": "solute_hkf88_gems"}}, {"method": {"34": " mv_constant "}}]}})JSON",
            {}, &d);
        CHECK(s.symbol == "X");
        CHECK(s.methods.size() == 2);
        CHECK(s.methods[0].method == "HKF88");  // the id 3 decides
        CHECK(s.methods[1].method == "ConstantVolume");
        CHECK(s.aggregate_state == AggregateState::Other);
        CHECK(count(d, "LEG-QRK-001") == 1);
        CHECK(count(d, "LEG-QRK-002") == 1);
        CHECK(count(d, "LEG-QRK-006") == 1);
        CHECK(count(d, "LEG-QRK-007") == 1);
    }
    // LEG-QRK-008: a value that is null is missing; a unit list that is short or has empty units (LEG-QRK-011)
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance(
            R"JSON({"symbol": "Y", "sm_gibbs_energy": {"values": [null], "units": ["J/mol"]},
                "TPMethods": [{"method": {"9": "fluid_prsv"}, "eos_gas_crit_props": {"values": [100, 30, 0.1], "units": ["K"]}}]})JSON",
            {}, &d);
        CHECK(s.standard_state_properties.count(PropertyName::G) == 0);
        CHECK(count(d, "LEG-QRK-008") == 1);
        CHECK(count(d, "LEG-QRK-011") == 1);
        CHECK(near(s.methods.at(0).parameters.at("Pc").value, 30e5));  // the unit of the model, bar
    }
    // an unknown method id is an error; a method that does nothing is a warning
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance(
            R"JSON({"symbol": "Z", "TPMethods": [{"method": {"99": "nothing"}}, {"method": {"6": "landau_berman88"}}]})JSON", {}, &d);
        CHECK(count(d, "LEG-MTH-001") == 1);
        CHECK(count(d, "LEG-MTH-002") == 1);
        CHECK(!d.ok());
        CHECK(s.methods.empty());  // neither is a method of the record
    }
    // a key of another method is placed in the method of the record that has it, else reported with LEG-KEY-002
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance(
            R"JSON({"symbol": "W", "TPMethods": [
                 {"method": {"3": "solute_hkf88_reaktoro"}, "eos_hkf_coeffs": {"values": [1, 2, 3, 4, 5, 6, 7]},
                  "m_volume_fpt_coeffs": {"values": [1e-6, 0, 0, 0, 0]}},
                 {"method": {"36": "mv_equation_berman88"}}]})JSON", {}, &d);
        CHECK(s.methods.size() == 2);
        CHECK(s.methods[1].parameters.count("a1") == 1);  // the volume coefficients moved to the volume method
        Diagnostics d2;
        const SubstanceRecord t = readLegacySubstance(
            R"JSON({"symbol": "V", "TPMethods": [{"method": {"3": "solute_hkf88_reaktoro"}, "m_volume_fpt_coeffs": {"values": [1e-6, 0, 0, 0, 0]}}]})JSON",
            {}, &d2);
        CHECK(count(d2, "LEG-KEY-002") == 1);
        CHECK(t.methods[0].extra_json.empty());  // reported, not kept
    }
    // text that is not JSON gives an error
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance("not json", {}, &d);
        CHECK(!d.ok());
        CHECK(s.symbol.empty());
    }
    // the coverage factor of the options
    {
        LegacyReadOptions options;
        options.error_coverage_factor = 2.0;
        const SubstanceRecord s =
            readLegacySubstance(R"JSON({"symbol": "U", "sm_gibbs_energy": {"values": [1], "errors": [4]}})JSON", options);
        const auto& u = *s.standard_state_properties.at(PropertyName::G).uncertainty;
        CHECK(u.standard == 2.0 && u.coverage_factor_as_written == 2.0);
    }
}

void testElementAndInfo()
{
    Diagnostics d;
    const ElementRecord e = readLegacyElement(
        R"JSON({"symbol": "Fe", "name": "Iron", "class_": {"0": "ELEMENT"}, "atomic_mass": {"name": "M0i", "values": [55.845]},
            "entropy": {"name": "S0i", "values": [27.28]}, "datasources": ["Supcrt-Slop98"]})JSON", {}, &d);
    CHECK(e.symbol == "Fe" && e.class_ == ElementClass::Element);
    CHECK(near(e.molar_mass->value, 0.055845));  // kg/mol
    CHECK(e.standard_entropy->value == 27.28);
    CHECK(e.data_sources.size() == 1);
    CHECK(e.extra_json.empty());

    const ElementRecord z = readLegacyElement(R"JSON({"symbol": "Zz", "class_": {"4": "CHARGE"}})JSON");
    CHECK(z.class_ == ElementClass::Charge);
    const ElementRecord u = readLegacyElement(R"JSON({"symbol": "Xx", "class_": {"2": "ISOTOPE"}})JSON");
    CHECK(u.class_ == ElementClass::Other);

    const ThermoDataSetInfo info = readLegacyDocumentInfo(
        R"JSON({"thermodataset": "aq17", "date": "09.12.2019", "datasources": ["a", "b"],
            "conventions": {"water_triple_point_reference": "NEA_HGK"}, "substances": []})JSON");
    CHECK(info.symbol == "aq17" && info.legacy_date == "09.12.2019" && info.legacy_datasources.size() == 2);
    CHECK(info.water_triple_point_reference == "NEA_HGK");
    CHECK(readLegacyDocumentInfo("[]").symbol.empty());
}

// Corundum of aq17: alpha0 = 4.19e-5 1/K and kappa0 = 2520 kbar = 2.52e11 Pa
void testCorundum()
{
    std::ifstream in("Resources/databases/aq17-thermofun.json");
    CHECK(in.good());
    const json j = json::parse(in);
    bool found = false;
    for (const auto& rec : j["substances"])
        if (rec["symbol"] == "Corundum")
        {
            found = true;
            const SubstanceRecord s = readLegacySubstance(rec.dump());
            bool ok = false;
            for (const auto& app : s.methods)
                if (app.parameters.count("alpha0"))
                {
                    ok = true;
                    CHECK(near(app.parameters.at("alpha0").value, 4.19e-5, 1e-6));
                    CHECK(near(app.parameters.at("kappa0").value, 2.52e11, 1e-6));
                }
            CHECK(ok || s.structural.count("alpha0") == 1);
        }
    CHECK(found);
}

// Every record of every golden database is read without an error, and the records agree with the file
void testRules()
{
    // a log K method gets the coefficients of log K and of dCp
    {
        const ReactionRecord r = readLegacyReaction(R"JSON({"symbol": "R", "TPMethods": [
            {"method": {"0": "logk_fpt_function"}, "logk_ft_coeffs": {"values": [1, 2, 3, 4, 5, 6, 7]},
             "dr_heat_capacity_ft_coeffs": {"values": [10, 20, 30, 40, 50]}},
            {"method": {"7": "logk_3_term_extrap"}}]})JSON");
        CHECK(r.methods.size() == 1 && r.methods[0].method == "LogKConstantCp");
        CHECK(r.methods[0].parameters.at("a0").value == 1.0 && r.methods[0].parameters.at("e").value == 50.0);
        const ReactionRecord alone = readLegacyReaction(R"JSON({"symbol": "R", "TPMethods": [
            {"method": {"0": "logk_fpt_function"}, "logk_ft_coeffs": {"values": [1, 2, 3, 4, 5, 6, 7]}}]})JSON");
        CHECK(alone.methods.size() == 1 && alone.methods[0].method == "LogKPolynomial");
        const ReactionRecord cp = readLegacyReaction(R"JSON({"symbol": "R", "TPMethods": [
            {"method": {"11": "dr_heat_capacity_ft"}, "dr_heat_capacity_ft_coeffs": {"values": [10, 20, 30, 40, 50]}},
            {"method": {"6": "logk_2_term_extrap"}}]})JSON");
        CHECK(cp.methods.size() == 1 && cp.methods[0].method == "LogKVantHoff" && cp.methods[0].parameters.at("a").value == 10.0);
    }
    // a reaction without methods comes from its reactants
    CHECK(readLegacyReaction(R"JSON({"symbol": "R"})JSON").preferred_property_source == PropertySource::Reactants);
    CHECK(!readLegacyReaction(R"JSON({"symbol": "R", "TPMethods": [{"method": {"7": "logk_3_term_extrap"}}]})JSON").preferred_property_source);

    // a substance from a reaction: no methods, or only a constant volume
    {
        const SubstanceRecord noKey = readLegacySubstance(R"JSON({"symbol": "S", "reaction": "S"})JSON");
        CHECK(noKey.defining_reaction == "S" && noKey.preferred_property_source == PropertySource::Reaction);
        const SubstanceRecord emptyList = readLegacySubstance(R"JSON({"symbol": "S", "reaction": "S", "TPMethods": []})JSON");
        CHECK(emptyList.preferred_property_source == PropertySource::Reaction);
        const SubstanceRecord volumeOnly = readLegacySubstance(R"JSON({"symbol": "S", "TPMethods": [{"method": {"34": "mv_constant"}}]})JSON");
        CHECK(volumeOnly.preferred_property_source == PropertySource::Reaction);
        const SubstanceRecord direct = readLegacySubstance(R"JSON({"symbol": "S", "reaction": "S", "TPMethods": [{"method": {"0": "cp_ft_equation"}}]})JSON");
        CHECK(direct.defining_reaction == "S" && !direct.preferred_property_source);
    }

    // water vapour
    {
        auto state = [](const SubstanceRecord& s) { return s.methods.at(0).parameters.at("state").value; };
        const SubstanceRecord flag = readLegacySubstance(R"JSON({"symbol": "H2O(g)", "class_": {"1": "SC_GASFLUID"},
            "TPMethods": [{"method": {"2": "solute_hkf88_gems"}}, {"method": {"8": "fug_critical_param"}}]})JSON");
        CHECK(flag.methods.size() == 1 && flag.methods[0].method == "WaterHGK84" && flag.methods[0].implementation == "gems" && state(flag) == 1.0);
        const SubstanceRecord eos = readLegacySubstance(R"JSON({"symbol": "H2O(g)", "class_": {"1": "SC_GASFLUID"},
            "TPMethods": [{"method": {"3": "solute_hkf88_reaktoro"}}, {"method": {"31": "water_eos_hgk84_reaktoro"}}]})JSON");
        CHECK(eos.methods.size() == 1 && eos.methods[0].method == "WaterHGK84" && state(eos) == 1.0);
        const SubstanceRecord gas = readLegacySubstance(R"JSON({"symbol": "H2O(g)", "aggregate_state": {"0": "AS_GAS"},
            "TPMethods": [{"method": {"31": "water_eos_hgk84_reaktoro"}}]})JSON");
        CHECK(state(gas) == 1.0);
        const SubstanceRecord liquid = readLegacySubstance(R"JSON({"symbol": "H2O@", "aggregate_state": {"4": "AS_AQUEOUS"}, "class_": {"3": "SC_AQSOLVENT"},
            "TPMethods": [{"method": {"31": "water_eos_hgk84_reaktoro"}}]})JSON");
        CHECK(liquid.methods.at(0).parameters.count("state") == 0);
    }

    // the IAPWS-95 of GEMS has no code: the Reaktoro one, with a message
    {
        Diagnostics d;
        const SubstanceRecord s = readLegacySubstance(R"JSON({"symbol": "H2O@", "TPMethods": [{"method": {"30": "water_eos_iapws95_gems"}}]})JSON", {}, &d);
        CHECK(s.methods.at(0).method == "WaterIAPWS95" && s.methods.at(0).implementation == "reaktoro");
        CHECK(count(d, "LEG-MTH-003") == 1 && d.ok());
    }

    // numbers of a single-precision source; a value that is the empty marker of GEMS
    {
        const SubstanceRecord s = readLegacySubstance(R"JSON({"symbol": "S",
            "sm_entropy_abs": {"values": [65.20999908447266]}, "sm_enthalpy": {"values": [65.21]}, "sm_gibbs_energy": {"values": [-1029884]},
            "m_expansivity": {"units": ["kbar"], "values": [1.1754943508222875e-38]},
            "m_compressibility": {"units": ["1e-05/K"], "values": [1.1754943508222875e-38]}})JSON");
        CHECK(s.standard_state_properties.at(PropertyName::S).single_precision_source);
        CHECK(!s.standard_state_properties.at(PropertyName::H).single_precision_source);
        CHECK(!s.standard_state_properties.at(PropertyName::G).single_precision_source);  // an integer says nothing
        CHECK(s.structural.empty());
    }

    // a label of m_expansivity other than the swapped one is reported
    {
        Diagnostics d;
        readLegacySubstance(R"JSON({"symbol": "S", "m_expansivity": {"units": ["1/K"], "values": [4e-5]}})JSON", {}, &d);
        CHECK(count(d, "LEG-QRK-012") == 1);
    }

    // a key that ThermoFun does not read is reported, not kept; the original text keeps it
    {
        Diagnostics d;
        const std::string text = R"JSON({"symbol": "S", "no_such_key": 1})JSON";
        const SubstanceRecord s = readLegacySubstance(text, {}, &d);
        CHECK(count(d, "LEG-KEY-001") == 1);
        CHECK(s.extra_json.empty() && s.original_text == text);
    }

    // every legacy method names a method of the catalog with the parameters of its specification
    for (const auto* list : {&legacy::legacySubstanceMethods(), &legacy::legacyReactionMethods()})
        for (const auto& m : *list)
        {
            if (!*m.method)
                continue;
            const auto on = list == &legacy::legacySubstanceMethods() ? OperatesOn::Substance : OperatesOn::Reaction;
            const auto* spec = MethodCatalog::instance().find(m.method, on);
            CHECK(spec != nullptr);
            if (!spec)
                continue;
            for (const auto& p : m.parameters)
            {
                bool found = false;
                for (const auto& q : spec->parameters)
                    if (q.name == p.name)
                    {
                        found = true;
                        CHECK(ThermoFun::units::dimension(q.si_unit) == ThermoFun::units::dimension(p.unit));
                    }
                CHECK(found);
            }
        }
}

// The map covers every legacy method, with its name, and the coefficient keys of the methods that have code
void testMapCoversTheLegacyTables()
{
    using namespace ThermoFun;
    for (const auto& [id, name] : enum_method_substance)
    {
        const auto* m = legacy::legacySubstanceMethod(static_cast<int>(id));
        CHECK(m != nullptr);
        if (m)
            CHECK(name == m->name);
    }
    for (const auto& [id, name] : enum_method_reaction)
    {
        const auto* m = legacy::legacyReactionMethod(static_cast<int>(id));
        CHECK(m != nullptr);
        if (m)
            CHECK(name == m->name);
    }
    auto covers = [](const legacy::LegacyMethod* m, const std::vector<std::string>& keys) {
        if (!m || !*m->method || !MethodCatalog::instance().find(m->method)->implemented)
            return;  // a method that is not used reads no coefficients
        // tp_table is an object, read on its own; the old table lists two names that the old parser never reads
        // (it reads solute_holland_powell98_coeff, which the map has)
        const std::set<std::string> notArrays = {"tp_table", "solute_holland_powell98_coeffs", "solute_anderson91_coeffs"};
        for (const auto& key : keys)
            if (!key.empty() && !notArrays.count(key))
                CHECK(legacy::legacyCoefficientKeys().count(key) == 1);
    };
    for (const auto& [type, keys] : substance_method_parameters)
        covers(legacy::legacySubstanceMethod(static_cast<int>(type)), keys);
    for (const auto& [type, keys] : reaction_method_parameters)
        covers(legacy::legacyReactionMethod(static_cast<int>(type)), keys);
}

// The public classes hold the record of their JSON text; an old setter clears it
void testRecordLink()
{
    ThermoFun::Database db("pytests/test-thermoengine-thermofun.json");
    CHECK(db.numberOfSubstances() > 0 && db.numberOfReactions() > 0 && db.numberOfElements() > 0);
    for (const auto& s : db.getSubstances())
    {
        CHECK(s.record() != nullptr);
        if (s.record())
            CHECK(s.record()->symbol == s.symbol());
    }
    for (const auto& r : db.getReactions())
        CHECK(r.record() != nullptr);
    for (const auto& e : db.getElements())
        CHECK(e.record() != nullptr);

    // a copy shares the record; a setter clears it only in the object that changes
    ThermoFun::Substance a = db.getSubstance("Cal");
    ThermoFun::Substance b = a;
    CHECK(a.record() != nullptr && a.record() == b.record());
    b.setName("changed");
    CHECK(b.record() == nullptr);
    CHECK(a.record() != nullptr);
    CHECK(db.getSubstance("Cal").record() != nullptr);

    ThermoFun::Reaction r = db.getReactions().front();
    r.setSymbol("changed");
    CHECK(r.record() == nullptr);
    ThermoFun::Element e = db.getElements().front();
    e.setValence(1);
    CHECK(e.record() == nullptr);

    // an object made by the setters has no record
    CHECK(ThermoFun::Substance().record() == nullptr);
}

// The migration report of a database: how many diagnostics of each code
void addToReport(std::map<std::string, int>& report, const Diagnostics& d)
{
    for (const auto& x : d.all())
        ++report[x.code];
}

void testAllDatabases()
{
    const char* files[] = {"Resources/databases/aq17-thermofun.json", "Resources/databases/cemdata18-thermofun.json",
                           "Resources/databases/heracles-thermofun.json", "Resources/databases/mines16-thermofun.json",
                           "Resources/databases/psinagra-12-07-thermofun.json", "Resources/databases/slop98-organic-thermofun.json",
                           "Resources/databases/slop98-thermofun.json", "pytests/test-aq17-gem-lma-thermofun.json",
                           "tests/extra_models/extra_models_db.json"};
    size_t substances = 0, reactions = 0, elements = 0, errors = 0, methods = 0;
    json reports = json::object();
    for (const char* file : files)
    {
        std::ifstream in(file);
        const json j = json::parse(in);
        std::map<std::string, int> report;
        for (const auto& rec : j.value("substances", json::array()))
        {
            Diagnostics d;
            const SubstanceRecord s = readLegacySubstance(rec.dump(), {}, &d);
            for (const auto& x : d.errors())
                if (errors++ < 5)
                    std::printf("  %s %s: %s\n", file, s.symbol.c_str(), x.message.c_str());
            addToReport(report, d);
            CHECK(s.symbol == rec["symbol"].get<std::string>());
            CHECK(s.methods.size() <= rec.value("TPMethods", json::array()).size());
            methods += s.methods.size();
            for (const char* key : {"sm_gibbs_energy", "sm_enthalpy"})
                if (rec.contains(key) && rec[key].contains("values") && !rec[key]["values"].empty() && rec[key]["values"][0].is_number())
                    CHECK(s.standard_state_properties.count(key == std::string("sm_gibbs_energy") ? PropertyName::G : PropertyName::H) == 1);
            ++substances;
        }
        for (const auto& rec : j.value("reactions", json::array()))
        {
            Diagnostics d;
            const ReactionRecord r = readLegacyReaction(rec.dump(), {}, &d);
            for (const auto& x : d.errors())
                if (errors++ < 5)
                    std::printf("  %s %s: %s\n", file, r.symbol.c_str(), x.message.c_str());
            addToReport(report, d);
            CHECK(r.symbol == rec["symbol"].get<std::string>());
            size_t expected = 0;
            for (const auto& item : rec.value("reactants", json::array()))
                expected += item.contains("symbol") && item.contains("coefficient") && !item["symbol"].is_null() && !item["coefficient"].is_null();
            CHECK(r.stoichiometry.size() == expected);
            ++reactions;
        }
        for (const auto& rec : j.value("elements", json::array()))
        {
            Diagnostics d;
            const ElementRecord e = readLegacyElement(rec.dump(), {}, &d);
            addToReport(report, d);
            CHECK(d.ok());
            CHECK(e.symbol == rec["symbol"].get<std::string>());
            ++elements;
        }
        reports[file] = report;
    }
    // the migration reports are stored: TFUN_WRITE_REPORT=1 writes them again, in a change that explains the difference
    const char* const stored = "tests/model/data/migration_report.json";
    if (std::getenv("TFUN_WRITE_REPORT"))
        std::ofstream(stored) << reports.dump(1) << "\n";
    std::ifstream storedFile(stored);
    CHECK(storedFile.good());
    if (storedFile.good())
    {
        const json expected = json::parse(storedFile);
        CHECK(expected == reports);
        if (expected != reports)
            std::printf("  migration report differs:\n%s\n", json::diff(expected, reports).dump(1).c_str());
    }
    std::printf("  legacy reader: %zu substances, %zu reactions, %zu elements, %zu methods, %zu errors\n", substances, reactions,
                elements, methods, errors);
    CHECK(substances > 4000 && reactions > 600 && elements > 300);
    CHECK(errors == 0);
}

}

void runLegacyReader()
{
    testSolute();
    testCpMineral();
    testSolventAndGas();
    testReaction();
    testQuirks();
    testElementAndInfo();
    testCorundum();
    testRules();
    testMapCoversTheLegacyTables();
    testRecordLink();
    testAllDatabases();
}
