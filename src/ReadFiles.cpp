#include "ReadFiles.h"
#include "Substance.h"

namespace ThermoFun {

auto parseIssues(std::string data, string name, string prop) -> bool
{
    if (data == "*")
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
    s.setThermoParameters(thermoParam (data, name));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefProp (data, name));

    return s;
}

auto thermoParam (const char * data, std::string name) -> ThermoParametersSubstance
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

auto thermoRefProp (const char *data, string name) -> ThermoPropertiesSubstance
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

}
