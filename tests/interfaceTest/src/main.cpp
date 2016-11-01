#include <iostream>
#include <fstream>
#include <sys/time.h>
// TCorrPT include
//#include "Thermo.h"
// GEMS4R include
//#include "node.h"
// TCorrPT output include
//#include "Common/OutputToCSV.h"
//#include "ThermoPropertiesUnits.h"
#include "Interfaces/Interface.h"

using namespace std;
using namespace TCorrPT;

int main(int argc, char *argv[])
{
    struct timeval start, middle, end;
    gettimeofday(&start, NULL);

    cout << "Hello World!" << endl;

    string file = "Resources/test_multi_new.VertexSubstance.json";
    char config_json_file_path[256] = "Resources/GEMS4/TestMulti";


    Database temp(file);
    Database tdb;
    vector<Substance> vSubst = temp.getSubstances();
    std::vector<string> substanceSymbols;

    /// adding the names of the solvent to be used in the aqueous species properties
    /// calculations
    for (int i = 0; i < vSubst.size(); i++)
    {
        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
        vSubst[i].setSolventSymbol("H2O@");
        tdb.addSubstance(vSubst[i]);
        substanceSymbols.push_back(vSubst[i].symbol());
    }

    double T = 25;
    double P = 1;

    int c = 0;

    Interface API (tdb);

//    API.thermoCalculate("H2O@", T, P, "entropy").toCSV();

//    double S = API.thermoCalculate("H2O@", T, P, "entropy").toDouble();
    API.addTP_pairs(25, 1);
    API.addSubstanceSymbols(substanceSymbols);
    API.addPropertyName("gibbs_energy");

    API.thermoCalculate().toCSV();

//    propertiesUnits["tmeperature"] = "K";

    /// Used to test different waters solvent models
//    Substance water;
//    water.setName("water");
//    water.setSymbol("H2O@_");
//    water.setFormula("H2O");
//    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
//    water.setAggregateState(AggregateState::type::AQUEOUS);

//    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_WJNR);

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);

//    tdb.addSubstance(water);

//    vector<Substance> vSubst = tdb.getSubstances();

//    for (int i = 0; i < vSubst.size(); i++)
//    {
//        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
//        vSubst[i].setSolventSymbol(water.symbol());

//        tdb2.addSubstance(vSubst[i]);
//    }

    Thermo thermo (tdb);
    ThermoPropertiesSubstance result_tcorrpt, tps_gems;

//    OutputToCSV out (argv[0]);
//    out.openThermoPropertiesSubstanceFile("CompareP0_T5_370.csv");

    P=1;
    gettimeofday(&middle, NULL);
    double delta = ((middle.tv_sec  - start.tv_sec) * 1000000u +
             middle.tv_usec - start.tv_usec) / 1.e6;

//    P = 4000; T = 400;
//    // test PRSV
//    result_tcorrpt = thermo.thermoPropertiesSubstance(T,P,"CO2hp");



    gettimeofday(&end, NULL);
    double delta_calc = ((end.tv_sec  - middle.tv_sec) * 1000000u +
             end.tv_usec - middle.tv_usec) / 1.e6;



//    cout << "==================================================================" << endl;
//    cout << "+ Time for GEMS4R and TCorrPT initialization: " << delta << "s "<< endl;
//    cout << "+ Time for "<< c << " T-P calculations: "<< delta_calc << "s " << endl;
//    cout << "+ Total time: " << delta + delta_calc << "s "<< endl;
//    cout << "==================================================================" << endl<< endl;



    return 0;
}

