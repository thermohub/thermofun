#include "ReadFiles.h"
#include "Substance.h"

namespace ThermoFun {

auto parseSubstance (const char * data) -> Substance
{
    Substance s;
    string kbuf;

    bsonio::bson_to_key( data, substName, kbuf );
    s.setName(kbuf);

    bsonio::bson_to_key( data, substSymbol, kbuf );
    s.setSymbol(kbuf);

    bsonio::bson_to_key( data, substFormula, kbuf );
    s.setFormula(kbuf);

    bsonio::bson_to_key( data, substCharge, kbuf );
    s.setCharge(atoi(kbuf.c_str()));

    bsonio::bson_to_key( data, substMolarMass, kbuf );
    s.setMolarMass(atof(kbuf.c_str()));

    bsonio::bson_to_key( data, substAggState, kbuf );
    s.setAggregateState(AggregateState::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substClass, kbuf );
    s.setSubstanceClass(SubstanceClass::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substSolventNname, kbuf );
    s.setSolventSymbol(kbuf);

    bsonio::bson_to_key( data, substMethodEOS, kbuf );
    s.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substMethodT, kbuf );
    s.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substMethodP, kbuf );
    s.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( data, substRefT, kbuf );
    s.setReferenceT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefP, kbuf );
    s.setReferenceP(std::stod(kbuf.c_str()));

    // get thermodynamic parameters
    s.setThermoParameters(thermoParam (data));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefProp (data));

    return s;
}

auto thermoParam (const char * data) -> ThermoParametersSubstance
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersSubstance ps;

    bsonio::bson_to_key(data, substExpans, kbuf);
    if (kbuf != "*") ps.isobaric_expansivity = std::stod(kbuf.c_str());
    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_to_key(data, substCompres, kbuf);
    if (kbuf != "*") ps.isothermal_compresibility = std::stod(kbuf.c_str());
    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_read_array_path(data, substEOSad, vkbuf); ps.Cp_nonElectrolyte_coeff.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_nonElectrolyte_coeff.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substEOSbm, vkbuf); ps.volume_BirchM_coeff.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.volume_BirchM_coeff.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substEOScg, vkbuf);/* ps.critical_parameters.resize(vkbuf.size());*/
//    std::transform(kbuf.begin(), kbuf.end(), tps..begin(), [](const std::string& val)
//    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substEOSgasCrit, vkbuf); ps.critical_parameters.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.critical_parameters.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substEOShkf, vkbuf); ps.HKF_parameters.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.HKF_parameters.begin(), [](const std::string& val)
    { return std::stod(val); });

    // temporary fix - need to think how to handle more thna 1 TP interval
    ps.temperature_intervals.push_back({273.15, 2273.15});

    bsonio::bson_read_array_path(data, substCpParam, vkbuf);
    ps.Cp_coeff.resize(1); ps.Cp_coeff[0].resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_coeff[0].begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(data, substTransProp, vkbuf); ps.phase_transition_prop.resize(1); ps.phase_transition_prop[0].resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop[0].begin(), [](const std::string& val)
    { return std::stod(val); });

//    bsonio::bson_read_array_path(data, substTransProp, vkbuf); ps.phase_transition_prop_Berman.resize(1); ps.phase_transition_prop_Berman[0].resize(vkbuf.size());
//    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop_Berman[0].begin(), [](const std::string& val)
//    { return std::stod(val); });

    return ps;
}

auto thermoRefProp (const char *data) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string kbuf;

    bsonio::bson_to_key( data, substRefCp0, kbuf );
    tps.heat_capacity_cp = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefG0, kbuf );
    tps.gibbs_energy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefH0, kbuf );
    tps.enthalpy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefS0, kbuf );
    tps.entropy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( data, substRefV0, kbuf );
    tps.volume = (std::stod(kbuf.c_str()));

    return tps;
}

}
