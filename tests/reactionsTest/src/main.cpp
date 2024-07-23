#include <iostream>
#include "ThermoFun/ThermoFun.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    string databaseFile = "Resources/TestReactionsPSI.VertexReaction.json";

    Database thermoDB(databaseFile);

    ThermoEngine thermo(thermoDB);

    ThermoPropertiesReaction tpr;

    double T = 298.15;
    double P = 1e5;

    tpr = thermo.thermoPropertiesReaction(T, P, "CaSiO3@");

    std::cout << tpr.log_equilibrium_constant << endl << tpr.reaction_gibbs_energy << endl;

    return 0;
}

