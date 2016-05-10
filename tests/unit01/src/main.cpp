#include <iostream>
//#include "tcorrpt.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "tcorrpt_global.h"
#include "Thermo.h"

using namespace std;
using namespace TCorrPT;

int main()
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
    double currentP = 2000,                    ///<
           currentT = 400;                    ///<

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

    Thermo thermo (dtb);

    // Test error out of the interval
//    currentT = 2005;
    result2 = thermo.thermoPropertiesSubstance(currentT, currentP, "Corundum");

    // Test error no substance
//    result2 = thermo.thermoPropertiesSubstance(currentT, currentP, "alala");

#endif
// +++ END test CP +++

    // +++ Test H2O_HGK +++
    #define TEST_H2O_HGK
    #ifdef TEST_H2O_HGK

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    ThermoPropertiesSubstance h2o_data_atPrTr;
    ThermoPropertiesSubstance result2;
    ThermoPropertiesSolvent result1, result3;

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

    Thermo thermo (dtb);
    result2 = thermo.thermoPropertiesSubstance(25, 1, "water");

    water.setThermoReferenceProperties(result2);

    WaterHGK waterHGK(water);

    result1 = waterHGK.thermoPropertiesSolvent(25, 1);

    result3 = thermo.thermoPropertiesSolvent(25, 1, "water");

#endif
// +++ END test H2O_HGK +++

    cout << "Bye World!" << endl;

    return 0;
}

