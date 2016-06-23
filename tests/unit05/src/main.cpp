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

    Database tdb(file);
    Database tdb2;

    double T = 5;
    double P = 0;

    Substance water;
    water.setName("water");
    water.setSymbol("H2O@");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WJNG);

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
    out.openThermoPropertiesSubstanceFile("ThermoPropSubst.csv");

    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol()) );
        }

        T +=5;
    } while (T <= 370);

    cout << "Bye World!" << endl;

    return 0;
}

