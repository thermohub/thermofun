#include "Interfaces/Interface.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    string databaseFile = "Resources/TestReactionsPSI.VertexReaction.json";

    Database thermoDB(databaseFile);

    Thermo thermo(thermoDB);

    ThermoPropertiesReaction tpr;

    double T = 25;
    double P = 1;

    tpr = thermo.thermoPropertiesReaction(T, P, "CaSiO3@");



    cout << "here" << endl;
//    thermo.setSolventSymbolForAqSubst("H2O@");

//    double T = 100;
//    double P = 50;

//    double S = thermo.calculateProperties("H2O@", 25, 1, "entropy").toDouble();

//    thermo.calculateProperties({"Al+3", "OH-", "SiO2@"}, {"gibbs_energy","entropy", "volume"}, T, P).toCSV("results.csv");

//    API.addTP_pairs(25, 1);
//    API.addSubstances(substanceSymbols);
//    API.addProperties({"gibbs_energy","entropy"});
//    API.thermoCalculate().toCSV();

    return 0;
}

