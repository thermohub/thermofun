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

    // +++ Test HKFgems +++
//    #define TEST_HKFgems
    #ifdef TEST_HKFgems

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);
    WaterHGK H2OHGKgems ( water );

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);
//    WaterHGKreaktoro H2OHGKreaktoro ( water );

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);
//    WaterWP95reaktoro H2OWP95reaktoro ( water );

    Substance al3;
    al3.setName("Al+3");
    al3.setFormula("Al+3");
    al3.setCharge(3);
    al3.setSolventName("water");

    al3.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    al3.setAggregateState(AggregateState::type::AQUEOUS);

    al3.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    al3.setMethod_P(MethodCorrP_Thrift::type::CPM_HKF);
    al3.setMethod_T(MethodCorrT_Thrift::type::CTM_HKF);

    ThermoPropertiesSubstance al3_PrTr;

    ParamsHKF hkf;

    hkf.Gf = -116510.03824091777;
    hkf.Hf = -128769.03423328472;
    hkf.Sr = -81.202913652642508;
    hkf.a1 = -0.33984;
    hkf.a2 = -1607.89;
    hkf.a3 = 12.0699;
    hkf.a4 = -21143;
    hkf.c1 = 14.4295;
    hkf.c2 = -88523;
    hkf.wref = 274030;

    ThermoParametersSubstance prs;
    prs.HKF_parameters = hkf;
    al3.setThermoParameters(prs);

    double T, P;
    T = 25;
    P = 1;

    PropertiesSolvent wp = H2OHGKgems.propertiesSolvent(T, P, 0);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WJNG);

    WaterJNgems H2OJNgems ( water );

    ElectroPropertiesSolvent wes = H2OJNgems.electroPropertiesSolvent(T, P);

    SoluteHKFgems al3HKFgems (al3);

    ThermoPropertiesSubstance result1;

    result1 = al3HKFgems.thermoProperties(T, P, wp, wes);

#endif

    // +++ Test HKFgems +++
//    #define TEST_HKFreaktoro
    #ifdef TEST_HKFreaktoro

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);
//    WaterHGK H2OHGKgems ( water );
    //

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);
    WaterHGKreaktoro H2OHGKreaktoro ( water );

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);
//    WaterWP95reaktoro H2OWP95reaktoro ( water );

    Substance al3;
    al3.setName("Al+3");
    al3.setFormula("Al+3");
    al3.setCharge(3);
    al3.setSolventName("water");

    al3.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    al3.setAggregateState(AggregateState::type::AQUEOUS);

    al3.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    al3.setMethod_P(MethodCorrP_Thrift::type::CPM_HKF);
    al3.setMethod_T(MethodCorrT_Thrift::type::CTM_HKF);

    ThermoPropertiesSubstance al3_PrTr;

    ParamsHKF hkf;

    hkf.Gf = -116510.03824091777;
    hkf.Hf = -128769.03423328472;
    hkf.Sr = -81.202913652642508;
    hkf.a1 = -0.33984;
    hkf.a2 = -1607.89;
    hkf.a3 = 12.0699;
    hkf.a4 = -21143;
    hkf.c1 = 14.4295;
    hkf.c2 = -88523;
    hkf.wref = 274030;

    ThermoParametersSubstance prs;
    prs.HKF_parameters = hkf;
    al3.setThermoParameters(prs);

    double T, P;
    T = 25;
    P = 1;

    PropertiesSolvent wp = H2OHGKreaktoro.propertiesSolvent(T, P, 0);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WJNG);

    WaterJNreaktoro H2OJNreaktoro ( water );

    ElectroPropertiesSolvent wes = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp);

    SoluteHKFreaktoro al3HKFreaktoro (al3);

    ThermoPropertiesSubstance result1;

    result1 = al3HKFreaktoro.thermoProperties(T, P, wp, wes);

#endif

#define TEST_HKF_ALL
#ifdef TEST_HKF_ALL

    WaterHGK H2OHGKgems ( water );

    WaterHGKreaktoro H2OHGKreaktoro ( water );

    WaterWP95reaktoro H2OWP95reaktoro ( water );

    Substance al3;
    al3.setName("Al+3");
    al3.setFormula("Al+3");
    al3.setCharge(3);
    al3.setSolventName("water");

    al3.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    al3.setAggregateState(AggregateState::type::AQUEOUS);

    al3.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    al3.setMethod_P(MethodCorrP_Thrift::type::CPM_HKF);
    al3.setMethod_T(MethodCorrT_Thrift::type::CTM_HKF);

    ParamsHKF hkf;

    hkf.Gf = -116510.03824091777;
    hkf.Hf = -128769.03423328472;
    hkf.Sr = -81.202913652642508;
    hkf.a1 = -0.33984;
    hkf.a2 = -1607.89;
    hkf.a3 = 12.0699;
    hkf.a4 = -21143;
    hkf.c1 = 14.4295;
    hkf.c2 = -88523;
    hkf.wref = 274030;

    ThermoParametersSubstance prs;
    prs.HKF_parameters = hkf;
    al3.setThermoParameters(prs);

    double T, P;
    T = 25;
    P = 1;

    PropertiesSolvent wp0 = H2OHGKgems.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp1 = H2OHGKreaktoro.propertiesSolvent(T, P, 0);
    PropertiesSolvent wp2 = H2OWP95reaktoro.propertiesSolvent(T, P, 0);

    WaterJNreaktoro H2OJNreaktoro ( water );
    WaterJNgems     H2OJNgems ( water );

    ElectroPropertiesSolvent wes0 = H2OJNgems.electroPropertiesSolvent(T, P);
    ElectroPropertiesSolvent wes1 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp1);
    ElectroPropertiesSolvent wes2 = H2OJNreaktoro.electroPropertiesSolvent(T, P, wp2);

    SoluteHKFgems al3HKFgems (al3);
    SoluteHKFreaktoro al3HKFreaktoro (al3);

    ThermoPropertiesSubstance resultG00, resultG01, resultG02, resultG10, resultG11, resultG12, resultG20, resultG21, resultG22,
            resultR11, resultR12, resultR22, resultR21;

    resultG00  = al3HKFgems.thermoProperties(T, P, wp0, wes0);
    resultG01  = al3HKFgems.thermoProperties(T, P, wp0, wes1);
    resultG02  = al3HKFgems.thermoProperties(T, P, wp0, wes2);
    resultG10  = al3HKFgems.thermoProperties(T, P, wp1, wes0);
    resultG11  = al3HKFgems.thermoProperties(T, P, wp1, wes1);
    resultG12  = al3HKFgems.thermoProperties(T, P, wp1, wes2);
    resultG20  = al3HKFgems.thermoProperties(T, P, wp2, wes0);
    resultG21  = al3HKFgems.thermoProperties(T, P, wp2, wes1);
    resultG22  = al3HKFgems.thermoProperties(T, P, wp2, wes2);
    resultR11  = al3HKFreaktoro.thermoProperties(T, P, wp1, wes1);
    resultR12  = al3HKFreaktoro.thermoProperties(T, P, wp1, wes2);
    resultR22  = al3HKFreaktoro.thermoProperties(T, P, wp2, wes2);
    resultR21  = al3HKFreaktoro.thermoProperties(T, P, wp2, wes1);

#endif


    cout << "Bye World!" << endl;

    return 0;
}

