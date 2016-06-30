#include "ReadFiles.h"
#include "Substance.h"

namespace TCorrPT {

auto parseSubstance (bson bso) -> Substance
{
    Substance s;
    string kbuf;

    bsonio::bson_to_key( bso.data, substName, kbuf );
    s.setName(kbuf);

    bsonio::bson_to_key( bso.data, substSymbol, kbuf );
    s.setSymbol(kbuf);

    bsonio::bson_to_key( bso.data, substFormula, kbuf );
    s.setFormula(kbuf);

    bsonio::bson_to_key( bso.data, substCharge, kbuf );
    s.setCharge(atoi(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substMolarMass, kbuf );
    s.setMolarMass(atof(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substAggState, kbuf );
    s.setAggregateState(AggregateState::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( bso.data, substClass, kbuf );
    s.setSubstanceClass(SubstanceClass::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( bso.data, substSolventNname, kbuf );
    s.setSolventSymbol(kbuf);

    bsonio::bson_to_key( bso.data, substMethodEOS, kbuf );
    s.setMethodGenEoS(MethodGenEoS_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( bso.data, substMethodT, kbuf );
    s.setMethod_T(MethodCorrT_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( bso.data, substMethodP, kbuf );
    s.setMethod_P(MethodCorrP_Thrift::type(std::stoi(kbuf.c_str())));

    bsonio::bson_to_key( bso.data, substRefT, kbuf );
    s.setReferenceT(std::stod(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substRefP, kbuf );
    s.setReferenceP(std::stod(kbuf.c_str()));

    // get thermodynamic parameters
    s.setThermoParameters(thermoParam (bso));
    // get reference thermodynamic properties
    s.setThermoReferenceProperties(thermoRefProp (bso));

    return s;
}

auto thermoParam (bson bso) -> ThermoParametersSubstance
{
    vector<string> vkbuf;
    string kbuf;
    ThermoParametersSubstance ps;

    bsonio::bson_to_key(bso.data, substExpans, kbuf);
    if (kbuf != "*") ps.isobaric_expansivity = std::stod(kbuf.c_str());
    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_to_key(bso.data, substCompres, kbuf);
    if (kbuf != "*") ps.isothermal_compresibility = std::stod(kbuf.c_str());
    else ps.isobaric_expansivity = 0.0;

    bsonio::bson_read_array_path(bso.data, substEOSad, vkbuf); ps.Cp_nonElectrolyte_coeff.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_nonElectrolyte_coeff.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(bso.data, substEOSbm, vkbuf); ps.volume_BirchM_coeff.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.volume_BirchM_coeff.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(bso.data, substEOScg, vkbuf);/* ps.critical_parameters.resize(vkbuf.size());*/
//    std::transform(kbuf.begin(), kbuf.end(), tps..begin(), [](const std::string& val)
//    { return std::stod(val); });

    bsonio::bson_read_array_path(bso.data, substEOSgasCrit, vkbuf); ps.critical_parameters.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.critical_parameters.begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(bso.data, substEOShkf, vkbuf); ps.HKF_parameters.resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.HKF_parameters.begin(), [](const std::string& val)
    { return std::stod(val); });

    // temporary fix - need to think how to handle more thna 1 TP interval
    ps.temperature_intervals.push_back({273.15, 2273.15});

    bsonio::bson_read_array_path(bso.data, substCpParam, vkbuf);
    ps.Cp_coeff.resize(1); ps.Cp_coeff[0].resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.Cp_coeff[0].begin(), [](const std::string& val)
    { return std::stod(val); });

    bsonio::bson_read_array_path(bso.data, substTransProp, vkbuf); ps.phase_transition_prop.resize(1); ps.phase_transition_prop[0].resize(vkbuf.size());
    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop[0].begin(), [](const std::string& val)
    { return std::stod(val); });

//    bsonio::bson_read_array_path(bso.data, substTransProp, vkbuf); ps.phase_transition_prop_Berman.resize(1); ps.phase_transition_prop_Berman[0].resize(vkbuf.size());
//    std::transform(vkbuf.begin(), vkbuf.end(), ps.phase_transition_prop_Berman[0].begin(), [](const std::string& val)
//    { return std::stod(val); });

    return ps;
}

auto thermoRefProp (bson bso) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    string kbuf;

    bsonio::bson_to_key( bso.data, substRefCp0, kbuf );
    tps.heat_capacity_cp = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substRefG0, kbuf );
    tps.gibbs_energy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substRefH0, kbuf );
    tps.enthalpy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substRefS0, kbuf );
    tps.entropy = (std::stod(kbuf.c_str()));

    bsonio::bson_to_key( bso.data, substRefV0, kbuf );
    tps.volume = (std::stod(kbuf.c_str()));

    return tps;
}

}
