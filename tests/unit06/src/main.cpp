#include <iostream>
#include <fstream>
//#include "tcorrpt.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "tcorrpt_global.h"
#include "Thermo.h"
#include "Common/OutputToCSV.h"

using namespace std;
using namespace TCorrPT;

int main(int argc, char *argv[])
{

    cout << "Hello World!" << endl;

    string file = argv[1];

    Database tdb/*(file)*/;
    Database tdb2;

    double T = 1000;
    double P = 60000;

    Substance water, water2;
    water.setName("water");
    water.setSymbol("H2O@");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WSV14);

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WZD);

    tdb.addSubstance(water);

    water2.setName("water2");
    water2.setSymbol("H2O@2");
    water2.setFormula("H2O2");
    water2.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water2.setAggregateState(AggregateState::type::AQUEOUS);

    water2.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WJNR);

    water2.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);

    tdb.addSubstance(water2);

    ThermoPropertiesSubstance result, result2, result3;
    PropertiesSolvent ps, ps2;
    ElectroPropertiesSolvent eps, eps2;

    Thermo thermo (tdb);

    result = thermo.thermoPropertiesSubstance(T, P, "H2O@");

    ps = thermo.propertiesSolvent(T, P, "H2O@");

    eps = thermo.electroPropertiesSolvent(T, P, "H2O@");
    eps2 = thermo.electroPropertiesSolvent(T, P, "H2O@2");

    ps2 = thermo.propertiesSolvent(T, P, "H2O@2");

    result2 = thermo.thermoPropertiesSubstance(T, P, "H2O@2");


    Substance al3;
    al3.setName("Al+3");
    al3.setFormula("Al+3");
    al3.setSymbol("Al+3");
    al3.setCharge(3);
    al3.setSolventSymbol("H2O@2");

    al3.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    al3.setAggregateState(AggregateState::type::AQUEOUS);

    al3.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HKF);
    al3.setMethod_P(MethodCorrP_Thrift::type::CPM_HKF);
    al3.setMethod_T(MethodCorrT_Thrift::type::CTM_HKF);

    vector<double> hkf = {-0.33802, -1700.71, 14.5185, -20758, 10.7, -80600, 275300 };

    ThermoPropertiesSubstance ref;

    ref.gibbs_energy = -115609*cal_to_J;
    ref.enthalpy = -126834*cal_to_J;
    ref.entropy = -77.7*cal_to_J;


    ThermoParametersSubstance prs;
    prs.HKF_parameters = hkf;
    al3.setThermoParameters(prs);
    al3.setThermoReferenceProperties(ref);

    tdb.addSubstance(al3);

    result3 = thermo.thermoPropertiesSubstance(T, P, "Al+3");

    cout << "Bye World!" << endl;

    return 0;
}

