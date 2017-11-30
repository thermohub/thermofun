#include "ThermoFun.h"
#include "bsonio/io_settings.h"
#include "DBClient/DatabaseClient.h"
#include "DBClient/ReactionData.h"
#include "DBClient/AbstractData.h"
#include "DBClient/SubstanceData.h"

using namespace std;
using namespace ThermoFun;

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;

    bsonio::BsonioSettings::settingsFileName = "./Resources/ThermoFun.json";
    DatabaseClient dbc_;

//    DatabaseClient dbc_("./Resources/ThermoFun.json");

//    auto t = dbc_.substData().getJsonBsonRecord("597b4bc8b29df90f0000002f:").first;
//    auto u = dbc_.reactData().getJsonBsonRecord("597b4bc8b29df90f0000002f:").first;

    Database db = dbc_.thermoFunDatabase(19);
    Database db2_ = dbc_.thermoFunDatabase(19);

    auto tdblist_ = dbc_.sourcetdbNamesIndexes();
    auto ellist_ = dbc_.availableElements(19);

//    auto rcd = dbc_.reactData();

    auto rcd = dbc_.reactData();

    auto loadedReacData = rcd.loadRecordsValues("{ \"_label\" : \"reaction\"}", 19, dbc_.availableElementsKey(19) );


//    for (auto e : ellist_)
//        auto symbol = e.symbol();

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

    std::map<Element, double> elem = dbc_.parseSubstanceFormula("FeHSiO3+2");

    cout << "Bye World!" << endl;
    return 0;
}

