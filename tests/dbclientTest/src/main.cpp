#include "DBClient/DBClient.h"
#include "Interfaces/Interface.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    DBClient dbc("./Resources/ThermoFun.ini");

    Database db = dbc.getDatabase(15);
    db.setAllAqSubstanceSolventSymbol("H2O@");

    Thermo th(DBClient("./Resources/ThermoFun.ini").getDatabase(15));

    th.setSolventSymbolForAllAqSubst("H2O@");

    double T = 25;
    double P = 1;

    ThermoPropertiesSubstance MgSi, CaSi, FeHSi, RaC, RaS, SiO;

//    ThermoPropertiesSubstance tps = th.thermoPropertiesSubstance(T, P, "Mg+2");

    MgSi = th.thermoPropertiesSubstance(T, P, "MgSiO3@");

    CaSi = th.thermoPropertiesSubstance(T, P, "CaSiO3@");

    FeHSi = th.thermoPropertiesSubstance(T, P, "FeHSiO3+2");

    RaC = th.thermoPropertiesSubstance(T, P, "RaCO3");

    RaS = th.thermoPropertiesSubstance(T, P, "RaSO4");

    SiO = th.thermoPropertiesSubstance(T, P, "SiO3-2");


    cout << "Bye World!" << endl;

    return 0;
}

