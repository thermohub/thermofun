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

    Substance hcl;
    hcl.setName("HCl");
    hcl.setFormula("HCl@");
    hcl.setCharge(0);
    hcl.setSolventName("water");

    hcl.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    hcl.setAggregateState(AggregateState::type::AQUEOUS);

    hcl.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_CPT);
    hcl.setMethod_P(MethodCorrP_Thrift::type::CPM_AKI);
    hcl.setMethod_T(MethodCorrT_Thrift::type::CTM_CST);

    vd Interval, Coeff, ADparam;
    vvd TCpInterval,
    CpCoefficients;

    Interval = {0,1500};
    Coeff = {0.28821, 0.0217563, -365734, 464.476, -4.74975e-06};
    ADparam = {-0.28, 11.642, -7.4244};

    TCpInterval.push_back(Interval);
    CpCoefficients.push_back(Coeff);

    ThermoParametersSubstance prs;
    prs.Cp_coeff = CpCoefficients;
    prs.temperature_intervals = TCpInterval;
    prs.Cp_nonElectrolyte_coeff = ADparam;
    hcl.setThermoParameters(prs);

    double T, P;
    T = 35;
    P = 1;

    ThermoPropertiesSubstance rHCl, TrPrHCl;

    TrPrHCl.gibbs_energy = -126045;
    TrPrHCl.volume = 2.06286;
    TrPrHCl.enthalpy = -165244;
    TrPrHCl.entropy = 45.408;
    TrPrHCl.heat_capacity_cp = 46.944;

    hcl.setThermoReferenceProperties(TrPrHCl);
    hcl.setReferenceT(25);
    hcl.setReferenceP(1);

    EmpiricalCpIntegration CpHCl (hcl);
    rHCl = CpHCl.thermoProperties(T, P);

    WaterIdealGasWoolley wig ( water );

    ThermoPropertiesSubstance wigp = wig.thermoProperties(T, P);

    PropertiesSolvent wp0 = H2OHGKgems.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp1 = H2OHGKreaktoro.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp2 = H2OWP95reaktoro.propertiesSolvent(T, P, 0);

    ThermoPropertiesSubstance wtp = H2OHGKgems.thermoPropertiesSubstance(T, P, 0);

    AkinfievDiamondEOS hcl_AD (hcl);

    ThermoPropertiesSubstance result;

    result = hcl_AD.thermoProperties(T, P, rHCl, wtp, wigp, wp0);

//    ThermoPropertiesSubstance wp3 = H2OWP95reaktoro.thermoPropertiesSubstance(T, P, 1);

//    WaterJNreaktoro H2OJNreaktoro ( water );
//    WaterJNgems     H2OJNgems ( water );

//    ElectroPropertiesSolvent wes0 = H2OJNgems.electroPropertiesSolvent(T, P);
//    ElectroPropertiesSolvent wes1 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp1);
//    ElectroPropertiesSolvent wes2 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp2);

//    SoluteHKFgems al3HKFgems (hcl);
//    SoluteHKFreaktoro al3HKFreaktoro (hcl);

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

