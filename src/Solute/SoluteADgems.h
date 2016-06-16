#ifndef SOLUTEADGEMS
#define SOLUTEADGEMS

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

void Akinfiev_EOS_increments(Reaktoro::Temperature T, Reaktoro::Pressure P, Reaktoro::ThermoScalar Gig, Reaktoro::ThermoScalar Sig, Reaktoro::ThermoScalar CPig,
        Reaktoro::ThermoScalar Gw, Reaktoro::ThermoScalar Sw, Reaktoro::ThermoScalar CPw, Reaktoro::ThermoScalar rho, Reaktoro::ThermoScalar alp, Reaktoro::ThermoScalar bet, Reaktoro::ThermoScalar dalpT, vd ADparam,
        Reaktoro::ThermoScalar &Geos, Reaktoro::ThermoScalar &Veos, Reaktoro::ThermoScalar &Seos, Reaktoro::ThermoScalar &CPeos, Reaktoro::ThermoScalar &Heos );

/// Returns the thermodynamic properties of an aqueous solute using the Akinfiev and Diamond EOS
/// @ref Akinfiev and Diamond (2003)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param wtp water thermo properties
/// @param wigp water ideal gas properties
/// @param wp water solvent properties
auto thermoPropertiesAqSoluteAD(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance;

}

#endif // SOLUTEADGEMS

