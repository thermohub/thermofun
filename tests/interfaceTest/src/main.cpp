#include "Interfaces/Interface.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    string file = "Resources/test_multi_new.VertexSubstance.json";

    Database tdb(file);

    double T = 250;
    double P = 1000;

    Interface API (tdb);

//    API.addProperties();

    API.setSolventSymbolForAqSubst("H2O_WP");

    API.addSubstances({"","",});

//    API.addTP_pair(50, 500);

    API.thermoCalculate().to2DVectorDouble();

    Thermo thermo(tdb);

//    thermo.thermoPropertiesSubstance();

    double S = API.thermoCalculate("H2O@", T, P, "entropy").toDouble();

//    API.addTP_pairs(25, 1);
//    API.addSubstances(substanceSymbols);
//    API.addProperties({"gibbs_energy","entropy"});
//    API.thermoCalculate().toCSV();

    return 0;
}

