#ifndef SOLUTEHKFGEMS
#define SOLUTEHKFGEMS

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {


//-------------------------------------------------------------------------
// Calculation  t/d parametres for water solution (Res to TPWORK)
// (Using equations given  by  Tanger  and  Helgeson (1988), Shock et al.(1991),
// and  Johnson et al. (1991)).
//auto calc_thkf( AQSREF& arf, double P, double T, double Dw, double betaw,
//                        double alphaw, double daldTw, double Z, double Q,
//                        double Y, double X, int geqn) -> void;

auto thermoPropertiesAqSoluteHKFgems(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes) -> ThermoPropertiesSubstance;

auto gShok2(Reaktoro::Temperature T, Reaktoro::Pressure P, const PropertiesSolvent &wts ) -> FunctionG;

auto omeg92(FunctionG g, Substance species) -> ElectroPropertiesSubstance;

}


#endif // SOLUTEHKFGEMS

