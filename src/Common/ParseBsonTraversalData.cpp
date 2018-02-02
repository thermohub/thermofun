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
#include "Common/Exception.h"

namespace ThermoFun {

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

    object->findKey( propPath+".values.0", sval );
    object->findKey(  propPath+".errors.0", serr );
    if (!parseIssues(sval, name, propPath+".values.0")) {
        val = (std::stod(sval.c_str()));
        status = {Reaktoro_::Status::read, message};;
    }
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
            string jsonrecord = dbc.substData().getJsonRecordVertex(_idSubst+":");
            auto domdata = jsonio::unpackJson( jsonrecord, "VertexSubstance" ); // with default values
            domdata->findKey("properties.symbol", substSymb);
            // record = dbc.substData().getJsonBsonRecordVertex(_idSubst+":").second;
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
                string jsonrecord = dbc.reactData().getJsonRecordVertex(_idReact+":");
                auto domdata = jsonio::unpackJson( jsonrecord, "VertexReaction" ); // with default values
                // record = dbc.reactData().getJsonBsonRecordVertex(_idReact+":").second;

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

    object->findKey( elemName, kbuf );
    if (!parseIssues(kbuf, name, elemName)) { e.setName(kbuf); name = kbuf;}

    object->findKey( elemSymbol, kbuf );
    if (!parseIssues(kbuf, name, elemSymbol)) { e.setSymbol(kbuf);}

    string entropy_ = elemEntropy;
    entropy_ += ".values.0";
    object->findKey( entropy_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, entropy_.c_str())) { e.setEntropy(atof(kbuf.c_str()));}

    string cp_ = elemHeatCapacity;
    cp_ += ".values.0";
    object->findKey( cp_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, cp_.c_str())) { e.setHeatCapacity(atof(kbuf.c_str()));}

    string mass_ = elemMolarMass;
    mass_ += ".values.0";
    object->findKey( mass_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, mass_.c_str())) { e.setMolarMass(atof(kbuf.c_str()));}

    object->findKey( elemClass, kbuf );
    if (!parseIssues(kbuf, name, elemClass)) { e.setClass(atoi(kbuf.c_str()));}

    object->findKey( elemIsotopeMass, kbuf );
    if (!parseIssues(kbuf, name, elemIsotopeMass)) { e.setIsotopeMass(atof(kbuf.c_str()));}

    return e;
}

auto parseSubstance (const jsonio::JsonDom *object) -> Substance
{
    Substance s;
    string kbuf;
    string name;

    object->findKey( substName, kbuf );
    if (!parseIssues(kbuf, name, substName)) { s.setName(kbuf); name = kbuf;}

    object->findKey( substSymbol, kbuf );
    if (!parseIssues(kbuf, name, substSymbol)) s.setSymbol(kbuf);

    object->findKey( substFormula, kbuf );
    if (!parseIssues(kbuf, name, substFormula )) s.setFormula(kbuf);

    object->findKey( substCharge, kbuf );
    if (!parseIssues(kbuf, name, substCharge)) s.setCharge(atoi(kbuf.c_str()));

    object->findKey( substMolarMass, kbuf );
    if (!parseIssues(kbuf, name, substMolarMass)) s.setMolarMass(atof(kbuf.c_str()));

    object->findKey( substAggState, kbuf );
    if (!parseIssues(kbuf, name, substAggState)) s.setAggregateState(AggregateState::type(std::stoi(kbuf.c_str())));

    object->findKey( substClass, kbuf );
    if (!parseIssues(kbuf, name, substClass)) s.setSubstanceClass(SubstanceClass::type(std::stoi(kbuf.c_str())));

    object->findKey( lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) s.setLowerT(std::stod(kbuf.c_str()));

    object->findKey( upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) s.setUpperT(std::stod(kbuf.c_str()));

    object->findKey( lowerP, kbuf );
    if (!parseIssues(kbuf, name, lowerP)) s.setLowerP(std::stod(kbuf.c_str()));

    object->findKey( upperP, kbuf );
    if (!parseIssues(kbuf, name, upperP)) s.setUpperP(std::stod(kbuf.c_str()));

//    bsonio::bson_to_key( data, substSolventNname, kbuf );
//    if (!parseIssues(kbuf, name, substSolventNname)) s.setSolventSymbol(kbuf);

    object->findKey( substMethodEOS, kbuf );
    if (!parseIssues(kbuf, name, substMethodEOS)) s.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( substMethodT, kbuf );
    if (!parseIssues(kbuf, name, substMethodT)) s.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( substMethodP, kbuf );
    if (!parseIssues(kbuf, name, substMethodP)) s.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( substRefT, kbuf );
    if (!parseIssues(kbuf, name, substRefT)) s.setReferenceT(std::stod(kbuf.c_str()));

    object->findKey( substRefP, kbuf );
    if (!parseIssues(kbuf, name, substRefP)) s.setReferenceP(std::stod(kbuf.c_str()));

    // get thermodynamic parameters
    s.setThermoParameters(thermoParamSubst (object, name));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefPropSubst (object, name));

    return s;
}

auto thermoParamSubst (const jsonio::JsonDom *object, std::string name) -> ThermoParametersSubstance
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersSubstance ps;

    string expans = substExpans_ ; expans += ".values.0";
    string compres = substCompres_ ; compres += ".values.0";

    object->findKey( expans, kbuf);
    if (!parseIssues(kbuf, name, expans))  ps.isobaric_expansivity = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    object->findKey( compres, kbuf);
    if (!parseIssues(kbuf, name, compres))  ps.isothermal_compresibility = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    object->findArray(  substEOSad, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSad))
    {
        ps.Cp_nonElectrolyte_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_nonElectrolyte_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray(  substEOSbm, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSbm))
    {
        ps.volume_BirchM_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.volume_BirchM_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray(  substEOScg, vkbuf);/* ps.critical_parameters.resize(vkbuf.size());*/
//    std::transform(kbuf.begin(), kbuf.end(), tps..begin(), [](const std::string& val)
//    { return std::stod(val); });

    object->findArray(  substEOSgasCrit, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSgasCrit))
    {
        ps.critical_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.critical_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( substEOShkf, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOShkf))
    {
        ps.HKF_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.HKF_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    // temporary fix - need to think how to handle more thna 1 TP interval
    ps.temperature_intervals.push_back({273.15, 2273.15});

    object->findArray( substCpParam, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substCpParam))
    {
        ps.Cp_coeff.resize(1); ps.Cp_coeff[0].resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_coeff[0].begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( substTransProp, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substTransProp))
    {
        ps.phase_transition_prop.resize(1); ps.phase_transition_prop[0].resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop[0].begin(), [](const std::string& val)
        { return std::stod(val); });
    }

//    bsonio::bson_read_array_path(data, substTransProp, vkbuf); ps.phase_transition_prop_Berman.resize(1); ps.phase_transition_prop_Berman[0].resize(vkbuf.size());
//    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop_Berman[0].begin(), [](const std::string& val)
//    { return std::stod(val); });

    return ps;
}

auto thermoRefPropSubst (const jsonio::JsonDom *object, string name) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string idSubst, message;

    object->findKey( _id, idSubst );
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

    object->findKey( reacName, kbuf );
    if (!parseIssues(kbuf, name, reacName)) { r.setName(kbuf); name = kbuf;}

    object->findKey( reacSymbol, kbuf );
    if (!parseIssues(kbuf, name, reacSymbol)) r.setSymbol(kbuf);

    object->findKey( reacEquation, kbuf );
    if (!parseIssues(kbuf, name, reacEquation)) r.setEquation(kbuf);

    object->findKey( reacMethodEOS, kbuf );
    if (!parseIssues(kbuf, name, reacMethodEOS)) r.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( reacMethodT, kbuf );
    if (!parseIssues(kbuf, name, reacMethodT)) r.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( reacMethodP, kbuf );
    if (!parseIssues(kbuf, name, reacMethodP)) r.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    object->findKey( reacRefT, kbuf );
    if (!parseIssues(kbuf, name, reacRefT)) r.setReferenceT(std::stod(kbuf.c_str()));

    object->findKey( reacRefP, kbuf );
    if (!parseIssues(kbuf, name, reacRefP)) r.setReferenceP(std::stod(kbuf.c_str()));

    object->findKey( lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) r.setLowerT(std::stod(kbuf.c_str()));

    object->findKey( upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) r.setUpperT(std::stod(kbuf.c_str()));

    object->findKey( lowerP, kbuf );
    if (!parseIssues(kbuf, name, lowerP)) r.setLowerP(std::stod(kbuf.c_str()));

    object->findKey( upperP, kbuf );
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

    object->findArray(  reacLogKfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKfT))
    {
        pr.reaction_logK_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_logK_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( reacLogKPT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKPT))
    {
        pr.logK_TP_array.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.logK_TP_array.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( reacDrCpfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrCpfT))
    {
        pr.reaction_Cp_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_Cp_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( reacDrVfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrVfT))
    {
        pr.reaction_V_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_V_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    double lT = 273.15; double uT = 2273.15;
    object->findKey( lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) lT = std::stod(kbuf.c_str());
    object->findKey( upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) uT = std::stod(kbuf.c_str());
    pr.temperature_intervals.push_back({lT, uT});

    object->findArray(  reacRBcoeff, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacRBcoeff))
    {
        pr.reaction_RB_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_RB_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    object->findArray( reacFMcoeff, vkbuf);
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

    object->findKey( _id, idReac );
    if (!parseIssues(idReac, name, _id)) message = "_id : " + idReac;

    tpr.log_equilibrium_constant.sta  = readValueError(object, reacRefLogK0_, tpr.log_equilibrium_constant.val,  tpr.log_equilibrium_constant.err,  name, message);
    tpr.reaction_heat_capacity_cp.sta = readValueError(object, reacRefCp0_,   tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, name, message);
    tpr.reaction_gibbs_energy.sta     = readValueError(object, reacRefG0_,    tpr.reaction_gibbs_energy.val,     tpr.reaction_gibbs_energy.err,     name, message);
    tpr.reaction_enthalpy.sta         = readValueError(object, reacRefH0_,    tpr.reaction_enthalpy.val,         tpr.reaction_enthalpy.err,         name, message);
    tpr.reaction_entropy.sta          = readValueError(object, reacRefS0_,    tpr.reaction_entropy.val,          tpr.reaction_entropy.err,          name, message);
    tpr.reaction_volume.sta           = readValueError(object, reacRefV0_,    tpr.reaction_volume.val,           tpr.reaction_volume.err,           name, message);

    return tpr;
}

}
