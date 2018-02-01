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
#include "jsonio/thrift_node.h"
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

auto readValueError(const char * data, string propPath, double &val, double &err, string name, string message) -> Reaktoro_::StatusMessage
{
    string sval, serr;
    Reaktoro_::StatusMessage status = {Reaktoro_::Status::notdefined, message};

    bsonio::bson_to_key( data, propPath+".values.0", sval );
    bsonio::bson_to_key( data, propPath+".errors.0", serr );
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
    string _idSubst, _idReact, substSymb; string level_;
    bson record;
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
            record = dbc.substData().getJsonBsonRecordVertex(_idSubst+":").second;
            bsonio::bson_to_key( record.data, "properties.symbol", substSymb );

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

            Substance substance = parseSubstance(record.data);

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
                record = dbc.reactData().getJsonBsonRecordVertex(_idReact+":").second;

                Reaction reaction = ThermoFun::parseReaction(record.data);

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

auto parseElement (const char *data) -> Element
{
    Element e;
    string kbuf;
    string name;

    bsonio::bson_to_key( data, elemName, kbuf );
    if (!parseIssues(kbuf, name, elemName)) { e.setName(kbuf); name = kbuf;}

    bsonio::bson_to_key( data, elemSymbol, kbuf );
    if (!parseIssues(kbuf, name, elemSymbol)) { e.setSymbol(kbuf);}

    string entropy_ = elemEntropy;
    entropy_ += ".values.0";
    bsonio::bson_to_key( data, entropy_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, entropy_.c_str())) { e.setEntropy(atof(kbuf.c_str()));}

    string cp_ = elemHeatCapacity;
    cp_ += ".values.0";
    bsonio::bson_to_key( data, cp_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, cp_.c_str())) { e.setHeatCapacity(atof(kbuf.c_str()));}

    string mass_ = elemMolarMass;
    mass_ += ".values.0";
    bsonio::bson_to_key( data, mass_.c_str(), kbuf );
    if (!parseIssues(kbuf, name, mass_.c_str())) { e.setMolarMass(atof(kbuf.c_str()));}

    bsonio::bson_to_key( data, elemClass, kbuf );
    if (!parseIssues(kbuf, name, elemClass)) { e.setClass(atoi(kbuf.c_str()));}

    bsonio::bson_to_key( data, elemIsotopeMass, kbuf );
    if (!parseIssues(kbuf, name, elemIsotopeMass)) { e.setIsotopeMass(atof(kbuf.c_str()));}

    return e;
}

auto parseSubstance (const char * data) -> Substance
{
    Substance s;
    string kbuf;
    string name;

    bsonio::bson_to_key( data, substName, kbuf );
    if (!parseIssues(kbuf, name, substName)) { s.setName(kbuf); name = kbuf;}

    bsonio::bson_to_key( data, substSymbol, kbuf );
    if (!parseIssues(kbuf, name, substSymbol)) s.setSymbol(kbuf);

    bsonio::bson_to_key( data, substFormula, kbuf );
    if (!parseIssues(kbuf, name, substFormula )) s.setFormula(kbuf);

    bsonio::bson_to_key( data, substCharge, kbuf );
    if (!parseIssues(kbuf, name, substCharge)) s.setCharge(atoi(kbuf.c_str()));

    bsonio::bson_to_key( data, substMolarMass, kbuf );
    if (!parseIssues(kbuf, name, substMolarMass)) s.setMolarMass(atof(kbuf.c_str()));

    bsonio::bson_to_key( data, substAggState, kbuf );
    if (!parseIssues(kbuf, name, substAggState)) s.setAggregateState(AggregateState::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substClass, kbuf );
    if (!parseIssues(kbuf, name, substClass)) s.setSubstanceClass(SubstanceClass::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) s.setLowerT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) s.setUpperT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, lowerP, kbuf );
    if (!parseIssues(kbuf, name, lowerP)) s.setLowerP(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, upperP, kbuf );
    if (!parseIssues(kbuf, name, upperP)) s.setUpperP(std::stod(kbuf.c_str()));

//    bsonio::bson_to_key( data, substSolventNname, kbuf );
//    if (!parseIssues(kbuf, name, substSolventNname)) s.setSolventSymbol(kbuf);

    bsonio::bson_to_key( data, substMethodEOS, kbuf );
    if (!parseIssues(kbuf, name, substMethodEOS)) s.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substMethodT, kbuf );
    if (!parseIssues(kbuf, name, substMethodT)) s.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substMethodP, kbuf );
    if (!parseIssues(kbuf, name, substMethodP)) s.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substRefT, kbuf );
    if (!parseIssues(kbuf, name, substRefT)) s.setReferenceT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefP, kbuf );
    if (!parseIssues(kbuf, name, substRefP)) s.setReferenceP(std::stod(kbuf.c_str()));

    // get thermodynamic parameters
    s.setThermoParameters(thermoParamSubst (data, name));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefPropSubst (data, name));

    return s;
}

auto thermoParamSubst (const char * data, std::string name) -> ThermoParametersSubstance
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersSubstance ps;

    string expans = substExpans_ ; expans += ".values.0";
    string compres = substCompres_ ; compres += ".values.0";

    bsonio::bson_to_key(data, expans, kbuf);
    if (!parseIssues(kbuf, name, expans))  ps.isobaric_expansivity = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_to_key(data, compres, kbuf);
    if (!parseIssues(kbuf, name, compres))  ps.isothermal_compresibility = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_read_array_path(data, substEOSad, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSad))
    {
        ps.Cp_nonElectrolyte_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_nonElectrolyte_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, substEOSbm, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSbm))
    {
        ps.volume_BirchM_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.volume_BirchM_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, substEOScg, vkbuf);/* ps.critical_parameters.resize(vkbuf.size());*/
//    std::transform(kbuf.begin(), kbuf.end(), tps..begin(), [](const std::string& val)
//    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substEOSgasCrit, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOSgasCrit))
    {
        ps.critical_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.critical_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, substEOShkf, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substEOShkf))
    {
        ps.HKF_parameters.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.HKF_parameters.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    // temporary fix - need to think how to handle more thna 1 TP interval
    ps.temperature_intervals.push_back({273.15, 2273.15});

    bsonio::bson_read_array_path(data, substCpParam, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, substCpParam))
    {
        ps.Cp_coeff.resize(1); ps.Cp_coeff[0].resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_coeff[0].begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, substTransProp, vkbuf);
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

auto thermoRefPropSubst (const char *data, string name) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string idSubst, message;

    bsonio::bson_to_key( data, _id, idSubst );
    if (!parseIssues(idSubst, name, _id)) message = "_id : " + idSubst;

    tps.heat_capacity_cp.sta = readValueError(data, substRefCp0_, tps.heat_capacity_cp.val, tps.heat_capacity_cp.err, name, message);
    tps.gibbs_energy.sta     = readValueError(data, substRefG0_,  tps.gibbs_energy.val,     tps.gibbs_energy.err,     name, message);
    tps.enthalpy.sta         = readValueError(data, substRefH0_,  tps.enthalpy.val,         tps.enthalpy.err,         name, message);
    tps.entropy.sta          = readValueError(data, substRefS0_,  tps.entropy.val,          tps.entropy.err,          name, message);
    tps.volume.sta           = readValueError(data, substRefV0_,  tps.volume.val,           tps.volume.err,           name, message);

    return tps;
}


auto parseReaction (const char *data) -> Reaction
{
    Reaction r;
    string kbuf;
    string name;

    bsonio::bson_to_key( data, reacName, kbuf );
    if (!parseIssues(kbuf, name, reacName)) { r.setName(kbuf); name = kbuf;}

    bsonio::bson_to_key( data, reacSymbol, kbuf );
    if (!parseIssues(kbuf, name, reacSymbol)) r.setSymbol(kbuf);

    bsonio::bson_to_key( data, reacEquation, kbuf );
    if (!parseIssues(kbuf, name, reacEquation)) r.setEquation(kbuf);

    bsonio::bson_to_key( data, reacMethodEOS, kbuf );
    if (!parseIssues(kbuf, name, reacMethodEOS)) r.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, reacMethodT, kbuf );
    if (!parseIssues(kbuf, name, reacMethodT)) r.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, reacMethodP, kbuf );
    if (!parseIssues(kbuf, name, reacMethodP)) r.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, reacRefT, kbuf );
    if (!parseIssues(kbuf, name, reacRefT)) r.setReferenceT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefP, kbuf );
    if (!parseIssues(kbuf, name, reacRefP)) r.setReferenceP(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) r.setLowerT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) r.setUpperT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, lowerP, kbuf );
    if (!parseIssues(kbuf, name, lowerP)) r.setLowerP(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, upperP, kbuf );
    if (!parseIssues(kbuf, name, upperP)) r.setUpperP(std::stod(kbuf.c_str()));


    // get thermodynamic parameters
    r.setThermoParameters(thermoParamReac (data, name));
    // get reference thermodynamic properties
    r.setThermoReferenceProperties(thermoRefPropReac (data, name));

    return r;
}

auto thermoParamReac (const char * data, std::string name) -> ThermoParametersReaction
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersReaction pr;

    bsonio::bson_read_array_path(data, reacLogKfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKfT))
    {
        pr.reaction_logK_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_logK_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, reacLogKPT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacLogKPT))
    {
        pr.logK_TP_array.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.logK_TP_array.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, reacDrCpfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrCpfT))
    {
        pr.reaction_Cp_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_Cp_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, reacDrVfT, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacDrVfT))
    {
        pr.reaction_V_fT_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_V_fT_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    double lT = 273.15; double uT = 2273.15;
    bsonio::bson_to_key( data, lowerT, kbuf );
    if (!parseIssues(kbuf, name, lowerT)) lT = std::stod(kbuf.c_str());
    bsonio::bson_to_key( data, upperT, kbuf );
    if (!parseIssues(kbuf, name, upperT)) uT = std::stod(kbuf.c_str());
    pr.temperature_intervals.push_back({lT, uT});

    bsonio::bson_read_array_path(data, reacRBcoeff, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacRBcoeff))
    {
        pr.reaction_RB_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_RB_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    bsonio::bson_read_array_path(data, reacFMcoeff, vkbuf);
    if (vkbuf.size() > 0) if (!parseIssues(vkbuf[0], name, reacFMcoeff))
    {
        pr.reaction_FM_coeff.resize(vkbuf.size());
        std::transform(vkbuf.begin(), vkbuf.end(), pr.reaction_FM_coeff.begin(), [](const std::string& val)
        { return std::stod(val); });
    }

    return pr;
}

auto thermoRefPropReac (const char *data, string name) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    string message, idReac;

    bsonio::bson_to_key( data, _id, idReac );
    if (!parseIssues(idReac, name, _id)) message = "_id : " + idReac;

    tpr.log_equilibrium_constant.sta  = readValueError(data, reacRefLogK0_, tpr.log_equilibrium_constant.val,  tpr.log_equilibrium_constant.err,  name, message);
    tpr.reaction_heat_capacity_cp.sta = readValueError(data, reacRefCp0_,   tpr.reaction_heat_capacity_cp.val, tpr.reaction_heat_capacity_cp.err, name, message);
    tpr.reaction_gibbs_energy.sta     = readValueError(data, reacRefG0_,    tpr.reaction_gibbs_energy.val,     tpr.reaction_gibbs_energy.err,     name, message);
    tpr.reaction_enthalpy.sta         = readValueError(data, reacRefH0_,    tpr.reaction_enthalpy.val,         tpr.reaction_enthalpy.err,         name, message);
    tpr.reaction_entropy.sta          = readValueError(data, reacRefS0_,    tpr.reaction_entropy.val,          tpr.reaction_entropy.err,          name, message);
    tpr.reaction_volume.sta           = readValueError(data, reacRefV0_,    tpr.reaction_volume.val,           tpr.reaction_volume.err,           name, message);

    return tpr;
}

}
