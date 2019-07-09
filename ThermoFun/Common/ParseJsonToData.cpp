// C++ includes
#include <algorithm>
#include <stdlib.h>

// ThermoFun includes
#include "ParseJsonToData.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"
#include "Common/Exception.h"

// JSON
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ThermoFun {

auto getTPMethods(const json &j, Substance& s) -> void;
auto getTPMethods(const json &j, Reaction& r) -> void;
auto thermoParamSubst (const json& j, std::string prop_name, ThermoParametersSubstance& ps) -> void;
auto thermoParamReac (const json& j, ThermoParametersReaction& pr) -> void;

auto thermoRefPropSubst (const json &j) -> ThermoPropertiesSubstance;
auto thermoRefPropReac (const json &j) -> ThermoPropertiesReaction;

//auto index_from_map (std::string map) -> int
//{
//    unsigned first = map.find("\"");
//    unsigned second = map.find("\"", first+1);
//    string strNew = map.substr (first+1,second-(first+1));
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

//auto parseIssues(std::string data, string name, string prop) -> bool
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

auto readValueError(const json& j, string propPath, double &val, double &err, string message) -> Reaktoro_::StatusMessage
{
    string sval, serr;
    Reaktoro_::StatusMessage status = {Reaktoro_::Status::notdefined, message};

    if (j[propPath].contains("values"))
    {
        val = j[propPath]["values"][0].get<double>();
        status = {Reaktoro_::Status::read, message};
    }

    if (j[propPath].contains("errors"))
    {
        err = j[propPath]["errors"][0].get<double>();
    }

    return status;
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
        break;       // and exits the switch
    case SubstanceTPMethodType::solute_hkf88_reaktoro:
        cout << '2';
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

auto getTPMethods(const json &j, Reaction& r) -> void
{
    ThermoParametersReaction pr;
    json methods = j["TPMethods"];

    for(auto it = methods.begin(); it != methods.end(); ++it)
    {
        int key = stoi(it.value()["method"].begin().key());
        std::string name = it.value()["method"].begin().value();
        setTPMethods_old(ReactionTPMethodType(key), r);
        thermoParamReac(it.value(), pr);
    }
    r.setThermoParameters(pr);
}

auto getTPMethods(const json& j, Substance &s) -> void
{
    ThermoParametersSubstance ps;
    json methods = j["TPMethods"];

    for(auto it = methods.begin(); it != methods.end(); ++it)
    {
        int key = stoi(it.value()["method"].begin().key());
        std::string name = it.value()["method"].begin().value();
        setTPMethods_old(SubstanceTPMethodType(key), s);
        thermoParamSubst(it.value(), name, ps);
    }

    if (j.contains("m_expansivity"))
        ps.isobaric_expansivity = j["m_expansivity"]["values"][0].get<double>();

    if (j.contains("m_compressibility"))
        ps.isothermal_compresibility = j["m_compressibility"]["values"][0].get<double>();

    s.setThermoParameters(ps);
}

auto thermoParamSubst (const json& j, std::string prop_name, ThermoParametersSubstance& ps) -> void
{
    vector<string> vkbuf;
    string kbuf;

    if (j.contains("eos_akinfiev_diamond_coeffs"))
        ps.Cp_nonElectrolyte_coeff = j["eos_akinfiev_diamond_coeffs"]["values"].get<vector<double>>();

    if (j.contains("eos_birch_murnaghan_coeffs"))
        ps.volume_BirchM_coeff = j["eos_birch_murnaghan_coeffs"]["values"].get<vector<double>>();

//    if (j.contains("eos_churakov_gottschalk_coeffs"))s.resize(vkbuf.size());

    if (j.contains("eos_gas_crit_props"))
        ps.critical_parameters = j["eos_gas_crit_props"]["values"].get<vector<double>>();

    if (j.contains("eos_hkf_coeffs"))
        ps.HKF_parameters = j["eos_hkf_coeffs"]["values"].get<vector<double>>();

    // temporary fix - need to think how to handle more than 1 TP interval - for new structure - simplified
    if (prop_name == "cp_ft_equation")
    {
        std::vector<double> low_up;
        if (j.contains("limitsTP"))
        {
            if (j["limitsTP"].contains("lowerT"))
                low_up.push_back(j["limitsTP"]["lowerT"].get<double>());
            if (j["limitsTP"].contains("upperT"))
                low_up.push_back(j["limitsTP"]["upperT"].get<double>());
        }
        ps.temperature_intervals.push_back(low_up);
    }

    if (j.contains("m_heat_capacity_ft_coeffs"))
        ps.Cp_coeff.push_back(j["m_heat_capacity_ft_coeffs"]["values"].get<vector<double>>());

    if (j.contains("m_phase_trans_props"))
        ps.phase_transition_prop.push_back(j["m_phase_trans_props"]["values"].get<vector<double>>());

    if (j.contains("m_landau_phase_trans_props"))
        ps.phase_transition_prop.push_back(j["m_landau_phase_trans_props"]["values"].get<vector<double>>());

    if (j.contains("phase_transition_prop_Berman"))
        ps.phase_transition_prop_Berman.push_back(j["phase_transition_prop_Berman"]["values"].get<vector<double>>());
}

auto thermoParamReac (const json &j, ThermoParametersReaction& pr) -> void
{
    vector<string> vkbuf;
    string kbuf;

    if (j.contains("logk_ft_coeffs"))
        pr.reaction_logK_fT_coeff = j["logk_ft_coeffs"]["values"].get<vector<double>>();
//    if (j.contains("logk_pt_values"))  // static const char * reacLogKPT             = "logk_pt_values.pptv"; //
//        pr.logK_TP_array = j["logk_pt_values"]["values"].get<vector<double>>();
    if (j.contains("dr_heat_capacity_ft_coeffs"))
        pr.reaction_Cp_fT_coeff = j["dr_heat_capacity_ft_coeffs"]["values"].get<vector<double>>();
    if (j.contains("dr_volume_fpt_coeffs"))
        pr.reaction_V_fT_coeff = j["dr_volume_fpt_coeffs"]["values"].get<vector<double>>();
    if (j.contains("dr_ryzhenko_coeffs"))
        pr.reaction_RB_coeff = j["dr_ryzhenko_coeffs"]["values"].get<vector<double>>();
    if (j.contains("dr_marshall_franck_coeffs"))
        pr.reaction_FM_coeff = j["dr_marshall_franck_coeffs"]["values"].get<vector<double>>();
    if (j.contains("dr_dolejs_manning10_coeffs"))
        pr.reaction_DM10_coeff = j["dr_dolejs_manning10_coeffs"]["values"].get<vector<double>>();


//    double lT = 0.0; double uT = 0.0;
//    double lP = 0.0; double uP = 0.0;
//    if (object->findValue( lowerT, kbuf ))
//        if (!parseIssues(kbuf, name, lowerT)) lT = std::stod(kbuf.c_str());
//    if (object->findValue( upperT, kbuf ))
//        if (!parseIssues(kbuf, name, upperT)) uT = std::stod(kbuf.c_str());
//    pr.temperature_intervals.push_back({lT, uT});

//    if (object->findValue( lowerP, kbuf ))
//        if (!parseIssues(kbuf, name, lowerP)) lT = std::stod(kbuf.c_str());
//    if (object->findValue( upperP, kbuf ))
//        if (!parseIssues(kbuf, name, upperP)) uT = std::stod(kbuf.c_str());
//    pr.pressure_intervals.push_back({lT, uT});

}

auto thermoRefPropSubst (const json& j) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string message;

    if (j.contains("sm_heat_capacity_p"))
        tps.heat_capacity_cp.sta = readValueError(j, "sm_heat_capacity_p" , tps.heat_capacity_cp.val, tps.heat_capacity_cp.err,  message);
    if (j.contains("sm_gibbs_energy"))
        tps.gibbs_energy.sta     = readValueError(j, "sm_gibbs_energy",  tps.gibbs_energy.val,     tps.gibbs_energy.err,      message);
    if (j.contains("sm_enthalpy"))
        tps.enthalpy.sta         = readValueError(j, "sm_enthalpy",  tps.enthalpy.val,         tps.enthalpy.err,          message);
    if (j.contains("sm_entropy_abs"))
        tps.entropy.sta          = readValueError(j, "sm_entropy_abs",  tps.entropy.val,          tps.entropy.err,           message);
    if (j.contains("sm_volume"))tps.volume.sta           = readValueError(j, "sm_volume",  tps.volume.val,           tps.volume.err,            message);

    return tps;
}

auto thermoRefPropReac (const json &j) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    string message;

    if (j.contains("logKr"))
        tpr.log_equilibrium_constant.sta  = readValueError(j, "logKr", tpr.log_equilibrium_constant.val,  tpr.log_equilibrium_constant.err,  message);
    if (j.contains("drsm_heat_capacity_p"))
        tpr.reaction_heat_capacity_cp.sta = readValueError(j, "drsm_heat_capacity_p",   tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, message);
    if (j.contains("drsm_gibbs_energy"))
        tpr.reaction_gibbs_energy.sta     = readValueError(j, "drsm_gibbs_energy",    tpr.reaction_gibbs_energy.val,     tpr.reaction_gibbs_energy.err,     message);
    if (j.contains("drsm_enthalpy"))
        tpr.reaction_enthalpy.sta         = readValueError(j, "drsm_enthalpy",    tpr.reaction_enthalpy.val,         tpr.reaction_enthalpy.err,         message);
    if (j.contains("drsm_entropy"))
        tpr.reaction_entropy.sta          = readValueError(j, "drsm_entropy",    tpr.reaction_entropy.val,          tpr.reaction_entropy.err,          message);
    if (j.contains("drsm_volume"))tpr.reaction_volume.sta           = readValueError(j, "drsm_volume",    tpr.reaction_volume.val,           tpr.reaction_volume.err,           message);

    return tpr;
}

auto parseElement (const std::string& data) -> Element
{
    Element e;
    string kbuf;
    string name;
    json j = json::parse(data);
    if (j.contains("properties"))
        j = j["properties"];

    if (j.contains("name"))
         {e.setName(j["name"]); name = j["name"];}

    if (j.contains("symbol"))
          e.setSymbol(j["symbol"]);

    if (j.contains("number"))
          e.setNumber(j["number"].get<int>());

    if (j.contains("entropy"))
          e.setEntropy(j["entropy"]["values"][0].get<double>());

    if (j.contains("heat_capacity"))
          e.setHeatCapacity(j["heat_capacity"]["values"][0].get<double>());

    if (j.contains("atomic_mass"))
         e.setMolarMass(j["atomic_mass"]["values"][0].get<double>());

    if (j.contains("volume"))
          e.setVolume(j["volume"]["values"][0].get<double>());

    if (j.contains("class_"))
         e.setClass(stoi(j["class_"].begin().key()));

    if (j.contains("isotope_mass"))
         e.setIsotopeMass(j["isotope_mass"].get<int>());

    return e;
}

auto parseSubstance (const std::string& data) -> Substance
{
    Substance s;
    vector<string> vkbuf;
    string kbuf;
    json j = json::parse(data);
    if (j.contains("properties"))
        j = j["properties"];

    if (j.contains("name"))
        s.setName(j["name"]);

    if (j.contains("symbol"))
        s.setSymbol(j["symbol"]);

    if (j.contains("formula"))
        s.setFormula(j["formula"]);

    if (j.contains("formula_charge"))
        s.setCharge(j["formula_charge"].get<int>());

    if (j.contains("mass_per_mole"))
        s.setMolarMass(j["mass_per_mole"].get<double>());

    if (j.contains("aggregate_state"))
        s.setAggregateState(
                    static_cast<AggregateState::type>(stoi(j["aggregate_state"].begin().key())));

    if (j.contains("class_"))
        s.setSubstanceClass(
                    static_cast<SubstanceClass::type>(stoi(j["class_"].begin().key())));

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("lowerT"))
            s.setLowerT(j["limitsTP"]["lowerT"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("upperT"))
            s.setUpperT(j["limitsTP"]["upperT"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("lowerP"))
            s.setLowerP(j["limitsTP"]["lowerP"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("upperP"))
            s.setUpperP(j["limitsTP"]["upperP"].get<double>());

    if (j.contains("Tst"))
        s.setReferenceT(j["Tst"].get<double>());

    if (j.contains("Pst"))
        s.setReferenceP(j["Pst"].get<double>());

    // get temperature and pressure correction methods
    if (j.contains("TPMethods"))
        getTPMethods(j, s);

    // get thermodynamic parameters
//    s.setThermoParameters(thermoParamSubst (object, name));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefPropSubst (j));

    return s;
}


auto parseReaction (const std::string& data) -> Reaction
{
    Reaction r;
    string kbuf;
    vector<string> vkbuf;
    json j = json::parse(data);
    if (j.contains("properties"))
        j = j["properties"];

    if (j.contains("name"))
        r.setName(j["name"]);

    if (j.contains("symbol"))
        r.setSymbol(j["symbol"]);

    if (j.contains("equation"))
        r.setEquation(j["equation"]);

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("lowerT"))
            r.setLowerT(j["limitsTP"]["lowerT"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("upperT"))
            r.setUpperT(j["limitsTP"]["upperT"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("lowerP"))
            r.setLowerP(j["limitsTP"]["lowerP"].get<double>());

    if (j.contains("limitsTP"))
        if (j["limitsTP"].contains("upperP"))
            r.setUpperP(j["limitsTP"]["upperP"].get<double>());

    if (j.contains("Tst"))
        r.setReferenceT(j["Tst"].get<double>());

    if (j.contains("Pst"))
        r.setReferenceP(j["Pst"].get<double>());

    // get temperature and pressure correction methods
    if (j.contains("TPMethods"))
        getTPMethods(j, r);

    // get thermodynamic parameters
//    r.setThermoParameters(thermoParamReac (object, name));
    // get reference thermodynamic properties
    r.setThermoReferenceProperties(thermoRefPropReac(j));

    return r;
}

}
