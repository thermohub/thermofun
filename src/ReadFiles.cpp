#include "ReadFiles.h"
#include "Substance.h"
#include "Reaction.h"

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

    bsonio::bson_to_key(data, substExpans, kbuf);
    if (!parseIssues(kbuf, name, substExpans))  ps.isobaric_expansivity = std::stod(kbuf.c_str());
//    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_to_key(data, substCompres, kbuf);
    if (!parseIssues(kbuf, name, substCompres))  ps.isothermal_compresibility = std::stod(kbuf.c_str());
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
    string kbuf;

    bsonio::bson_to_key( data, substRefCp0, kbuf );
    if (!parseIssues(kbuf, name, substRefCp0)) tps.heat_capacity_cp = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefG0, kbuf );
    if (!parseIssues(kbuf, name, substRefG0)) tps.gibbs_energy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefH0, kbuf );
    if (!parseIssues(kbuf, name, substRefH0)) tps.enthalpy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefS0, kbuf );
    if (!parseIssues(kbuf, name, substRefS0)) tps.entropy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefV0, kbuf );
    if (!parseIssues(kbuf, name, substRefV0)) tps.volume = (std::stod(kbuf.c_str()));

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
    string kbuf;

    bsonio::bson_to_key( data, reacRefLogK0, kbuf );
    if (!parseIssues(kbuf, name, reacRefLogK0)) tpr.log_equilibrium_constant = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefCp0, kbuf );
    if (!parseIssues(kbuf, name, reacRefCp0)) tpr.reaction_heat_capacity_cp = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefG0, kbuf );
    if (!parseIssues(kbuf, name, reacRefG0)) tpr.reaction_gibbs_energy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefH0, kbuf );
    if (!parseIssues(kbuf, name, reacRefH0)) tpr.reaction_enthalpy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefS0, kbuf );
    if (!parseIssues(kbuf, name, reacRefS0)) tpr.reaction_entropy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, reacRefV0, kbuf );
    if (!parseIssues(kbuf, name, reacRefV0)) tpr.reaction_volume = (std::stod(kbuf.c_str()));

    return tpr;
}

}
