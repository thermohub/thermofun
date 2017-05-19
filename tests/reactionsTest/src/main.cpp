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

    cout << tpr.log_equilibrium_constant << endl << tpr.reaction_gibbs_energy << endl;

    return 0;
}

