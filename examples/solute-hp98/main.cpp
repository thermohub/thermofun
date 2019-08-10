
#include <ThermoFun/ThermoFun.h>
using namespace ThermoFun;

int main()
{
    Database db;

    Substance water;
    water.setSymbol("water");
    water.setFormula("H2O");
    water.setSubstanceClass(SubstanceClass::type::AQSOLVENT);
    water.setAggregateState(AggregateState::type::AQUEOUS);
    water.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_WJNR);
    water.setMethod_T(MethodCorrT_Thrift::type::CTM_WWP);
    water.setReferenceP(1e5);
    water.setReferenceT(298.15);

    db.addSubstance(water);

    Substance OH_;

    OH_.setSymbol("OH-");
    OH_.setFormula("OH-");
    OH_.setSubstanceClass(SubstanceClass::type::AQSOLUTE);
    OH_.setAggregateState(AggregateState::type::AQUEOUS);
    OH_.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_HP98);
    OH_.setReferenceP(1e5);
    OH_.setReferenceT(298.15);

    ThermoParametersSubstance params;
    params.solute_holland_powell98_coeff = {0.0};
    OH_.setThermoParameters(params);

    ThermoPropertiesSubstance tpsr;
    tpsr.gibbs_energy = -157220;
    tpsr.enthalpy = -230020;
    tpsr.entropy = -10.71;
    tpsr.heat_capacity_cp = -137.2;
    tpsr.volume = -0.418;

    OH_.setThermoReferenceProperties(tpsr);

    db.addSubstance(OH_);

    ThermoEngine engine(db);
    engine.setSolventSymbol("water");

    double TK = 298.15;
    double pPa = 1e5; 

    auto tps = engine.thermoPropertiesSubstance(TK, pPa, "OH-");

    cout <<"G0 " << tps.gibbs_energy << endl;
    cout <<"H0 " << tps.enthalpy << endl;
    cout <<"S0 " << tps.entropy << endl;
    cout <<"V0 " << tps.volume << endl;
    cout <<"Cp0 " << tps.heat_capacity_cp << endl;

    //G --157220 // calc -157218
    //H	-230020 // calc -230018
    //S	-10.71 // -10.71
    //V	-0.418 // -0.418
    //cp(0)	-137.2 // -137.2


    return 0;
}
