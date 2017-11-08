#include "ThermoFun.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    DBClient dbc("./Resources/ThermoFun.ini");
    DatabaseClient dbc_("./Resources/ThermoFun.ini");

    auto ndx = dbc_.getSourcetdbIndexes();

    auto names = dbc_.sourcetdbNamesIndexes(ndx);

    Database db_ = dbc_.getDatabase(19);


    auto ellist_ = dbc_.availableElements(19);

    auto rcd = dbc_.reactData();

    auto loadedReacData = rcd.loadRecordsValues("{ \"_label\" : \"reaction\"}", 19, dbc_.availableElementsList_(19) );


//    for (auto e : ellist_)
//        auto symbol = e.symbol();



    auto tdblist = dbc.getSourcetdbList();

    auto ellist = dbc.makeAvailableElementsList(19);

    Database db = dbc.getDatabase(20);

    Substance water;
    water.setName("water");
    water.setSymbol("H2O@_");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);

    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_WJNR);

    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WAT);

    db.addSubstance(water);

    Thermo th(db/*DBClient("./Resources/ThermoFun.ini").getDatabase(15)*/);

    th.setSolventSymbol("H2O@_");

    double T = 650;
    double P = 2000;

    auto qtz = th.thermoPropertiesSubstance(T, P, "Quartz");

    auto watP = th.propertiesSolvent(T, P, "H2O@_");

    auto wat = th.thermoPropertiesSubstance(T, P, "H2O@_");

    auto ca = th.thermoPropertiesSubstance(T, P, "Ca+2");

    auto co2 = th.thermoPropertiesSubstance(T, P, "CO2@");

    ThermoPropertiesSubstance MgSi, CaSi, FeHSi, RaC, RaS, SiO, CaSi_FM, SiOaq;

//    for (uint i = 0; i <150000; i++)
//    {
//        SiOaq = th.thermoPropertiesSubstance(T, P, "SiO2@");
//    }

//    for (uint i = 0; i <150000; i++)
//    {
////        SiOaq = th.thermoPropertiesSubstanceF(T, P, "SiO2@");
//    }

//    for (uint i = 0; i <150000; i++)
//    {
//        SiOaq = th.thermoPropertiesSubstance(T, P, "SiO2@");
//    }

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

