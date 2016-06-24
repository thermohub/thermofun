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

    double T = 25;
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

    result = thermo.thermoPropertiesSubstance(T, P, "Al+3");

    cout << "\n Al+3 " << "T:" << T << " C, P:"<< P << " bar"<< endl;
    cout << "G0: "  << result.gibbs_energy << endl;
    cout << "H0: "  << result.enthalpy << endl;
    cout << "S0: "  << result.entropy << endl;
    cout << "Cp0: " << result.heat_capacity_cp << endl;
    cout << "Cv0: " << result.heat_capacity_cv << endl;
    cout << "V0: " << result.volume << endl;
    cout << "A0: " << result.helmholtz_energy << endl;
    cout << "U0: " << result.internal_energy << endl;

    OutputToCSV out (argv[0]);
    out.openThermoPropertiesSubstanceFile("ThermoPropSubst.csv");
    out.writeThermoPropertiesSubstance("Al+3", T, P, result);

    result = thermo.thermoPropertiesSubstance(T, P, "CO2@AD");

    cout << "\n CO2@AD "  << "T:" << T << " C, P:"<< P << " bar"<< endl;
    cout << "G0: "  << result.gibbs_energy << endl;
    cout << "H0: "  << result.enthalpy << endl;
    cout << "S0: "  << result.entropy << endl;
    cout << "Cp0: " << result.heat_capacity_cp << endl;
    cout << "Cv0: " << result.heat_capacity_cv << endl;
    cout << "V0: " << result.volume << endl;
    cout << "A0: " << result.helmholtz_energy << endl;
    cout << "U0: " << result.internal_energy << endl;

    result = thermo.thermoPropertiesSubstance(T, P, "Gibbsite");

    cout << "\n Gibbsite "  << "T:" << T << " C, P:"<< P << " bar"<< endl;
    cout << "G0: "  << result.gibbs_energy << endl;
    cout << "H0: "  << result.enthalpy << endl;
    cout << "S0: "  << result.entropy << endl;
    cout << "Cp0: " << result.heat_capacity_cp << endl;
    cout << "Cv0: " << result.heat_capacity_cv << endl;
    cout << "V0: " << result.volume << endl;
    cout << "A0: " << result.helmholtz_energy << endl;
    cout << "U0: " << result.internal_energy << endl;

    result = thermo.thermoPropertiesSubstance(T, P, "H2O@");

    cout << "\n H2O@ "  << "T:" << T << " C, P:"<< P << " bar"<< endl;
    cout << "G0: "  << result.gibbs_energy << endl;
    cout << "H0: "  << result.enthalpy << endl;
    cout << "S0: "  << result.entropy << endl;
    cout << "Cp0: " << result.heat_capacity_cp << endl;
    cout << "Cv0: " << result.heat_capacity_cv << endl;
    cout << "V0: " << result.volume << endl;
    cout << "A0: " << result.helmholtz_energy << endl;
    cout << "U0: " << result.internal_energy << endl;

    result = thermo.thermoPropertiesSubstance(T, P, "K+");

    cout << "\n K+ "  << "T:" << T << " C, P:"<< P << " bar"<< endl;
    cout << "G0: "  << result.gibbs_energy << endl;
    cout << "H0: "  << result.enthalpy << endl;
    cout << "S0: "  << result.entropy << endl;
    cout << "Cp0: " << result.heat_capacity_cp << endl;
    cout << "Cv0: " << result.heat_capacity_cv << endl;
    cout << "V0: " << result.volume << endl;
    cout << "A0: " << result.helmholtz_energy << endl;
    cout << "U0: " << result.internal_energy << endl;

    cout << "Bye World!" << endl;

    return 0;
}

