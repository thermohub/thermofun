#include <algorithm>
#include <stdlib.h>

#include "ParseJsonToData.h"
#include "nlohmann/json.hpp"

#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"
#include "Common/Exception.h"

using json = nlohmann::json;

namespace ThermoFun {

auto getTPMethods(const json &j, Substance& s) -> void;
auto thermoParamSubst (const json& data, std::string name, std::string prop_name, ThermoParametersSubstance& ps) -> void;
/// Parses a bson object and loads reference thermodynamic properties into the internal sturcture
/// @param bso bson object
auto thermoRefPropSubst (const json &j, std::string name) -> ThermoPropertiesSubstance;

auto index_from_map (std::string map) -> int
{
    unsigned first = map.find("\"");
    unsigned second = map.find("\"", first+1);
    string strNew = map.substr (first+1,second-(first+1));
    return stoi(strNew);
}

bool extractMapFirst( const std::string& data,
                      const std::string& fieldPath, int& key, std::string& value )
{
//    std::map<std::string,std::string> class_map;
//    if( domData->findObject( fieldPath, class_map ) )
//    {
//        auto it = class_map.begin();
//        if( it != class_map.end() )
//        {
//            key = std::stoi(it->first);
//            value = it->second;
//            return true;
//        }
//    }
//    return false;
}

auto parseIssues(std::string data, string name, string prop) -> bool
{
    if ((data == "*") || (data == "" ) || (data == "{}"))
    {
        flog.open(parsinglogfile, ios::app);
        flog << "Could not parse " << prop << " for "<< name << ", using default value! " << endl;
        flog.close();
        return true;
    }
    else
        return false;
}

auto readValueError(const json& j, string propPath, double &val, double &err, string name, string message) -> Reaktoro_::StatusMessage
{
    string sval, serr;
    Reaktoro_::StatusMessage status = {Reaktoro_::Status::notdefined, message};

    if (j[propPath].contains("values"))
    {
        val = j["propPath"]["values"][0].get<double>();
        status = {Reaktoro_::Status::read, message};
    }

    if (j[propPath].contains("errors"))
    {
        err = j["propPath"]["errors"][0].get<double>();
    }
        
    return status;
}

auto databaseFromRecordList(const DatabaseClient &dbc, const List_VertexId_VertexType &recordList, DefinesSubstLevelOptions levelOptions ) -> Database
{
    string _idSubst, _idReact, substSymb;
    string level_;
    Database tdb;

    // The set of all aqueous species in the database
    SubstancesMap substances_map;
    // The set of all gaseous species in the database
    ReactionsMap reactions_map;

    // get substances and the reaction symbol if necessary
//    for(auto iterator = recordList.begin(); iterator != recordList.end(); iterator++)
//    {
//        if (iterator->second == "substance")
//        {
//            _idSubst = iterator->first;
//            string jsonrecord = dbc.substData().getJsonRecordVertex(_idSubst);
//            auto domdata = jsonio::unpackJson( jsonrecord, "VertexSubstance" ); // with default values
//            domdata->findValue("properties.symbol", substSymb);
//            // record = dbc.substData().getJsonBsonRecordVertex(_idSubst).second;
//            // bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

//            auto mode = levelOptions.definesSubstLevelMode;

//            switch(mode)
//            {
//            case DefinesSubstLevelMode::followAll        : level_ = "-1";  // follows all connected data
//                break;
//            case DefinesSubstLevelMode::followGiven      : level_ = levelOptions.level; // follows edges defines with level
//                break;
//            case DefinesSubstLevelMode::followDiefferentPerSubstance    : {
//                if (levelOptions.substSymbol_definesLevel.find(substSymb) != levelOptions.substSymbol_definesLevel.end()) // follows edges defines with specific leveles for substSymbols
//                    level_ = levelOptions.substSymbol_definesLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
//                else
//                    level_ = levelOptions.level;
//            }
//                break;
//            }

//            Substance substance = parseSubstance(domdata.get());

//            // get reaction symbol which define substance with _idSubst
//            string definesReactSymb = dbc.substData().definesReactionSymbol(_idSubst, level_);
//            if (definesReactSymb != "")
//            {
//                substance.setReactionSymbol(definesReactSymb);
//                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
//            }

//            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
//                substances_map[substance.symbol()] = substance;
//            } else {
//                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
//            }
//        } else
//            if (iterator->second == "reaction")
//            {
//                _idReact = iterator->first;
//                string jsonrecord = dbc.reactData().getJsonRecordVertex(_idReact);
//                auto domdata = jsonio::unpackJson( jsonrecord, "VertexReaction" ); // with default values
//                // record = dbc.reactData().getJsonBsonRecordVertex(_idReact).second;

//                Reaction reaction = ThermoFun::parseReaction(domdata.get());

//                // get reactants by following reaction incoming takes edge
//                reaction.setReactants(dbc.reactData().reactantsCoeff(_idReact));

//                if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
//                    reactions_map[reaction.symbol()] = reaction;
//                } else {
//                    errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
//                }
//            }
//    }
    tdb.addMapReactions(reactions_map);
    tdb.addMapSubstances(substances_map);
    return tdb;
}

auto parseElement (const std::string& data) -> Element
{
    Element e;
    string kbuf;
    string name;
    json j = json::parse(data);

    if (j.contains("name"))
         {e.setName(j["name"]); name = j["name"];}

    if (j.contains("symbol"))
          e.setSymbol(j["symbol"]);

    if (j.contains("entropy"))
          e.setEntropy(j["entropy"]["values"][0].get<double>());

    if (j.contains("heat_capacity"))
          e.setHeatCapacity(j["heat_capacity"]["values"][0].get<double>());

    if (j.contains("atomic_mass"))
         e.setMolarMass(j["atomic_mass"]["values"][0].get<double>());

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
    string name;
    json j = json::parse(data);

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
        s.setLowerT(j["limitsTP"]["lowerT"].get<double>());

    if (j.contains("limitsTP"))
        s.setUpperT(j["limitsTP"]["upperT"].get<double>());

    if (j.contains("limitsTP"))
        s.setLowerP(j["limitsTP"]["lowerP"].get<double>());

    if (j.contains("limitsTP"))
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
    s.setThermoReferenceProperties(thermoRefPropSubst (j, name));

    return s;
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

auto getTPMethods(const std::vector<std::string>& vkbuf) -> listmethods
{
    listmethods tpmethods;

//    for (auto kbuf : vkbuf)
//    {
//        auto jmethod = jsonio::unpackJson( kbuf );
//        int key;
//        Method method;
//        extractMapFirst(jmethod.get(), "method", key, method.name);
//        method.parameter_coefficients = getParameterCoefficients(/*jmethod.get(),*/ SubstanceTPMethodType(key));
//    }
    return tpmethods;
}

auto getTPMethods(/*const std::string& data,*/ const std::vector<std::string> &vkbuf, Reaction &r) -> void
{
    ThermoParametersReaction pr;
//    for (auto kbufx : vkbuf)
//    {
//        auto jmethod = jsonio::unpackJson(kbufx);
//        int key; std::string name;
//        extractMapFirst(jmethod.get(), "method", key, name);
//        setTPMethods_old(ReactionTPMethodType(key), r);
//        thermoParamReac(jmethod.get(), r.name(), pr);
//    }
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
        thermoParamSubst(it.value(), s.name(), name, ps);
    }

    if (j.contains("m_expansivity"))
        ps.isobaric_expansivity = j["m_expansivity"].get<float>();

    if (j.contains("m_compressibility"))
        ps.isothermal_compresibility = j["m_compressibility"].get<float>();

    s.setThermoParameters(ps);
}

auto thermoParamSubst (const json& j, std::string name, std::string prop_name, ThermoParametersSubstance& ps) -> void
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

    // temporary fix - need to think how to handle more than 1 TP interval
    if (prop_name == "cp_ft_equation")
    {
        std::vector<double> low_up;
        if (j.contains("limitsTP"))
        {
            if (j["limitsTP"].contains("lowerT"))
                low_up.push_back(j["limitsTP"]["lowerT"].get<double>());
            if (j["limitsTP"].contains("limitsTP"))
                low_up.push_back(j["limitsTP"]["lowerT"].get<double>());
        }
        ps.temperature_intervals.push_back(low_up);
    }

    if (j.contains("m_heat_capacity_ft_coeffs"))
    {
        ps.Cp_coeff.push_back(j["m_heat_capacity_ft_coeffs"]["values"].get<vector<double>>());
    }

    if (j.contains("m_phase_trans_props"))
    {
        ps.phase_transition_prop.push_back(j["m_phase_trans_props"]["values"].get<vector<double>>());
    }

    if (j.contains("m_landau_phase_trans_props"))
    {
        ps.phase_transition_prop.push_back(j["m_landau_phase_trans_props"]["values"].get<vector<double>>());
    }

    if (j.contains("phase_transition_prop_Berman"))
    {
        ps.phase_transition_prop_Berman.push_back(j["phase_transition_prop_Berman"]["values"].get<vector<double>>());
    }
}

auto thermoRefPropSubst (const json& j, string name) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string idSubst, message;

    if (j.contains("_id"))
        idSubst = j["_id"];

    if (!parseIssues(idSubst, name, _id)) message = "_id : " + idSubst;

    tps.heat_capacity_cp.sta = readValueError(j, "sm_gibbs_energy", tps.heat_capacity_cp.val, tps.heat_capacity_cp.err, name, message);
    tps.gibbs_energy.sta     = readValueError(j, "sm_entropy_abs",  tps.gibbs_energy.val,     tps.gibbs_energy.err,     name, message);
    tps.enthalpy.sta         = readValueError(j, "sm_enthalpy",  tps.enthalpy.val,         tps.enthalpy.err,         name, message);
    tps.entropy.sta          = readValueError(j, "sm_volume",  tps.entropy.val,          tps.entropy.err,          name, message);
    tps.volume.sta           = readValueError(j, "sm_heat_capacity_p",  tps.volume.val,           tps.volume.err,           name, message);

    return tps;
}


auto parseReaction (const std::string& data) -> Reaction
{
    Reaction r;
    string kbuf;
    vector<string> vkbuf;
    string name;

//    if (object->findValue( reacName, kbuf ))
//        if (!parseIssues(kbuf, name, reacName)) { r.setName(kbuf); name = kbuf;}

//    if (object->findValue( reacSymbol, kbuf ))
//        if (!parseIssues(kbuf, name, reacSymbol)) r.setSymbol(kbuf);

//    if (object->findValue( reacEquation, kbuf ))
//        if (!parseIssues(kbuf, name, reacEquation)) r.setEquation(kbuf);

//    if (object->findValue( reacMethodEOS, kbuf ))
//        if (!parseIssues(kbuf, name, reacMethodEOS)) r.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

//    if (object->findValue( reacMethodT, kbuf ))
//        if (!parseIssues(kbuf, name, reacMethodT)) r.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

//    if (object->findValue( reacMethodP, kbuf ))
//        if (!parseIssues(kbuf, name, reacMethodP)) r.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

//    if (object->findValue( reacRefT, kbuf ))
//        if (!parseIssues(kbuf, name, reacRefT)) r.setReferenceT(std::stod(kbuf.c_str()));

//    if (object->findValue( reacRefP, kbuf ))
//        if (!parseIssues(kbuf, name, reacRefP)) r.setReferenceP(std::stod(kbuf.c_str()));

//    if (object->findValue( lowerT, kbuf ))
//        if (!parseIssues(kbuf, name, lowerT)) r.setLowerT(std::stod(kbuf.c_str()));

//    if (object->findValue( upperT, kbuf ))
//        if (!parseIssues(kbuf, name, upperT)) r.setUpperT(std::stod(kbuf.c_str()));

//    if (object->findValue( lowerP, kbuf ))
//        if (!parseIssues(kbuf, name, lowerP)) r.setLowerP(std::stod(kbuf.c_str()));

//    if (object->findValue( upperP, kbuf ))
//        if (!parseIssues(kbuf, name, upperP)) r.setUpperP(std::stod(kbuf.c_str()));

//    // get temperature and pressure coreection methods
//    if (object->findArray(TPMethods, vkbuf))
//        getTPMethods(/*object,*/ vkbuf, r);

//    // get thermodynamic parameters
////    r.setThermoParameters(thermoParamReac (object, name));
//    // get reference thermodynamic properties
//    r.setThermoReferenceProperties(thermoRefPropReac (object, name));

    return r;
}

auto thermoParamReac (const std::string& data, std::string name, ThermoParametersReaction& pr) -> void
{
    vector<string> vkbuf;
    string kbuf;

//    if (object->findArray(  reacLogKfT, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKfT))
//    {
//        pr.reaction_logK_fT_coeff.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_logK_fT_coeff.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }

//    if (object->findArray( reacLogKPT, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKPT))
//    {
//        pr.logK_TP_array.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.logK_TP_array.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }

//    if (object->findArray( reacDrCpfT, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrCpfT))
//    {
//        pr.reaction_Cp_fT_coeff.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_Cp_fT_coeff.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }

//    if (object->findArray( reacDrVfT, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrVfT))
//    {
//        pr.reaction_V_fT_coeff.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_V_fT_coeff.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }

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

//    if (object->findArray(  reacRBcoeff, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacRBcoeff))
//    {
//        pr.reaction_RB_coeff.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_RB_coeff.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }

//    if (object->findArray( reacFMcoeff, vkbuf))
//        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacFMcoeff))
//    {
//        pr.reaction_FM_coeff.resize(vkbuf.size());
//        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_FM_coeff.begin(), [](const std::string& val)
//        { return std::stod(val); });
//    }
////    return pr;
}

auto thermoRefPropReac (const std::string& data, string name) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    string message, idReac;

//    if (object->findValue( _id, idReac ))
//        if (!parseIssues(idReac, name, _id)) message = "_id : " + idReac;

//    tpr.log_equilibrium_constant.sta  = readValueError(object, reacRefLogK0_, tpr.log_equilibrium_constant.val,  tpr.log_equilibrium_constant.err,  name, message);
//    tpr.reaction_heat_capacity_cp.sta = readValueError(object, reacRefCp0_,   tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, name, message);
//    tpr.reaction_gibbs_energy.sta     = readValueError(object, reacRefG0_,    tpr.reaction_gibbs_energy.val,     tpr.reaction_gibbs_energy.err,     name, message);
//    tpr.reaction_enthalpy.sta         = readValueError(object, reacRefH0_,    tpr.reaction_enthalpy.val,         tpr.reaction_enthalpy.err,         name, message);
//    tpr.reaction_entropy.sta          = readValueError(object, reacRefS0_,    tpr.reaction_entropy.val,          tpr.reaction_entropy.err,          name, message);
//    tpr.reaction_volume.sta           = readValueError(object, reacRefV0_,    tpr.reaction_volume.val,           tpr.reaction_volume.err,           name, message);

    return tpr;
}

//auto parseElement (const std::string &data) -> Element
//{
////  auto domdata = jsonio::unpackJson( data, "VertexElement" ); // with default values
////  return parseElement(domdata.get());
//}

//auto parseSubstance (const std::string& data) -> Substance
//{
////  auto domdata = jsonio::unpackJson( data, "VertexSubstance" ); // with default values
////  return parseSubstance(domdata.get());
//}

//auto parseReaction (const std::string& data) -> Reaction
//{
//// auto domdata = jsonio::unpackJson( data, "VertexReaction" ); // with default values
//// return parseReaction(domdata.get());
//}


}
