// C++ includes
#include <algorithm>
#include <stdlib.h>
#include <regex>

// ThermoFun includes
#include "ParseJsonToData.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"
#include "Common/Exception.h"
#include "Common/Units.hpp"

// JSON
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ThermoFun
{

auto getTPMethods(const json &j, Substance &s) -> void;
auto getTPMethods(const json &j, Reaction &r) -> void;
auto thermoParamSubst(const json &j, std::string prop_name, ThermoParametersSubstance &ps) -> void;
auto thermoParamReac(const json &j, ThermoParametersReaction &pr) -> void;

auto thermoRefPropSubst(const json &j) -> ThermoPropertiesSubstance;
auto thermoRefPropReac(const json &j) -> ThermoPropertiesReaction;

//auto index_from_map (std::string map) -> int
//{
//    unsigned first = map.find("\"");
//    unsigned second = map.find("\"", first+1);
//    std::string strNew = map.substr (first+1,second-(first+1));
//    return stoi(strNew);
//}

//bool extractMapFirst( const std::string& data,
//                      const std::string& fieldPath, int& key, std::string& value )
//{
////    std::map<std::string,std::string> class_map;
////    if( domData->findObject( fieldPath, class_map ) )
////    {
////        auto it = class_map.begin();
////        if( it != class_map.end() )
////        {
////            key = std::stoi(it->first);
////            value = it->second;
////            return true;
////        }
////    }
////    return false;
//}

//auto parseIssues(std::string data, std::string name, std::string prop) -> bool
//{
//    if ((data == "*") || (data == "" ) || (data == "{}"))
//    {
//        flog.open(parsinglogfile, ios::app);
//        flog << "Could not parse " << prop << " for "<< name << ", using default value! " << endl;
//        flog.close();
//        return true;
//    }
//    else
//        return false;
//}

auto readValueErrorUnit(const json& j, const std::string& propPath, double& val, double& err, const std::string& unit, const std::string& message) -> Reaktoro_::StatusMessage
{
    Reaktoro_::StatusMessage status = {Reaktoro_::Status::notdefined, message};
    std::string unit_in_record = unit;

    // Check and extract unit
    if (j.contains(propPath) && j[propPath].contains("units")) {
        const auto& units = j[propPath]["units"];
        if (units.is_array() && !units.empty() && !units[0].is_null()) {
            unit_in_record = units[0].get<std::string>();
        }
    }

    // Check and extract value
    if (j.contains(propPath) && j[propPath].contains("values")) {
        const auto& values = j[propPath]["values"];
        if (values.is_array() && !values.empty() && !values[0].is_null()) {
            val = units::convert(values[0].get<double>(), unit_in_record, unit);
            status = {Reaktoro_::Status::read, message};
        }
    }

    // Check and extract error
    if (j.contains(propPath) && j[propPath].contains("errors")) {
        const auto& errors = j[propPath]["errors"];
        if (errors.is_array() && !errors.empty() && !errors[0].is_null()) {
            err = units::convert(errors[0].get<double>(), unit_in_record, unit);
        }
    }

    return status;
}

auto convert_values_units(std::vector<double> values, const std::vector<std::string>& units_from, const std::vector<std::string>& units_to) -> std::vector<double>
{
    for (size_t i = 0; i < values.size(); ++i)
    {
        std::string from = (i < units_from.size()) ? units_from[i] : "";
        std::string to = (i < units_to.size()) ? units_to[i] : "";

        if (from.empty())
            from = to;

        if (from.empty() || to.empty()) {
            from = "1";
            to = "1";
        }

        values[i] = units::convert(values[i], from, to);
    }
    return values;
}

auto read_values_units(const json& j, const std::string& data, std::vector<double>& values, const std::vector<std::string>& units_to) -> void
{
    std::vector<std::string> units_from;

    if (j.contains(data))
    {
        const auto& entry = j[data];

        if (entry.contains("units") && entry["units"].is_array() && !entry["units"].is_null()) {
            units_from = entry["units"].get<std::vector<std::string>>();

            // Temporary fix for bad data
            if (data == "eos_hkf_coeffs" && units_from.size() >= 3 && units_from[2] == "(cal*K)/mol")
                units_from = units_to;
        }
        else {
            units_from = units_to;
        }

        if (entry.contains("values") && entry["values"].is_array() && !entry["values"].is_null()) {
            values = convert_values_units(entry["values"].get<std::vector<double>>(), units_from, units_to);
        }
    }
}

auto read_value_unit(const json& j, const std::string& data, double& value, const std::string& unit_to) -> void
{
    std::string unit_from = unit_to;
    double factor = 1.0;

    if (j.contains(data)) {
        const auto& entry = j[data];

        if (entry.contains("units") && entry["units"].is_array() && !entry["units"][0].is_null())
            unit_from = entry["units"][0].get<std::string>();

        // Temporary fixes
        if (data == "m_expansivity" && unit_from == "kbar") {
            unit_from = "1/K";
            factor = 1e5;
        }

        if (data == "m_compressibility" && unit_from == "1e-05/K") {
            unit_from = "kbar";
            factor = 1;
        }

        if (entry.contains("values") && entry["values"].is_array() && !entry["values"][0].is_null()) {
            value = units::convert(entry["values"][0].get<double>() * factor, unit_from, unit_to);
        }
    }
}


auto getParameterCoefficients(/*const std::string& data,*/ const SubstanceTPMethodType &type) -> std::unordered_map<std::string, std::vector<double>>
{
    std::unordered_map<std::string, std::vector<double>> coefficients;
    std::vector<double> coeffs;
    std::string coeffs_name;
    switch (type)
    {
    case SubstanceTPMethodType::solute_hkf88_gems:
        coeffs_name = "";
        break; // and exits the switch
    case SubstanceTPMethodType::solute_hkf88_reaktoro:
        //thfun_logger->info("2");
        break;
    }
    // throw error method not found
    return coefficients;
}

auto setTPMethods_old(const ReactionTPMethodType &type, Reaction &r) -> void
{
    switch (type)
    {
    case ReactionTPMethodType::dr_heat_capacity_ft:
    case ReactionTPMethodType::adsor_ion_exchange:
    case ReactionTPMethodType::logk_fpt_function:
    case ReactionTPMethodType::iso_compounds_grichuk88:
        r.setMethodGenEoS(MethodGenEoS_Thrift::type(new_old_r_methodtype.at(type)));
        break;
    case ReactionTPMethodType::logk_nordstrom_munoz88:
    case ReactionTPMethodType::logk_1_term_extrap0:
    case ReactionTPMethodType::logk_1_term_extrap1:
    case ReactionTPMethodType::logk_2_term_extrap:
    case ReactionTPMethodType::logk_3_term_extrap:
    case ReactionTPMethodType::logk_lagrange_interp:
    case ReactionTPMethodType::logk_marshall_frank78:
    case ReactionTPMethodType::solute_eos_ryzhenko_gems:
    case ReactionTPMethodType::logk_dolejs_manning10:
        r.setMethod_T(MethodCorrT_Thrift::type(new_old_r_methodtype.at(type)));
        break;
    case ReactionTPMethodType::dr_volume_fpt:
    case ReactionTPMethodType::dr_volume_constant:
        r.setMethod_P(MethodCorrP_Thrift::type(new_old_r_methodtype.at(type)));
        break;
    }
}

auto setTPMethods_old(const SubstanceTPMethodType &type, Substance &s) -> void
{
    switch (type)
    {
    case SubstanceTPMethodType::cp_ft_equation:
    case SubstanceTPMethodType::solute_hkf88_gems:
    case SubstanceTPMethodType::solute_hkf88_reaktoro:
    case SubstanceTPMethodType::water_diel_jnort91_reaktoro:
    case SubstanceTPMethodType::water_diel_jnort91_gems:
    case SubstanceTPMethodType::water_diel_sverj14:
    case SubstanceTPMethodType::water_diel_fern97:
        s.setMethodGenEoS(MethodGenEoS_Thrift::type(new_old_s_methodtype.at(type)));
        break;
    case SubstanceTPMethodType::landau_holland_powell98:
    case SubstanceTPMethodType::landau_berman88:
    case SubstanceTPMethodType::cp_ft_equation_saxena86:
    case SubstanceTPMethodType::water_eos_hgk84_lvs83_gems:
    case SubstanceTPMethodType::water_eos_iapws95_gems:
    case SubstanceTPMethodType::water_eos_hgk84_reaktoro:
    case SubstanceTPMethodType::water_eos_iapws95_reaktoro:
    case SubstanceTPMethodType::water_pvt_zhang_duan05:
    case SubstanceTPMethodType::standard_entropy_cp_integration:
        s.setMethod_T(MethodCorrT_Thrift::type(new_old_s_methodtype.at(type)));
        break;
    case SubstanceTPMethodType::mv_constant:
    case SubstanceTPMethodType::mv_equation_dorogokupets88:
    case SubstanceTPMethodType::mv_equation_berman88:
    case SubstanceTPMethodType::mv_eos_birch_murnaghan_gott97:
    case SubstanceTPMethodType::mv_eos_murnaghan_hp98:
    case SubstanceTPMethodType::mv_eos_tait_hp11:
    case SubstanceTPMethodType::fug_critical_param:
    case SubstanceTPMethodType::fluid_prsv:
    case SubstanceTPMethodType::fluid_churakov_gottschalk:
    case SubstanceTPMethodType::fluid_soave_redlich_kwong:
    case SubstanceTPMethodType::fluid_sterner_pitzer:
    case SubstanceTPMethodType::fluid_peng_robinson78:
    case SubstanceTPMethodType::fluid_comp_redlich_kwong_hp91:
    case SubstanceTPMethodType::solute_aknifiev_diamond03:
    case SubstanceTPMethodType::mv_pvnrt:
        s.setMethod_P(MethodCorrP_Thrift::type(new_old_s_methodtype.at(type)));
        break;
    }

    // throw error method not found
}

//auto getTPMethods(const std::vector<std::string>& vkbuf) -> listmethods
//{
//    listmethods tpmethods;

//    for (auto kbuf : vkbuf)
//    {
//        auto jmethod = jsonio::unpackJson( kbuf );
//        int key;
//        Method method;
//        extractMapFirst(jmethod.get(), "method", key, method.name);
//        method.parameter_coefficients = getParameterCoefficients(/*jmethod.get(),*/ SubstanceTPMethodType(key));
//    }
//    return tpmethods;
//}

auto getTPMethods(const json &j, Reaction &r) -> void
{
    ThermoParametersReaction pr;
    json methods = j["TPMethods"];

    for (auto it = methods.begin(); it != methods.end(); ++it)
    {
        const auto& val = it.value();
        if (val.contains("method")) {
            const auto& method = val["method"];
            if ((method.is_object() || method.is_array()) &&
                method.begin() != method.end() &&
                !method.begin()->is_null()) {

                int key = stoi(it.value()["method"].begin().key());
                std::string name = it.value()["method"].begin().value();
                setTPMethods_old(ReactionTPMethodType(key), r);
                thermoParamReac(it.value(), pr);
            }
        }
    }
    r.setThermoParameters(pr);
}

auto getTPMethods(const json &j, Substance &s) -> void
{
    ThermoParametersSubstance ps;
    json methods = j["TPMethods"];
    std::string unit_from;

    for (auto it = methods.begin(); it != methods.end(); ++it)
    {
        const auto& val = it.value();
        if (val.contains("method")) {
            const auto& method = val["method"];
            if ((method.is_object() || method.is_array()) &&
                method.begin() != method.end() &&
                !method.begin()->is_null()) {

                int key = stoi(it.value()["method"].begin().key());
                std::string name = it.value()["method"].begin().value();
                setTPMethods_old(SubstanceTPMethodType(key), s);
                thermoParamSubst(val, name, ps);
            }
        }
    }

    read_value_unit(j, "m_expansivity", ps.isobaric_expansivity, "1/K");
    read_value_unit(j, "m_compressibility", ps.isothermal_compresibility, "kbar");

    s.setThermoParameters(ps);
}

auto thermoParamSubst(const json &j, std::string prop_name, ThermoParametersSubstance &ps) -> void
{
    std::vector<std::string> vkbuf;
    std::string kbuf;

    read_values_units(j, "eos_akinfiev_diamond_coeffs", ps.Cp_nonElectrolyte_coeff, {"1", "(cm^3)/g", "(cm^3*K^0.5)/g"});
    // ps.volume_BirchM_coeff = read_values_units(j, "eos_birch_murnaghan_coeffs", {});
    read_values_units(j, "eos_gas_crit_props", ps.critical_parameters, {"K", "bar", "1", "1"});
    read_values_units(j, "eos_hkf_coeffs", ps.HKF_parameters, {"cal/(mol*bar)", "cal/mol", "(cal*K)/(mol*bar)", "(cal*K)/mol", "cal/(mol*K)", "(cal*K)/mol", "cal/mol"});

    // temporary fix - need to think how to handle more than 1 TP interval - for new structure - simplified
    if (prop_name == "cp_ft_equation")
    {
        std::vector<double> low_up;
        if (j.contains("limitsTP"))
        {
            if (j["limitsTP"].contains("lowerT"))
                if (!j["limitsTP"]["lowerT"].is_null())
                    low_up.push_back(j["limitsTP"]["lowerT"].get<double>());
            if (j["limitsTP"].contains("upperT"))
                if (!j["limitsTP"]["upperT"].is_null())
                    low_up.push_back(j["limitsTP"]["upperT"].get<double>());
        }
        ps.temperature_intervals.push_back(low_up);
    }
    std::vector<double> cp, ph;
    read_values_units(j, "m_heat_capacity_ft_coeffs", cp, {"J/(mol*K)", "J/(mol*K^2)", "(J*K)/mol", "J/(mol*K^0.5)", "J/(mol*K^3)", "J/(mol*K^4)", "J/(mol*K^5)", "(J*K^2)/mol", "J/mol", "J/(mol*K^1.5)", "J/(mol*K)"});
    if (cp.size() > 0)
        ps.Cp_coeff.push_back(cp);
    read_values_units(j, "m_phase_trans_props", ph, {"K", "J/(mol*K)", "J/mol", "J/bar", "K/bar"});
    if (ph.size() > 0)
        ps.phase_transition_prop.push_back(ph);
    read_values_units(j, "m_landau_phase_trans_props", ps.m_landau_phase_trans_props, {"degC", "J/(mol*K)", "J/bar"});
    read_values_units(j, "solute_holland_powell98_coeff", ps.solute_holland_powell98_coeff, {"kJ/(mol*K^2)"});
    // ps.phase_transition_prop_Berman.push_back(read_values_units(j, "", {});
}

auto thermoParamReac(const json &j, ThermoParametersReaction &pr) -> void
{
    std::vector<std::string> vkbuf, units_from, units_to;
    std::string kbuf;

    read_values_units(j, "logk_ft_coeffs", pr.reaction_logK_fT_coeff, {"1", "1/K", "K", "1", "K^2", "1/K^2", "K^0.5"});
    //    if (j.contains("logk_pt_values") && !j["logk_pt_values"]["values"].is_null())
    //        pr.logK_TP_array = j["logk_pt_values"]["values"].get<vector<double>>();
    read_values_units(j, "dr_heat_capacity_ft_coeffs", pr.reaction_Cp_fT_coeff, {"J/(mol*K)", "J/(mol*K^2)", "(J*K)/mol", "J/(mol*K^0.5)", "J/(mol*K^3)"});
    read_values_units(j, "dr_volume_fpt_coeffs", pr.reaction_V_fT_coeff, {"1/K", "1/K^2", "1/K^3", "1/bar", "1/bar^2"});
    read_values_units(j, "dr_ryzhenko_coeffs", pr.reaction_RB_coeff, {"1", "1", "1"});
    read_values_units(j, "dr_marshall_franck_coeffs", pr.reaction_FM_coeff, {"1", "K", "K^2", "K^3", "1", "K", "K^2"});
    read_values_units(j, "dr_dolejs_manning10_coeffs", pr.reaction_DM10_coeff, {"kJ/mol", "J/(mol*K)", "J/(mol*K)", "J/(mol*K^2)", "J/(mol*K)"});
}

auto thermoRefPropSubst(const json &j) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    std::string message;

    if (j.contains("sm_heat_capacity_p"))
        tps.heat_capacity_cp.sta = readValueErrorUnit(j, "sm_heat_capacity_p", tps.heat_capacity_cp.val, tps.heat_capacity_cp.err, "J/K/mol", message);
    if (j.contains("sm_gibbs_energy"))
        tps.gibbs_energy.sta = readValueErrorUnit(j, "sm_gibbs_energy", tps.gibbs_energy.val, tps.gibbs_energy.err, "J/mol", message);
    if (j.contains("sm_enthalpy"))
        tps.enthalpy.sta = readValueErrorUnit(j, "sm_enthalpy", tps.enthalpy.val, tps.enthalpy.err, "J/mol", message);
    if (j.contains("sm_entropy_abs"))
        tps.entropy.sta = readValueErrorUnit(j, "sm_entropy_abs", tps.entropy.val, tps.entropy.err, "J/K/mol", message);
    if (j.contains("sm_volume"))
        tps.volume.sta = readValueErrorUnit(j, "sm_volume", tps.volume.val, tps.volume.err, "J/bar", message);

    return tps;
}

auto thermoRefPropReac(const json &j) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    std::string message;

    if (j.contains("logKr"))
        tpr.log_equilibrium_constant.sta = readValueErrorUnit(j, "logKr", tpr.log_equilibrium_constant.val, tpr.log_equilibrium_constant.err, "1", message);
    if (j.contains("drsm_heat_capacity_p"))
        tpr.reaction_heat_capacity_cp.sta = readValueErrorUnit(j, "drsm_heat_capacity_p", tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, "J/K/mol", message);
    if (j.contains("drsm_gibbs_energy"))
        tpr.reaction_gibbs_energy.sta = readValueErrorUnit(j, "drsm_gibbs_energy", tpr.reaction_gibbs_energy.val, tpr.reaction_gibbs_energy.err, "J/mol", message);
    if (j.contains("drsm_enthalpy"))
        tpr.reaction_enthalpy.sta = readValueErrorUnit(j, "drsm_enthalpy", tpr.reaction_enthalpy.val, tpr.reaction_enthalpy.err, "J/mol", message);
    if (j.contains("drsm_entropy"))
        tpr.reaction_entropy.sta = readValueErrorUnit(j, "drsm_entropy", tpr.reaction_entropy.val, tpr.reaction_entropy.err, "J/K/mol", message);
    if (j.contains("drsm_volume"))
        tpr.reaction_volume.sta = readValueErrorUnit(j, "drsm_volume", tpr.reaction_volume.val, tpr.reaction_volume.err, "J/bar", message);

    return tpr;
}

auto getReactants(const json &r) -> std::map<std::string, double>
{
    std::map<std::string, double> reactants;
    for (auto it = r.begin(); it != r.end(); ++it)
    {
        json j = it.value();
        if (j.contains("symbol"))
            if (j.contains("coefficient"))
                if (!j["symbol"].is_null() && !j["coefficient"].is_null())
                    reactants[j["symbol"]] = j["coefficient"].get<double>();
    }
    return reactants;
}

auto parseElement(const std::string &data) -> Element
{
    Element e;
    std::string kbuf;
    std::string name;

    try
    {
        json j = json::parse(data);
        if (j.contains("properties"))
            if (!j["properties"].is_null())
                j = j["properties"];

        if (j.contains("name"))
            if (!j["name"].is_null())
            {
                e.setName(j["name"]);
                name = j["name"];
            }

        if (j.contains("symbol"))
            if (!j["symbol"].is_null())
                e.setSymbol(j["symbol"]);

        if (j.contains("number"))
            if (!j["number"].is_null())
                e.setNumber(j["number"].get<int>());
        double val;
        read_value_unit(j, "entropy", val, "J/(mol*K)");
        e.setEntropy(val);
        read_value_unit(j, "heat_capacity", val, "J/(mol*K)");
        e.setHeatCapacity(val);
        read_value_unit(j, "atomic_mass", val, "g/mol");
        e.setMolarMass(val);
        read_value_unit(j, "volume", val, "J/bar");
        e.setVolume(val);

        if (j.contains("class_"))
        {
            if (!j["class_"].is_null() && !j["class_"].empty() && j["class_"].is_object())
                e.setClass(stoi(j["class_"].begin().key()));
            else
                e.setClass(0);
        }

        if (j.contains("isotope_mass") && !j["isotope_mass"].is_null())
            if (!j["isotope_mass"].is_null())
                e.setIsotopeMass(j["isotope_mass"].get<int>());
    }
    catch (json::exception &ex)
    {
        thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
    }

    return e;
}

auto parseSubstance(const std::string &data) -> Substance
{
    Substance s;
    std::vector<std::string> vkbuf;
    std::string kbuf;

    try
    {
        json j = json::parse(data);
        if (j.contains("properties"))
            if (!j["properties"].is_null())
                j = j["properties"];

        if (j.contains("name"))
            if (!j["name"].is_null())
                s.setName(j["name"]);

        if (j.contains("symbol"))
            if (!j["symbol"].is_null())
                s.setSymbol(j["symbol"]);

        if (j.contains("formula"))
            if (!j["formula"].is_null())
                s.setFormula(j["formula"]);

        if (j.contains("formula_charge"))
            if (!j["formula_charge"].is_null())
                s.setCharge(j["formula_charge"].get<int>());

        if (j.contains("reaction"))
            if (!j["reaction"].is_null())
                s.setReactionSymbol(j["reaction"]);

        double val;
        read_value_unit(j, "mass_per_mole", val, "g/mol");
        s.setMolarMass(val);

        if (j.contains("aggregate_state"))
            if (!j["aggregate_state"].is_null() && !j["aggregate_state"].empty())
                s.setAggregateState(
                    static_cast<AggregateState::type>(stoi(j["aggregate_state"].begin().key())));

        if (j.contains("class_"))
            if (!j["class_"].is_null() && !j["class_"].empty())
                s.setSubstanceClass(
                    static_cast<SubstanceClass::type>(stoi(j["class_"].begin().key())));

        if (j.contains("limitsTP"))
            if (!j["limitsTP"].is_null())
            {
                if (j["limitsTP"].contains("lowerT"))
                    if (!j["limitsTP"]["lowerT"].is_null())
                        s.setLowerT(j["limitsTP"]["lowerT"].get<double>());
                if (j["limitsTP"].contains("upperT"))
                    if (!j["limitsTP"]["upperT"].is_null())
                        s.setUpperT(j["limitsTP"]["upperT"].get<double>());
            }

        if (j.contains("Tst"))
            if (!j["Tst"].is_null())
                s.setReferenceT(j["Tst"].get<double>());

        if (j.contains("Pst"))
            if (!j["Pst"].is_null())
                s.setReferenceP(j["Pst"].get<double>());

        // get temperature and pressure correction methods
        if (j.contains("TPMethods"))
            if (!j["TPMethods"].is_null())
                getTPMethods(j, s);
        // if no TPMethod is given but there is a reaction which defines the properties of the given substance
        if (!j.contains("TPMethods") && j.contains("reaction"))
            if (!j["reaction"].is_null())
                s.setThermoCalculationType(SubstanceThermoCalculationType::REACDC);

        // get thermodynamic parameters
        //    s.setThermoParameters(thermoParamSubst (object, name));
        // get reference thermodynamic properties
        s.setThermoReferenceProperties(thermoRefPropSubst(j));
    }
    catch (json::exception &ex)
    {
        thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
    }

    return s;
}

auto parseReaction(const std::string &data) -> Reaction
{
    Reaction r;
    std::string kbuf;
    std::vector<std::string> vkbuf;

    try
    {
        json j = json::parse(data);
        if (j.contains("properties"))
            if (!j["properties"].is_null())
                j = j["properties"];

        if (j.contains("name"))
            if (!j["name"].is_null())
                r.setName(j["name"]);

        if (j.contains("symbol"))
            if (!j["symbol"].is_null())
                r.setSymbol(j["symbol"]);

        if (j.contains("equation"))
            if (!j["equation"].is_null())
            {
                r.setEquation(j["equation"]);
                r.fromEquation(r.equation());
            }

        if (j.contains("limitsTP"))
            if (!j["limitsTP"].is_null())
            {
                if (j["limitsTP"].contains("lowerT"))
                    if (!j["limitsTP"]["lowerT"].is_null())
                        r.setLowerT(j["limitsTP"]["lowerT"].get<double>());
                if (j["limitsTP"].contains("upperT"))
                    if (!j["limitsTP"]["upperT"].is_null())
                        r.setUpperT(j["limitsTP"]["upperT"].get<double>());
            }

        if (j.contains("Tst"))
            if (!j["Tst"].is_null())
                r.setReferenceT(j["Tst"].get<double>());

        if (j.contains("Pst"))
            if (!j["Pst"].is_null())
                r.setReferenceP(j["Pst"].get<double>());

        if (j.contains("reactants"))
            if (!j["reactants"].is_null())
                r.setReactants(getReactants(j["reactants"]));

        // get temperature and pressure correction methods
        if (j.contains("TPMethods"))
            if (!j["TPMethods"].is_null())
                getTPMethods(j, r);

        // get thermodynamic parameters
        //    r.setThermoParameters(thermoParamReac (object, name));
        // get reference thermodynamic properties
        r.setThermoReferenceProperties(thermoRefPropReac(j));
    }
    catch (json::exception &ex)
    {
        thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
    }

    return r;
}

} // namespace ThermoFun
