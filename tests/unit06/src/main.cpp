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

    double T = 400;
    double P = 3000;

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

    ThermoPropertiesSubstance result, result2;
    PropertiesSolvent ps, ps2;
    ElectroPropertiesSolvent eps, eps2;

    Thermo thermo (tdb);

    result = thermo.thermoPropertiesSubstance(T, P, "H2O@");

    ps = thermo.propertiesSolvent(T, P, "H2O@");

    eps = thermo.electroPropertiesSolvent(T, P, "H2O@");
    eps2 = thermo.electroPropertiesSolvent(T, P, "H2O@2");

    ps2 = thermo.propertiesSolvent(T, P, "H2O@2");

    result2 = thermo.thermoPropertiesSubstance(T, P, "H2O@2");

//    vector<Substance> vSubst = tdb.getSubstances();

//    for (int i = 0; i < vSubst.size(); i++)
//    {
//        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
//        vSubst[i].setSolventSymbol(water.symbol());

//        tdb2.addSubstance(vSubst[i]);
//    }

//    ThermoPropertiesSubstance result;

//    Thermo thermo (tdb2);

//    OutputToCSV out (argv[0]);
//    out.openThermoPropertiesSubstanceFile("ThermoPropSubstSUBCRT.csv");

//    P = 2000;
//    result = thermo.thermoPropertiesSubstance(577, P, "Quartz");

//    do {

//        for (int i = 0; i < vSubst.size(); i++)
//        {
//            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
//            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);
//            P = 0;
//        }

//        T +=5;
//    } while (T <= 370);

//    out.closeThermoPropertiesSubstanceFile();

//    OutputToCSV out2 (argv[0]);
//    out2.openThermoPropertiesSubstanceFile("ThermoPropSubstSUPCRT.csv");


//    for (int i = 0; i < vSubst.size(); i++)
//    {
//        P= 500;
//        do
//        {
//            T= 5;
//            do {
//               out2.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol()) );
//               T +=5;
//            } while (T <= 350);
//            P +=500;
//        } while (P <=5000);
//    }


//    out2.closeThermoPropertiesSubstanceFile();
    cout << "Bye World!" << endl;

    return 0;
}

