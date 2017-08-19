#ifndef UNITS_H
#define UNITS_H

namespace ThermoFun {

struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;
struct ElectroPropertiesSubstance;
struct FunctionG;


auto changeUnitPressure (ThermoPropertiesSubstance  tpSubst, double multiplier) -> ThermoPropertiesSubstance;
auto changeUnitPressure ( ThermoPropertiesReaction   tpReact, double multiplier) -> ThermoPropertiesReaction;
auto changeUnitPressure (PropertiesSolvent          pSolv, double multiplier) -> PropertiesSolvent;
auto changeUnitPressure ( ElectroPropertiesSolvent   epSolv,  double multiplier) -> ElectroPropertiesSolvent;
auto changeUnitPressure (ElectroPropertiesSubstance epSubst, double multiplier) -> ElectroPropertiesSubstance;
auto changeUnitPressure (FunctionG                  fG, double multiplier) -> FunctionG;

}

#endif // UNITS_H
