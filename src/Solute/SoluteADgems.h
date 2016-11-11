#ifndef SOLUTEADGEMS
#define SOLUTEADGEMS

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

void Akinfiev_EOS_increments(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Reaktoro_::ThermoScalar Gig, Reaktoro_::ThermoScalar Sig, Reaktoro_::ThermoScalar CPig,
        Reaktoro_::ThermoScalar Gw, Reaktoro_::ThermoScalar Sw, Reaktoro_::ThermoScalar CPw, Reaktoro_::ThermoScalar rho, Reaktoro_::ThermoScalar alp, Reaktoro_::ThermoScalar bet, Reaktoro_::ThermoScalar dalpT, vd ADparam,
        Reaktoro_::ThermoScalar &Geos, Reaktoro_::ThermoScalar &Veos, Reaktoro_::ThermoScalar &Seos, Reaktoro_::ThermoScalar &CPeos, Reaktoro_::ThermoScalar &Heos );

/// Returns the thermodynamic properties of an aqueous solute using the Akinfiev and Diamond EOS
/// @ref Akinfiev and Diamond (2003)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param wtp water thermo properties
/// @param wigp water ideal gas properties
/// @param wp water solvent properties
auto thermoPropertiesAqSoluteAD(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance species, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance;

}

#endif // SOLUTEADGEMS

