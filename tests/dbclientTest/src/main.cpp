#include "ThermoFun.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    DBClient dbc("./Resources/ThermoFun.ini");

    Database db = dbc.getDatabase(15);

    Thermo th(DBClient("./Resources/ThermoFun.ini").getDatabase(15));

    th.setSolventSymbol("H2O@");

    double T = 100;
    double P = 1000;

    ThermoPropertiesSubstance MgSi, CaSi, FeHSi, RaC, RaS, SiO, CaSi_FM;

    MgSi = th.thermoPropertiesSubstance(T, P, "MgSiO3@");

    CaSi = th.thermoPropertiesSubstance(T, P, "CaSiO3@");

    FeHSi = th.thermoPropertiesSubstance(T, P, "FeHSiO3+2");

    RaC = th.thermoPropertiesSubstance(T, P, "RaCO3");

    RaS = th.thermoPropertiesSubstance(T, P, "RaSO4");

    SiO = th.thermoPropertiesSubstance(T, P, "SiO3-2");

    CaSi = th.thermoPropertiesSubstance(T, P, "CaSiO3@_FM_test");

    DBClient dbc2 = DBClient ("./Resources/ThermoFun.ini");

    mapFormulaElements elem = dbc.parseSubstanceFormula("FeHSiO3+2");

    cout << "Bye World!" << endl;

    return 0;
}

