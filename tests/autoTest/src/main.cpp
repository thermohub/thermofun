#include <iostream>
#include <fstream>
//#include "tcorrpt.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "tcorrpt_global.h"
#include "Thermo.h"
#include "Common/OutputToCSV.h"
#include "node.h"

using namespace std;
using namespace TCorrPT;

auto compare (ThermoPropertiesSubstance result_gems, ThermoPropertiesSubstance result_tcorrpt, string symbol, double T, double P ) -> int
{
    int i = 1; double d;
    double tolerance = 1e-05;

    d = (fabs(result_gems.gibbs_energy.val - result_tcorrpt.gibbs_energy.val) / fabs(result_gems.gibbs_energy.val));
    if ( d > tolerance)
    {
        cout << "At T:"<< T << " C and P:" << P << " bar" << endl <<
                "For substance " << symbol << " the Gibbs energy calculated in GEMS (" << result_gems.gibbs_energy << ")\n"
                "is not the same as claulcated with the TCorrPT library (" << result_tcorrpt.gibbs_energy << ")\n"
                "d% = " << d*100 << endl << endl;

        i = -1;
    }

    d = (fabs(result_gems.entropy.val - result_tcorrpt.entropy.val) / fabs(result_gems.entropy.val));
    if ( d > tolerance)
    {
        cout << "At T:"<< T << " C and P:" << P << " bar" << endl <<
                "For substance " << symbol << " the entropy calculated in GEMS (" << result_gems.entropy << ")\n"
                "is not the same as claulcated with the TCorrPT library (" << result_tcorrpt.entropy << ")\n"
                "d% = " << d*100 << endl << endl;

        i = -1;
    }

    d = (fabs(result_gems.enthalpy.val - result_tcorrpt.enthalpy.val) / fabs(result_gems.enthalpy.val));
    if ( d > tolerance)
    {
        cout << "At T:"<< T << " C and P:" << P << " bar" << endl <<
                "For substance " << symbol << " the enthalpy calculated in GEMS (" << result_gems.enthalpy << ")\n"
                "is not the same as claulcated with the TCorrPT library (" << result_tcorrpt.enthalpy << ")\n"
                "d% = " << d*100 << endl << endl;

        i = -1;
    }

    d = (fabs(result_gems.heat_capacity_cp.val - result_tcorrpt.heat_capacity_cp.val) / fabs(result_gems.heat_capacity_cp.val));
    if ( d > tolerance)
    {
        cout << "At T:"<< T << " C and P:" << P << " bar" << endl <<
                "For substance " << symbol << " the heat capacity (Cp) calculated in GEMS (" << result_gems.heat_capacity_cp << ")\n"
                "is not the same as claulcated with the TCorrPT library (" << result_tcorrpt.heat_capacity_cp << ")\n"
                "d% = " << d*100 << endl << endl;

        i = -1;
    }

    return i;
}

int main(int argc, char *argv[])
{

    cout << "Hello World!" << endl;

    string file = argv[1];

    char config_json_file_path[256] = "Resources/GEMS4/TestMulti";

    TNode* node = new TNode();

    node->GEM_init(config_json_file_path);

    Database temp(file);

    Database tdb;

    vector<Substance> vSubst = temp.getSubstances();

    for (int i = 0; i < vSubst.size(); i++)
    {
        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
        vSubst[i].setSolventSymbol("H2O@");
        tdb.addSubstance(vSubst[i]);
    }

    double T;
    double P;

//    Substance water;
//    water.setName("water");
//    water.setSymbol("H2O@");
//    water.setFormula("H2O");
//    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
//    water.setAggregateState(AggregateState::type::AQUEOUS);

//    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CEM_WJNR);

//    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAR);

//    tdb.addSubstance(water);

//    vector<Substance> vSubst = tdb.getSubstances();

//    for (int i = 0; i < vSubst.size(); i++)
//    {
//        if (vSubst[i].substanceClass() == SubstanceClass::type::AQSOLUTE)
//        vSubst[i].setSolventSymbol(water.symbol());

//        tdb2.addSubstance(vSubst[i]);
//    }

    ThermoPropertiesSubstance result;

    Thermo thermo (tdb);

    OutputToCSV out (argv[0]);
    out.openThermoPropertiesSubstanceFile("CompareP0_T5_370.csv");
    ThermoPropertiesSubstance tps;
    int xCH;

    P=1;
    result = thermo.thermoPropertiesSubstance(195, P, "Al+3");

    P = 0; T = 5;
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, 0, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, 0, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, 0, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, 0, T+273.15);
            tps.volume           = node->DC_V0 (xCH, 0, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

//            P = 0;
        }
        P = 0;

        T +=5;
    } while (T <= 370);
    out.closeThermoPropertiesSubstanceFile();

    result = thermo.thermoPropertiesSubstance(150, P, "Al+3");

    P = 1; T = 5;
    out.openThermoPropertiesSubstanceFile("CompareP1_T5_200.csv");
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, P*1e05, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, P*1e05, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, P*1e05, T+273.15);
            tps.volume           = node->DC_V0 (xCH, P*1e05, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

            P = 1;
        }

        T +=5;
    } while (T <= 200);
    out.closeThermoPropertiesSubstanceFile();

//    xCH = node->DC_name_to_xCH("Al+3");
//    tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, 195+273.15, false);

    P = 500; T = 25;
    out.openThermoPropertiesSubstanceFile("CompareP500_T25_800.csv");
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, P*1e05, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, P*1e05, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, P*1e05, T+273.15);
            tps.volume           = node->DC_V0 (xCH, P*1e05, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

            P = 500;
        }

        T +=25;
    } while (T <= 800);
    out.closeThermoPropertiesSubstanceFile();

    P = 1000; T = 25;
    out.openThermoPropertiesSubstanceFile("CompareP1000_T25_800.csv");
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, P*1e05, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, P*1e05, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, P*1e05, T+273.15);
            tps.volume           = node->DC_V0 (xCH, P*1e05, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

            P = 1000;
        }

        T +=25;
    } while (T <= 800);
    out.closeThermoPropertiesSubstanceFile();

    P = 1000; T = 25;
    out.openThermoPropertiesSubstanceFile("CompareP2000_T25_800.csv");
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, P*1e05, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, P*1e05, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, P*1e05, T+273.15);
            tps.volume           = node->DC_V0 (xCH, P*1e05, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

            P = 2000;
        }

        T +=25;
    } while (T <= 800);
    out.closeThermoPropertiesSubstanceFile();

    P = 5000; T = 25;
    out.openThermoPropertiesSubstanceFile("CompareP5000_T25_800.csv");
    do {

        for (int i = 0; i < vSubst.size(); i++)
        {
            result = thermo.thermoPropertiesSubstance(T,P,vSubst[i].symbol());
            out.writeThermoPropertiesSubstance( vSubst[i].symbol(), T, P, result);

            xCH = node->DC_name_to_xCH(vSubst[i].symbol().c_str());
            tps.gibbs_energy     = node->DC_G0 (xCH, P*1e05, T+273.15, false);
            tps.enthalpy         = node->DC_H0 (xCH, P*1e05, T+273.15);
            tps.entropy          = node->DC_S0 (xCH, P*1e05, T+273.15);
            tps.heat_capacity_cp = node->DC_Cp0(xCH, P*1e05, T+273.15);
            tps.volume           = node->DC_V0 (xCH, P*1e05, T+273.15);
            out.writeThermoPropertiesSubstance( vSubst[i].symbol() + "_Gems", T, P, tps);

            compare(tps, result, vSubst[i].symbol().c_str(), T, P);

            P = 5000;
        }

        T +=25;
    } while (T <= 800);
    out.closeThermoPropertiesSubstanceFile();

    cout << "Bye World!" << endl;

    return 0;
}

