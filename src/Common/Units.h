#ifndef UNITS_H
#define UNITS_H

namespace ThermoFun {

struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;
struct ElectroPropertiesSubstance;
struct FunctionG;


auto changeUnitPressure ( ThermoPropertiesSubstance  tpSubst, double &P, double multiplier) -> ThermoPropertiesSubstance;
auto changeUnitPressure ( ThermoPropertiesReaction   tpReact, double &P, double multiplier) -> ThermoPropertiesReaction;
auto changeUnitPressure ( PropertiesSolvent          pSolv,   double &P, double multiplier) -> PropertiesSolvent;
auto changeUnitPressure ( ElectroPropertiesSolvent   epSolv,  double &P, double multiplier) -> ElectroPropertiesSolvent;
auto changeUnitPressure ( ElectroPropertiesSubstance epSubst, double &P, double multiplier) -> ElectroPropertiesSubstance;
auto changeUnitPressure (FunctionG                  fG,      double &P, double multiplier) -> FunctionG;

}

#endif // UNITS_H
