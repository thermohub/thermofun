// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Text dump of every field of a record that the public getters give. Two records are equal
// when their dumps are equal. Numbers are written with 17 digits.
#pragma once

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "Element.h"
#include "Reaction.h"
#include "Substance.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"

namespace tfun_test {

using namespace ThermoFun;

class Dump
{
public:
    void line(const std::string& name, double x)
    {
        char buf[64];
        std::snprintf(buf, sizeof buf, "%.17g", x);
        text_ += name + " = " + buf + "\n";
    }

    void line(const std::string& name, const std::string& x) { text_ += name + " = \"" + x + "\"\n"; }
    void line(const std::string& name, int x) { text_ += name + " = " + std::to_string(x) + "\n"; }

    void line(const std::string& name, const std::vector<double>& x)
    {
        text_ += name + " [" + std::to_string(x.size()) + "]";
        for (double v : x)
        {
            char buf[64];
            std::snprintf(buf, sizeof buf, " %.17g", v);
            text_ += buf;
        }
        text_ += "\n";
    }

    void line(const std::string& name, const std::vector<std::vector<double>>& x)
    {
        text_ += name + " rows " + std::to_string(x.size()) + "\n";
        for (size_t i = 0; i < x.size(); ++i)
            line(name + "[" + std::to_string(i) + "]", x[i]);
    }

    void line(const std::string& name, const std::map<std::string, std::vector<double>>& x)
    {
        for (const auto& kv : x)
            line(name + "." + kv.first, kv.second);
    }

    void line(const std::string& name, const std::map<std::string, std::vector<std::vector<double>>>& x)
    {
        text_ += name + " keys " + std::to_string(x.size()) + "\n";
        for (const auto& kv : x)
            line(name + "." + kv.first, kv.second);
    }

    // val, derivatives, error, status code and message
    template <class S>
    void scalar(const std::string& name, const S& x)
    {
        line(name + ".val", static_cast<double>(x.val));
        line(name + ".ddt", static_cast<double>(x.ddt));
        line(name + ".ddp", static_cast<double>(x.ddp));
        line(name + ".err", static_cast<double>(x.err));
        line(name + ".sta", static_cast<int>(x.sta.first));
        line(name + ".sta.msg", x.sta.second);
    }

    const std::string& str() const { return text_; }

private:
    std::string text_;
};

inline void dumpSubstanceProperties(Dump& d, const std::string& p, const ThermoPropertiesSubstance& x)
{
    d.scalar(p + ".gibbs_energy", x.gibbs_energy);
    d.scalar(p + ".helmholtz_energy", x.helmholtz_energy);
    d.scalar(p + ".internal_energy", x.internal_energy);
    d.scalar(p + ".enthalpy", x.enthalpy);
    d.scalar(p + ".entropy", x.entropy);
    d.scalar(p + ".volume", x.volume);
    d.scalar(p + ".heat_capacity_cp", x.heat_capacity_cp);
    d.scalar(p + ".heat_capacity_cv", x.heat_capacity_cv);
}

inline void dumpReactionProperties(Dump& d, const std::string& p, const ThermoPropertiesReaction& x)
{
    d.scalar(p + ".ln_equilibrium_constant", x.ln_equilibrium_constant);
    d.scalar(p + ".log_equilibrium_constant", x.log_equilibrium_constant);
    d.scalar(p + ".reaction_gibbs_energy", x.reaction_gibbs_energy);
    d.scalar(p + ".reaction_helmholtz_energy", x.reaction_helmholtz_energy);
    d.scalar(p + ".reaction_internal_energy", x.reaction_internal_energy);
    d.scalar(p + ".reaction_enthalpy", x.reaction_enthalpy);
    d.scalar(p + ".reaction_entropy", x.reaction_entropy);
    d.scalar(p + ".reaction_volume", x.reaction_volume);
    d.scalar(p + ".reaction_heat_capacity_cp", x.reaction_heat_capacity_cp);
    d.scalar(p + ".reaction_heat_capacity_cv", x.reaction_heat_capacity_cv);
}

inline void dumpSubstanceParameters(Dump& d, const ThermoParametersSubstance& x)
{
    const std::string p = "param";
    d.line(p + ".isothermal_compresibility", x.isothermal_compresibility);
    d.line(p + ".isobaric_expansivity", x.isobaric_expansivity);
    d.line(p + ".temperature_intervals", x.temperature_intervals);
    d.line(p + ".pressure_intervals", x.pressure_intervals);
    d.line(p + ".Cp_coeff", x.Cp_coeff);
    d.line(p + ".Cp_nonElectrolyte_coeff", x.Cp_nonElectrolyte_coeff);
    d.line(p + ".phase_transition_prop", x.phase_transition_prop);
    d.line(p + ".phase_transition_prop_Berman", x.phase_transition_prop_Berman);
    d.line(p + ".m_landau_phase_trans_props", x.m_landau_phase_trans_props);
    d.line(p + ".HKF_parameters", x.HKF_parameters);
    d.line(p + ".volume_coeff", x.volume_coeff);
    d.line(p + ".uniquac_coeff", x.uniquac_coeff);
    d.line(p + ".nasa_coeff", x.nasa_coeff);
    d.line(p + ".nasa_temperature_intervals", x.nasa_temperature_intervals);
    d.line(p + ".tp_table.temperatures", x.tp_table.temperatures);
    d.line(p + ".tp_table.pressures", x.tp_table.pressures);
    d.line(p + ".tp_table.values", x.tp_table.values);
    d.line(p + ".tp_table.reference_pressure", x.tp_table.reference_pressure);
    d.line(p + ".volume_Tait_coeff", x.volume_Tait_coeff);
    d.line(p + ".critical_parameters", x.critical_parameters);
    d.line(p + ".volume_BirchM_coeff", x.volume_BirchM_coeff);
    d.line(p + ".empirical_coeff", x.empirical_coeff);
    d.line(p + ".solute_holland_powell98_coeff", x.solute_holland_powell98_coeff);
    d.line(p + ".coefficient_errors", x.coefficient_errors);
}

inline void dumpReactionParameters(Dump& d, const ThermoParametersReaction& x)
{
    const std::string p = "param";
    d.line(p + ".temperature_intervals", x.temperature_intervals);
    d.line(p + ".pressure_intervals", x.pressure_intervals);
    d.line(p + ".reaction_logK_fT_coeff", x.reaction_logK_fT_coeff);
    d.line(p + ".logK_TP_array", x.logK_TP_array);
    d.line(p + ".reaction_Cp_fT_coeff", x.reaction_Cp_fT_coeff);
    d.line(p + ".reaction_V_fT_coeff", x.reaction_V_fT_coeff);
    d.line(p + ".reaction_RB_coeff", x.reaction_RB_coeff);
    d.line(p + ".reaction_FM_coeff", x.reaction_FM_coeff);
    d.line(p + ".reaction_DM10_coeff", x.reaction_DM10_coeff);
    d.line(p + ".coefficient_errors", x.coefficient_errors);
}

inline std::string dumpSubstance(const Substance& s)
{
    Dump d;
    d.line("name", s.name());
    d.line("symbol", s.symbol());
    d.line("formula", s.formula());
    d.line("reactionSymbol", s.reactionSymbol());
    d.line("molarMass", s.molarMass());
    d.line("referenceT", s.referenceT());
    d.line("referenceP", s.referenceP());
    d.line("lowerT", s.lowerT());
    d.line("lowerP", s.lowerP());
    d.line("upperT", s.upperT());
    d.line("upperP", s.upperP());
    d.line("methodGenEOS", static_cast<int>(s.methodGenEOS()));
    d.line("method_T", static_cast<int>(s.method_T()));
    d.line("method_P", static_cast<int>(s.method_P()));
    d.line("substanceClass", static_cast<int>(s.substanceClass()));
    d.line("aggregateState", static_cast<int>(s.aggregateState()));
    d.line("thermoCalculationType", static_cast<int>(s.thermoCalculationType()));
    d.line("charge", s.charge());
    dumpSubstanceProperties(d, "thermoProperties", s.thermoProperties());
    dumpSubstanceProperties(d, "thermoReferenceProperties", s.thermoReferenceProperties());
    dumpSubstanceParameters(d, s.thermoParameters());
    d.line("jsonString", s.jsonString());
    return d.str();
}

inline std::string dumpReaction(const Reaction& r)
{
    Dump d;
    d.line("name", r.name());
    d.line("symbol", r.symbol());
    d.line("equation", r.equation());
    for (const auto& kv : r.reactants())
        d.line("reactant." + kv.first, kv.second);
    d.line("referenceT", r.referenceT());
    d.line("referenceP", r.referenceP());
    d.line("lowerT", r.lowerT());
    d.line("lowerP", r.lowerP());
    d.line("upperT", r.upperT());
    d.line("upperP", r.upperP());
    d.line("methodGenEOS", static_cast<int>(r.methodGenEOS()));
    d.line("method_T", static_cast<int>(r.method_T()));
    d.line("method_P", static_cast<int>(r.method_P()));
    dumpReactionProperties(d, "thermoReferenceProperties", r.thermoReferenceProperties());
    dumpReactionProperties(d, "thermo_ref_prop", r.thermo_ref_prop());
    dumpReactionParameters(d, r.thermoParameters());
    d.line("jsonString", r.jsonString());
    return d.str();
}

inline std::string dumpElement(const Element& e)
{
    Dump d;
    d.line("name", e.name());
    d.line("symbol", e.symbol());
    d.line("molarMass", e.molarMass());
    d.line("entropy", e.entropy());
    d.line("heatCapacity", e.heatCapacity());
    d.line("volume", e.volume());
    d.line("valence", e.valence());
    d.line("class_", e.class_());
    d.line("isotopeMass", e.isotopeMass());
    d.line("number", e.number());
    d.line("jsonString", e.jsonString());
    return d.str();
}

}
