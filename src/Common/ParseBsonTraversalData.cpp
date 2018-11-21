#include <algorithm>

#include "ParseBsonTraversalData.h"
#include "Database.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"
#include "DBClient/DatabaseClient.h"
#include "DBClient/SubstanceData.h"
#include "DBClient/ReactionData.h"

// jsonio includes
#include "jsonio/jsondomschema.h"
#include "jsonio/jsondomfree.h"
#include "Common/Exception.h"

namespace ThermoFun {

auto index_from_map (std::string map) -> int
{
    unsigned first = map.find("\"");
    unsigned second = map.find("\"", first+1);
    string strNew = map.substr (first+1,second-(first+1));
    return stoi(strNew);
}

bool extractMapFirst( const jsonio::JsonDomFree* domData,
                      const std::string& fieldPath, int& key, std::string& value )
{
    std::map<std::string,std::string> class_map;
    if( domData->findObject( fieldPath, class_map ) )
    {
        auto it = class_map.begin();
        if( it != class_map.end() )
        {
            key = std::stoi(it->first);
            value = it->second;
            return true;
        }
    }
    return false;
}

auto parseIssues(std::string data, string name, string prop) -> bool
{
    if ((data == "*") || (data == ""))
    {
        flog.open(parsinglogfile, ios::app);
        flog << "Could not parse " << prop << " for "<< name << ", using default value! " << endl;
        flog.close();
        return true;
    }
    else
        return false;
}

auto readValueError(const jsonio::JsonDom *object, string propPath, double &val, double &err, string name, string message) -> Reaktoro_::StatusMessage
{
    string sval, serr;
    Reaktoro_::StatusMessage status = {Reaktoro_::Status::notdefined, message};

    if (object->findValue( propPath+".values.0", sval ))
        if (!parseIssues(sval, name, propPath+".values.0")) {
        val = (std::stod(sval.c_str()));
        status = {Reaktoro_::Status::read, message};;
    }
    if (object->findValue(  propPath+".errors.0", serr ))
        if (!parseIssues(serr, name, propPath+".errors.0")) {
        err = (std::stod(serr.c_str()));
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
    for(auto iterator = recordList.begin(); iterator != recordList.end(); iterator++)
    {
        if (iterator->second == "substance")
        {
            _idSubst = iterator->first;
            string jsonrecord = dbc.substData().getJsonRecordVertex(_idSubst);
            auto domdata = jsonio::unpackJson( jsonrecord, "VertexSubstance" ); // with default values
            domdata->findValue("properties.symbol", substSymb);
            // record = dbc.substData().getJsonBsonRecordVertex(_idSubst).second;
            // bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

            auto mode = levelOptions.definesSubstLevelMode;

            switch(mode)
            {
            case DefinesSubstLevelMode::followAll        : level_ = "-1";  // follows all connected data
                break;
            case DefinesSubstLevelMode::followGiven      : level_ = levelOptions.level; // follows edges defines with level
                break;
            case DefinesSubstLevelMode::followDiefferentPerSubstance    : {
                if (levelOptions.substSymbol_definesLevel.find(substSymb) != levelOptions.substSymbol_definesLevel.end()) // follows edges defines with specific leveles for substSymbols
                    level_ = levelOptions.substSymbol_definesLevel[substSymb];   // if the substance symbol is not found in the map, it uses the default level
                else
                    level_ = levelOptions.level;
            }
                break;
            }

            Substance substance = parseSubstance(domdata.get());

            // get reaction symbol which define substance with _idSubst
            string definesReactSymb = dbc.substData().definesReactionSymbol(_idSubst, level_);
            if (definesReactSymb != "")
            {
                substance.setReactionSymbol(definesReactSymb);
                substance.setThermoCalculationType(ThermoFun::SubstanceThermoCalculationType::type::REACDC);
            }

            if ( substances_map.find(substance.symbol()) == substances_map.end() ) {
                substances_map[substance.symbol()] = substance;
            } else {
                errorSameSymbol("substance", substance.symbol(), __LINE__, __FILE__ );
            }
        } else
            if (iterator->second == "reaction")
            {
                _idReact = iterator->first;
                string jsonrecord = dbc.reactData().getJsonRecordVertex(_idReact);
                auto domdata = jsonio::unpackJson( jsonrecord, "VertexReaction" ); // with default values
                // record = dbc.reactData().getJsonBsonRecordVertex(_idReact).second;

                Reaction reaction = ThermoFun::parseReaction(domdata.get());

                // get reactants by following reaction incoming takes edge
                reaction.setReactants(dbc.reactData().reactantsCoeff(_idReact));

                if ( reactions_map.find(reaction.symbol()) == reactions_map.end() ) {
                    reactions_map[reaction.symbol()] = reaction;
                } else {
                    errorSameSymbol("reaction", reaction.symbol(), __LINE__, __FILE__ );
                }
            }
    }
    tdb.addMapReactions(reactions_map);
    tdb.addMapSubstances(substances_map);
    return tdb;
}

auto parseElement (const jsonio::JsonDom *object) -> Element
{
    Element e;
    string kbuf;
    string name;

    if (object->findValue( elemName, kbuf ))
        if (!parseIssues(kbuf, name, elemName)) { e.setName(kbuf); name = kbuf;}

    if (object->findValue( elemSymbol, kbuf ))
        if (!parseIssues(kbuf, name, elemSymbol)) { e.setSymbol(kbuf);}

    string entropy_ = elemEntropy;
    entropy_ += ".values.0";
    if (object->findValue( entropy_, kbuf ))
        if (!parseIssues(kbuf, name, entropy_.c_str())) { e.setEntropy(atof(kbuf.c_str()));}

    string cp_ = elemHeatCapacity;
    cp_ += ".values.0";
    if (object->findValue( cp_, kbuf ))
        if (!parseIssues(kbuf, name, cp_.c_str())) { e.setHeatCapacity(atof(kbuf.c_str()));}

    string mass_ = elemMolarMass;
    mass_ += ".values.0";
    if (object->findValue( mass_, kbuf ))
        if (!parseIssues(kbuf, name, mass_.c_str())) { e.setMolarMass(atof(kbuf.c_str()));}

    if (object->findValue( elemClass, kbuf ))
        if (!parseIssues(kbuf, name, elemClass)) { e.setClass(index_from_map(kbuf));}

    if (object->findValue( elemIsotopeMass, kbuf ))
        if (!parseIssues(kbuf, name, elemIsotopeMass)) { e.setIsotopeMass(atof(kbuf.c_str()));}

    return e;
}

auto parseSubstance (const jsonio::JsonDom *object) -> Substance
{
    Substance s;
    vector<string> vkbuf;
    string kbuf;
    string name;

    if (object->findValue( substName, kbuf ))
        if (!parseIssues(kbuf, name, substName)) { s.setName(kbuf); name = kbuf;}

    if (object->findValue( substSymbol, kbuf ))
        if (!parseIssues(kbuf, name, substSymbol)) s.setSymbol(kbuf);

    if (object->findValue( substFormula, kbuf ))
        if (!parseIssues(kbuf, name, substFormula )) s.setFormula(kbuf);

    if (object->findValue( substCharge, kbuf ))
        if (!parseIssues(kbuf, name, substCharge)) s.setCharge(atoi(kbuf.c_str()));

    if (object->findValue( substMolarMass, kbuf ))
        if (!parseIssues(kbuf, name, substMolarMass)) s.setMolarMass(atof(kbuf.c_str()));

    if (object->findValue( substAggState, kbuf ))
        if (!parseIssues(kbuf, name, substAggState)) s.setAggregateState(AggregateState::type(index_from_map(kbuf)));

    if (object->findValue( substClass, kbuf ))
        if (!parseIssues(kbuf, name, substClass)) s.setSubstanceClass(SubstanceClass::type(index_from_map(kbuf)));

    if (object->findValue( lowerT, kbuf ))
        if (!parseIssues(kbuf, name, lowerT)) s.setLowerT(std::stod(kbuf.c_str()));

    if (object->findValue( upperT, kbuf ))
        if (!parseIssues(kbuf, name, upperT)) s.setUpperT(std::stod(kbuf.c_str()));

    if (object->findValue( lowerP, kbuf ))
        if (!parseIssues(kbuf, name, lowerP)) s.setLowerP(std::stod(kbuf.c_str()));

    if (object->findValue( upperP, kbuf ))
        if (!parseIssues(kbuf, name, upperP)) s.setUpperP(std::stod(kbuf.c_str()));

//    bsonio::bson_to_key( data, substSolventNname, kbuf );
//    if (!parseIssues(kbuf, name, substSolventNname)) s.setSolventSymbol(kbuf);

//    object->findValue( substMethodEOS, kbuf );
//    if (!parseIssues(kbuf, name, substMethodEOS)) s.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

//    object->findValue( substMethodT, kbuf );
//    if (!parseIssues(kbuf, name, substMethodT)) s.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

//    object->findValue( substMethodP, kbuf );
//    if (!parseIssues(kbuf, name, substMethodP)) s.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    if (object->findValue( substRefT, kbuf ))
        if (!parseIssues(kbuf, name, substRefT)) s.setReferenceT(std::stod(kbuf.c_str()));

    if (object->findValue( substRefP, kbuf ))
        if (!parseIssues(kbuf, name, substRefP)) s.setReferenceP(std::stod(kbuf.c_str()));

    // get temperature and pressure coreection methods
    if (object->findArray(TPMethods, vkbuf))
        getTPMethods(object, vkbuf, s);

    // get thermodynamic parameters
//    s.setThermoParameters(thermoParamSubst (object, name));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefPropSubst (object, name));

    return s;
}

auto getParameterCoefficients(const jsonio::JsonDom *object, const SubstanceTPMethodType &type) -> std::unordered_map<std::string, std::vector<double>>
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
        s.setMethodGenEoS(MethodGenEoS_Thrift::type(new_old_methodtype.at(type)));
        break;
    case SubstanceTPMethodType::landau_holland_powell98:
    case SubstanceTPMethodType::landau_berman88:
    case SubstanceTPMethodType::cp_ft_equation_saxena86:
    case SubstanceTPMethodType::water_eos_hgk84_lvs83_gems:
    case SubstanceTPMethodType::water_eos_iapws95_gems:
    case SubstanceTPMethodType::water_eos_hgk84_reaktoro:
    case SubstanceTPMethodType::water_eos_iapws95_reaktoro:
    case SubstanceTPMethodType::water_pvt_zhang_duan05:
        s.setMethod_T(MethodCorrT_Thrift::type(new_old_methodtype.at(type)));
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
        s.setMethod_P(MethodCorrP_Thrift::type(new_old_methodtype.at(type)));
        break;
    }

    // throw error method not found
}

auto getTPMethods(const std::vector<std::string>& vkbuf) -> listmethods
{
    listmethods tpmethods;

    for (auto kbuf : vkbuf)
    {
        auto jmethod = jsonio::unpackJson( kbuf );
        int key;
        Method method;
        extractMapFirst(jmethod.get(), "method", key, method.name);
        method.parameter_coefficients = getParameterCoefficients(jmethod.get(), SubstanceTPMethodType(key));
    }
    return tpmethods;
}

auto getTPMethods(const jsonio::JsonDom *object, const std::vector<std::string> &vkbuf, Substance &s) -> void
{
    ThermoParametersSubstance ps;
    for (auto kbufx : vkbuf)
    {
        auto jmethod = jsonio::unpackJson(kbufx);
        int key; std::string name;
        extractMapFirst(jmethod.get(), "method", key, name);
        setTPMethods_old(SubstanceTPMethodType(key), s);
        thermoParamSubst(jmethod.get(), s.name(), ps);
    }
    string expans = substExpans_ ; expans += ".values.0";
    string compres = substCompres_ ; compres += ".values.0";
    string kbuf;
    if (object->findValue( expans, kbuf))
        if (!parseIssues(kbuf, s.name(), expans))  ps.isobaric_expansivity = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    if (object->findValue( compres, kbuf))
        if (!parseIssues(kbuf, s.name(), compres))  ps.isothermal_compresibility = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;
    s.setThermoParameters(ps);
}

auto thermoParamSubst (const jsonio::JsonDom *object, std::string name, ThermoParametersSubstance& ps) -> void
{
    vector<string> vkbuf;
    string kbuf;

    if (object->findArray(  substEOSad, vkbuf))
        if ((vkbuf.size() > 0)&& (ps.Cp_nonElectrolyte_coeff.size()==0)) if (!parseIssues(vkbuf[0], name, substEOSad))
    {
        ps.Cp_nonElectrolyte_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_nonElectrolyte_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray(  substEOSbm, vkbuf))
        if ((vkbuf.size() > 0)&& (ps.volume_BirchM_coeff.size()==0)) if (!parseIssues(vkbuf[0], name, substEOSbm))
    {
        ps.volume_BirchM_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.volume_BirchM_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray(  substEOScg, vkbuf)) {} /*ps.critical_parameters.resize(vkbuf.size());*/
//        std::transform(kbuf.begin(), kbuf.end(), tps..begin(), [](const std::string& val)
//    { return std::stod(val); });

    if (object->findArray(  substEOSgasCrit, vkbuf))
        if ((vkbuf.size() > 0) && (ps.critical_parameters.size()==0)) if (!parseIssues(vkbuf[0], name, substEOSgasCrit))
    {
        ps.critical_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.critical_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( substEOShkf, vkbuf))
        if ((vkbuf.size() > 0)&& (ps.HKF_parameters.size()==0)) if (!parseIssues(vkbuf[0], name, substEOShkf))
    {
        ps.HKF_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.HKF_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    // temporary fix - need to think how to handle more than 1 TP interval
    ps.temperature_intervals.push_back({273.15, 2273.15});

    if (object->findArray( substCpParam, vkbuf))
        if ((vkbuf.size() > 0) && (ps.Cp_coeff.size()==0)) if (!parseIssues(vkbuf[0], name, substCpParam))
    {
        ps.Cp_coeff.resize(1); ps.Cp_coeff[0].resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_coeff[0].begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( substTransProp, vkbuf))
        if ((vkbuf.size() > 0) && (ps.phase_transition_prop.size()==0)) if (!parseIssues(vkbuf[0], name, substTransProp))
    {
        ps.phase_transition_prop.resize(1); ps.phase_transition_prop[0].resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop[0].begin(), [](const std::string& val)
        { return std::stod(val); });
    }

//    bsonio::bson_read_array_path(data, substTransProp, vkbuf); ps.phase_transition_prop_Berman.resize(1); ps.phase_transition_prop_Berman[0].resize(vkbuf.size());
//    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop_Berman[0].begin(), [](const std::string& val)
//    { return std::stod(val); });

//    return ps;
}

auto thermoRefPropSubst (const jsonio::JsonDom *object, string name) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string idSubst, message;

    if (object->findValue( _id, idSubst ))
        if (!parseIssues(idSubst, name, _id)) message = "_id : " + idSubst;

    tps.heat_capacity_cp.sta = readValueError(object, substRefCp0_, tps.heat_capacity_cp.val, tps.heat_capacity_cp.err, name, message);
    tps.gibbs_energy.sta     = readValueError(object, substRefG0_,  tps.gibbs_energy.val,     tps.gibbs_energy.err,     name, message);
    tps.enthalpy.sta         = readValueError(object, substRefH0_,  tps.enthalpy.val,         tps.enthalpy.err,         name, message);
    tps.entropy.sta          = readValueError(object, substRefS0_,  tps.entropy.val,          tps.entropy.err,          name, message);
    tps.volume.sta           = readValueError(object, substRefV0_,  tps.volume.val,           tps.volume.err,           name, message);

    return tps;
}


auto parseReaction (const jsonio::JsonDom *object) -> Reaction
{
    Reaction r;
    string kbuf;
    string name;

    if (object->findValue( reacName, kbuf ))
        if (!parseIssues(kbuf, name, reacName)) { r.setName(kbuf); name = kbuf;}

    if (object->findValue( reacSymbol, kbuf ))
        if (!parseIssues(kbuf, name, reacSymbol)) r.setSymbol(kbuf);

    if (object->findValue( reacEquation, kbuf ))
        if (!parseIssues(kbuf, name, reacEquation)) r.setEquation(kbuf);

    if (object->findValue( reacMethodEOS, kbuf ))
        if (!parseIssues(kbuf, name, reacMethodEOS)) r.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    if (object->findValue( reacMethodT, kbuf ))
        if (!parseIssues(kbuf, name, reacMethodT)) r.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    if (object->findValue( reacMethodP, kbuf ))
        if (!parseIssues(kbuf, name, reacMethodP)) r.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    if (object->findValue( reacRefT, kbuf ))
        if (!parseIssues(kbuf, name, reacRefT)) r.setReferenceT(std::stod(kbuf.c_str()));

    if (object->findValue( reacRefP, kbuf ))
        if (!parseIssues(kbuf, name, reacRefP)) r.setReferenceP(std::stod(kbuf.c_str()));

    if (object->findValue( lowerT, kbuf ))
        if (!parseIssues(kbuf, name, lowerT)) r.setLowerT(std::stod(kbuf.c_str()));

    if (object->findValue( upperT, kbuf ))
        if (!parseIssues(kbuf, name, upperT)) r.setUpperT(std::stod(kbuf.c_str()));

    if (object->findValue( lowerP, kbuf ))
        if (!parseIssues(kbuf, name, lowerP)) r.setLowerP(std::stod(kbuf.c_str()));

    if (object->findValue( upperP, kbuf ))
        if (!parseIssues(kbuf, name, upperP)) r.setUpperP(std::stod(kbuf.c_str()));


    // get thermodynamic parameters
    r.setThermoParameters(thermoParamReac (object, name));
    // get reference thermodynamic properties
    r.setThermoReferenceProperties(thermoRefPropReac (object, name));

    return r;
}

auto thermoParamReac (const jsonio::JsonDom *object, std::string name) -> ThermoParametersReaction
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersReaction pr;

    if (object->findArray(  reacLogKfT, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKfT))
    {
        pr.reaction_logK_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_logK_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( reacLogKPT, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKPT))
    {
        pr.logK_TP_array.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.logK_TP_array.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( reacDrCpfT, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrCpfT))
    {
        pr.reaction_Cp_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_Cp_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( reacDrVfT, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrVfT))
    {
        pr.reaction_V_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_V_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    double lT = 273.15; double uT = 2273.15;
    if (object->findValue( lowerT, kbuf ))
        if (!parseIssues(kbuf, name, lowerT)) lT = std::stod(kbuf.c_str());
    if (object->findValue( upperT, kbuf ))
        if (!parseIssues(kbuf, name, upperT)) uT = std::stod(kbuf.c_str());
    pr.temperature_intervals.push_back({lT, uT});

    if (object->findArray(  reacRBcoeff, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacRBcoeff))
    {
        pr.reaction_RB_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_RB_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    if (object->findArray( reacFMcoeff, vkbuf))
        if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacFMcoeff))
    {
        pr.reaction_FM_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_FM_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    return pr;
}

auto thermoRefPropReac (const jsonio::JsonDom *object, string name) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    string message, idReac;

    if (object->findValue( _id, idReac ))
        if (!parseIssues(idReac, name, _id)) message = "_id : " + idReac;

    tpr.log_equilibrium_constant.sta  = readValueError(object, reacRefLogK0_, tpr.log_equilibrium_constant.val,  tpr.log_equilibrium_constant.err,  name, message);
    tpr.reaction_heat_capacity_cp.sta = readValueError(object, reacRefCp0_,   tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, name, message);
    tpr.reaction_gibbs_energy.sta     = readValueError(object, reacRefG0_,    tpr.reaction_gibbs_energy.val,     tpr.reaction_gibbs_energy.err,     name, message);
    tpr.reaction_enthalpy.sta         = readValueError(object, reacRefH0_,    tpr.reaction_enthalpy.val,         tpr.reaction_enthalpy.err,         name, message);
    tpr.reaction_entropy.sta          = readValueError(object, reacRefS0_,    tpr.reaction_entropy.val,          tpr.reaction_entropy.err,          name, message);
    tpr.reaction_volume.sta           = readValueError(object, reacRefV0_,    tpr.reaction_volume.val,           tpr.reaction_volume.err,           name, message);

    return tpr;
}

auto parseElement (const string& data) -> Element
{
  auto domdata = jsonio::unpackJson( data, "VertexElement" ); // with default values
  return parseElement(domdata.get());
}

auto parseSubstance (const string& data) -> Substance
{
  auto domdata = jsonio::unpackJson( data, "VertexSubstance" ); // with default values
  return parseSubstance(domdata.get());
}

auto parseReaction (const string& data) -> Reaction
{
 auto domdata = jsonio::unpackJson( data, "VertexReaction" ); // with default values
 return parseReaction(domdata.get());
}

}
