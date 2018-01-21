#include "ThermoFun.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    string databaseFile = "Resources/test_multi_new_elem.VertexSubstance.json";
    Interface thermo(databaseFile);
    thermo.setSolventSymbol("H2O@");

    thermo.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

    double T = 100;
    double P = 50;

    double S = thermo.thermoPropertiesSubstance( 25, 1, "H2O@", "entropy").toDouble();

    thermo.thermoPropertiesSubstance({{25, 1}, {40, 1}, {70, 100}, {90, 100}, {100, 100}}, {"Al+3", "OH-", "SiO2@"}, {"gibbs_energy","entropy", "volume", "enthalpy"}).toCSV("results.csv");

    thermo.thermoPropertiesSubstance({{25, 1}, {40, 1}, {70, 100}, {90, 100}, {100, 100}}, {"Al+3", "OH-", "SiO2@"}, {"gibbs_energy","entropy", "volume", "enthalpy"}).toCSVTransposed("resultsTransposed.csv");

//    API.addTP_pairs(25, 1);
//    API.addSubstances(substanceSymbols);
//    API.addProperties({"gibbs_energy","entropy"});
//    API.thermoCalculate().toCSV();

    return 0;
}

