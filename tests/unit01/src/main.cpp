#include <iostream>
#include <fstream>
#include "ThermoFun/Database.h"
#include "ThermoFun/Substance.h"
#include "ThermoFun/ThermoEngine.h"
#include "ThermoFun/ThermoModelsSubstance.h"
#include "ThermoFun/ThermoProperties.h"
#include "ThermoFun/ThermoParameters.h"
#include "ThermoFun/ThermoModelsSolvent.h"
#include "ThermoFun/GlobalVariables.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    /// thermo properties substance ///
    double sV0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
           sG0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
           sH0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
           sS0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
           saS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
           sCp0;             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR

    double G0, H0, S0,         ///< partial molal gibbs energy, enthalpy, entropy at current TP; [1] ERROR
           V0, Cp0,               ///< partial molal volume, heat capacity at current TP; [1] ERROR
           Fug, dVg;              ///< molar fugacity at TP (reserved), dVm of gas (reserved); [1] ERROR

    double Tr = 298.15,                  ///< Reference temperature for standard state (usually 298.15 K)
           Pr = 1e05;                  ///< Reference pressure (usually 1 bar or 10^5 Pa, sometimes 1.013 bar = 1 atm)
    double currentP = 2000e5,                    ///<
           currentT = 473.15;                    ///<

   /// END thermo properties ///

    cout << "Hello World!" << endl;

    Database dtb;

// +++ Test EmpCpIntegr +++
//#define TEST_EMPCPINTEGR
#ifdef TEST_EMPCPINTEGR

    vd Interval, Coeff;
    vvd TCpInterval,
    CpCoefficients;

    // Corundum
    // Reference properties
    V0 = 2.558;
    G0 = -1581808;
    H0 = -1675250;
    S0 = 50.9;
    Cp0 = 79.4529;

    Interval = {0,2000};
    Coeff = {139.5, 0.00589, -2.4606e6, -589.2};
    TCpInterval.push_back(Interval);
    CpCoefficients.push_back(Coeff);

    Substance corundum;
    corundum.setName("Corundum");
    corundum.setFormula("Al2O3");
    corundum.setSubstanceClass(SubstanceClass::type::COMPONENT);
    corundum.setAggregateState(AggregateState::type::CRYSTAL);

    ThermoPropertiesSubstance corr_data_atPrTr;
    ThermoPropertiesSubstance result1, result2;

    corr_data_atPrTr.volume = V0;
    corr_data_atPrTr.gibbs_energy =G0;
    corr_data_atPrTr.enthalpy = H0;
    corr_data_atPrTr.entropy = S0;
    corr_data_atPrTr.heat_capacity_cp = Cp0;

    corundum.setThermoReferenceProperties(corr_data_atPrTr);
    corundum.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_CPT);

    ThermoParametersSubstance param = corundum.thermoParameters();

    param.temperature_intervals = TCpInterval;
    param.Cp_coeff = CpCoefficients;

    corundum.setThermoParameters(param);

    ThermoModelsSubstance thermo_corundum (corundum);
    result1 = thermo_corundum.thermoProperties(currentT, currentP);

    // TCorrPT API

    dtb.addSubstance(corundum);

    ThermoEngine thermo (dtb);

    // Test error out of the interval
//    currentT = 2005;
    result2 = thermo.thermoPropertiesSubstance(currentT, currentP, "Corundum");

    // Test error no substance
//    result2 = thermo.thermoPropertiesSubstance(currentT, currentP, "alala");

#endif
// +++ END test CP +++

    // +++ Test H2O_HGK +++
    //#define TEST_H2O_HGK
    #ifdef TEST_H2O_HGK

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    ThermoPropertiesSubstance h2o_data_atPrTr;
    ThermoPropertiesSubstance result2;
    PropertiesSolvent result1, result3;

    h2o_data_atPrTr.volume = 0;
    h2o_data_atPrTr.gibbs_energy = 0;
    h2o_data_atPrTr.enthalpy = 0;
    h2o_data_atPrTr.entropy = 0;
    h2o_data_atPrTr.heat_capacity_cp = 0;

    water.setThermoReferenceProperties(h2o_data_atPrTr);
    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);

//    ThermoModelsSolvent thermo_water (water);
//    result1 = thermo_water.thermoProperties(25, 1);

    dtb.addSubstance(water);

    ThermoEngine thermo (dtb);
    currentP = 1e5;
    result2 = thermo.thermoPropertiesSubstance(298.15, currentP, "water");

    water.setThermoReferenceProperties(result2);

    WaterHGK waterHGK(water);

    result1 = waterHGK.propertiesSolvent(298.15, currentP, 0);

    result3 = thermo.propertiesSolvent(298.15, currentP, 0);

#endif
// +++ END test H2O_HGK +++

    // +++ Test H2O_HGKreaktoro +++
    //#define TEST_H2O_HGKreaktoro
    #ifdef TEST_H2O_HGKreaktoro

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    ThermoPropertiesSubstance h2o_data_atPrTr;
    ThermoPropertiesSubstance result2;
//    PropertiesSolvent result1, result3;

    h2o_data_atPrTr.volume = 0;
    h2o_data_atPrTr.gibbs_energy = 0;
    h2o_data_atPrTr.enthalpy = 0;
    h2o_data_atPrTr.entropy = 0;
    h2o_data_atPrTr.heat_capacity_cp = 0;

    water.setThermoReferenceProperties(h2o_data_atPrTr);
    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);

//    ThermoModelsSolvent thermo_water (water);
//    result1 = thermo_water.thermoProperties(25, 1);

    dtb.addSubstance(water);

    ThermoEngine thermo (dtb);
    currentP = 1e5;
    result2 = thermo.thermoPropertiesSubstance(298.15, currentP, "water");

    water.setThermoReferenceProperties(result2);

#endif
// +++ END test H2O_HGKreaktoro +++

    // +++ Test H2O_HGKgems Vs H2O_HGKreaktoro +++
    //#define TEST_H2O_VS
    #ifdef TEST_H2O_VS

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);
    WaterHGK H2OHGKgems ( water );

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);
    WaterHGKreaktoro H2OHGKreaktoro ( water );

    double T, P;
    T = 100+273.15;
    P = 5000e5;
    ThermoPropertiesSubstance resSubstG, resSubstR;

    ElectroPropertiesSolvent resSolvG, resSolvR;

    ofstream myfile;
    myfile.open ("H2Oprop_HGKg_vs_HGKr.csv");

    myfile <<"T,P,G0g,G0r,H0g,H0r,S0g,S0r,A0g,A0r,U0g,U0r,V0g,V0r,Cp0g,Cp0r,Cv0g,Cv0r\n";

    do
    {
        myfile << T << ","<<P<<",";
        resSubstG = H2OHGKgems.thermoPropertiesSubstance(T, P, 1);
        resSubstR = H2OHGKreaktoro.thermoPropertiesSubstance(T, P, 1);

//        resSolvG = H2OHGKgems.electroPropertiesSolvent(T,P);
//        resSolvR = H2OHGKreaktoro.electroPropertiesSolvent(T,P);

        myfile << resSubstG.gibbs_energy <<","<<resSubstR.gibbs_energy<<",";
        myfile << resSubstG.enthalpy <<","<<resSubstR.enthalpy<<",";
        myfile << resSubstG.entropy <<","<<resSubstR.entropy<<",";
        myfile << resSubstG.helmholtz_energy <<","<<resSubstR.helmholtz_energy<<",";
        myfile << resSubstG.internal_energy <<","<<resSubstR.internal_energy<<",";
        myfile << resSubstG.volume <<","<<resSubstR.volume<<",";
        myfile << resSubstG.heat_capacity_cp <<","<<resSubstR.heat_capacity_cp<<",";
        myfile << resSubstG.heat_capacity_cv <<","<<resSubstR.heat_capacity_cv<<"\n";

        T +=25;

    } while (T<=800);

    myfile.close();


#endif
// +++ END Test H2O_HGKgems Vs H2O_HGKreaktoro +++

    // +++ Test H2O_WP95reaktoro Vs H2O_HGKreaktoro +++
    //#define TEST_H2O_HGK_VS_H2O_WP95
    #ifdef TEST_H2O_HGK_VS_H2O_WP95

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);
    WaterWP95reaktoro H2OWP95reaktoro ( water );

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);
    WaterHGKreaktoro H2OHGKreaktoro ( water );

    double T, P;
    T = 100+273.15;
    P = 5000e5;
    ThermoPropertiesSubstance resWP95, resHGKreak;

//    ElectroPropertiesSolvent resSolvG, resSolvR;

    ofstream myfile;
    myfile.open ("H2Oprop_new.csv");

//    string path = argv[0];
//    string cut  = "unit01/";

//    path = path.substr(0, path.size() - cut.size());
//    path = "exec rm -r " + path + output_path +"*";
////    system(path.c_str());

    myfile <<"T,P,G0wp95,G0hgkR,H0wp95,H0hgkR,S0wp95,S0hgkR,A0wp95,A0hgkR,U0wp95,U0hgkR,V0wp95,V0hgkR,Cp0wp95,Cp0hgkR,Cv0wp95,Cv0hgkR\n";

//    do
//    {
    do
    {
        myfile << T << ","<<P<<",";
        resWP95 = H2OWP95reaktoro.thermoPropertiesSubstance(T, P, 1);
        resHGKreak = H2OHGKreaktoro.thermoPropertiesSubstance(T, P, 1);

//        resSolvG = H2OWP95reaktoro.electroPropertiesSolvent(T,P);
//        resSolvR = H2OHGKreaktoro.electroPropertiesSolvent(T,P);

        myfile << resWP95.gibbs_energy <<","<<resHGKreak.gibbs_energy<<",";
        myfile << resWP95.enthalpy <<","<<resHGKreak.enthalpy<<",";
        myfile << resWP95.entropy <<","<<resHGKreak.entropy<<",";
        myfile << resWP95.helmholtz_energy <<","<<resHGKreak.helmholtz_energy<<",";
        myfile << resWP95.internal_energy <<","<<resHGKreak.internal_energy<<",";
        myfile << resWP95.volume <<","<<resHGKreak.volume<<",";
        myfile << resWP95.heat_capacity_cp <<","<<resHGKreak.heat_capacity_cp<<",";
        myfile << resWP95.heat_capacity_cv <<","<<resHGKreak.heat_capacity_cv<<"\n";

        T +=25;

    } while (T<=800);
//        T = 25;
//        P +=500;
//    } while (P<=0);

    myfile.close();


#endif
// +++ END Test H2O_WP95reaktoro Vs H2O_HGKreaktoro +++


    cout << "Bye World!" << endl;

    return 0;
}

