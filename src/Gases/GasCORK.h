#ifndef GASCORK
#define GASCORK

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

enum tp_codes {  /// codes for fluid subroutines in EoS models (see v_mod.h)
    CEM_OFF_ = 'N',
    CEM_GAS_ = 'G',
    CEM_H2O_ = 'V',
    CEM_CO2_ = 'C',
    CEM_CH4_ = 'M',
    CEM_N2_ = 'T',
    CEM_H2_ = 'H',
    CEM_O2_ = 'O',
    CEM_AR_ = 'A',
    CEM_PO_ = 'P',
    CEM_NP_ = 'Q'
};


namespace TCorrPT {

auto thermoPropertiesGasCORK(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASCORK

