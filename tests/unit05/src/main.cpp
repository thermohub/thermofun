#include <iostream>
#include <fstream>
//#include "tcorrpt.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "ThermoFun_global.h"
#include "Thermo.h"
#include "Common/OutputToCSV.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{

    cout << "Hello World!" << endl;

    string file = argv[1];

    Database tdb(file);
    Database tdb2;

    double T = 100;
    double P = 5000;

    Substance water;
    water.setName("water");
    water.setSymbol("H2O@");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_WJNG);

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);

    tdb.addSubstance(water);

    vector<Substance> vSubst = tdb.getSubstances();

    for (int i = 0; i < vSubst.size(); i++)
    {
        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
        vSubst[i].setSolventSymbol(water.symbol());

        tdb2.addSubstance(vSubst[i]);
    }

    ThermoPropertiesSubstance result;

    Thermo thermo (tdb2);

    OutputToCSV out (argv[0]);
    out.openThermoPropertiesSubstanceFile("ThermoPropSubstSUBCRT_JNG_WAT.csv");

    P = 5000;
    result = thermo.thermoPropertiesSubstance(577, P, "Al+3");

    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);
        }

        T +=5;
    } while (T <= 800);

    out.closeThermoPropertiesSubstanceFile();

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

