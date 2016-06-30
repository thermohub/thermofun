#include <iostream>
#include <fstream>
//#include "tcorrpt.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "tcorrpt_global.h"
#include "Thermo.h"

using namespace std;
using namespace TCorrPT;

int main(int argc, char *argv[])
{

    cout << "Hello World!" << endl;

    Substance water;
    water.setName("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

#define TEST_AD_neutral
#ifdef TEST_AD_neutral

    WaterHGK H2OHGKgems ( water );

    WaterHGKreaktoro H2OHGKreaktoro ( water );

    WaterWP95reaktoro H2OWP95reaktoro ( water );

    Substance co2;
    co2.setName("CO2");
    co2.setFormula("CO2@");
    co2.setCharge(0);
    co2.setSolventSymbol("CO2@");

    co2.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    co2.setAggregateState(AggregateState::type::AQUEOUS);

    co2.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_CPT);
    co2.setMethod_P(MethodCorrP_Thrift::type::CPM_AKI);
    co2.setMethod_T(MethodCorrT_Thrift::type::CTM_CST);

    vd Interval, Coeff, ADparam;
    vvd TCpInterval,
    CpCoefficients;

    Interval = {0,1500};
    Coeff = {65.6823, 0.0111973, 280403, -599.129, -3.91572e-06};
    ADparam = {-0.085, -8.8321, 11.2684};

    TCpInterval.push_back(Interval);
    CpCoefficients.push_back(Coeff);

    ThermoParametersSubstance prs;
    prs.Cp_coeff = CpCoefficients;
    prs.temperature_intervals = TCpInterval;
    prs.Cp_nonElectrolyte_coeff = ADparam;
    co2.setThermoParameters(prs);

    double T, P;
    T = 25;
    P = 1;

    ThermoPropertiesSubstance rCO2, TrPrCO2;

    TrPrCO2.gibbs_energy = -386030;
    TrPrCO2.volume = 3.27028;
    TrPrCO2.enthalpy = -413337;
    TrPrCO2.entropy = 119.29;
    TrPrCO2.heat_capacity_cp = 209.749;

    co2.setThermoReferenceProperties(TrPrCO2);
    co2.setReferenceT(25);
    co2.setReferenceP(1);

    EmpiricalCpIntegration CpCO2 (co2);
    rCO2 = CpCO2.thermoProperties(T, P);

    WaterIdealGasWoolley wig ( water );

    ThermoPropertiesSubstance wigp = wig.thermoProperties(T, P);

    PropertiesSolvent wp0 = H2OHGKgems.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp1 = H2OHGKreaktoro.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp2 = H2OWP95reaktoro.propertiesSolvent(T, P, 0);

    ThermoPropertiesSubstance wtp = H2OHGKgems.thermoPropertiesSubstance(T, P, 0);

    SoluteAkinfievDiamondEOS CO2_AD (co2);

    ThermoPropertiesSubstance result;

    result = CO2_AD.thermoProperties(T, P, rCO2, wtp, wigp, wp0);

//    ThermoPropertiesSubstance wp3 = H2OWP95reaktoro.thermoPropertiesSubstance(T, P, 1);

//    WaterJNreaktoro H2OJNreaktoro ( water );
//    WaterJNgems     H2OJNgems ( water );

//    ElectroPropertiesSolvent wes0 = H2OJNgems.electroPropertiesSolvent(T, P);
//    ElectroPropertiesSolvent wes1 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp1);
//    ElectroPropertiesSolvent wes2 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp2);

//    SoluteHKFgems al3HKFgems (CO2);
//    SoluteHKFreaktoro al3HKFreaktoro (CO2);

//    ThermoPropertiesSubstance resultG00, resultG01, resultG02, resultG10, resultG11, resultG12, resultG20, resultG21, resultG22,
//            resultR11, resultR12, resultR22, resultR21;

//    resultG00  = al3HKFgems.thermoProperties(T, P, wp0, wes0);
//    resultG01  = al3HKFgems.thermoProperties(T, P, wp0, wes1);
//    resultG02  = al3HKFgems.thermoProperties(T, P, wp0, wes2);
//    resultG10  = al3HKFgems.thermoProperties(T, P, wp1, wes0);
//    resultG11  = al3HKFgems.thermoProperties(T, P, wp1, wes1);
//    resultG12  = al3HKFgems.thermoProperties(T, P, wp1, wes2);
//    resultG20  = al3HKFgems.thermoProperties(T, P, wp2, wes0);
//    resultG21  = al3HKFgems.thermoProperties(T, P, wp2, wes1);
//    resultG22  = al3HKFgems.thermoProperties(T, P, wp2, wes2);
//    resultR11  = al3HKFreaktoro.thermoProperties(T, P, wp1, wes1);
//    resultR12  = al3HKFreaktoro.thermoProperties(T, P, wp1, wes2);
//    resultR22  = al3HKFreaktoro.thermoProperties(T, P, wp2, wes2);
//    resultR21  = al3HKFreaktoro.thermoProperties(T, P, wp2, wes1);

//    WaterIdealGasWoolley wig ( water );

//    ThermoPropertiesSubstance wigp = wig.thermoProperties(T, P);

#endif


    cout << "Bye World!" << endl;

    return 0;
}

