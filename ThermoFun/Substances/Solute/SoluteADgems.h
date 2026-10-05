#ifndef SOLUTEADGEMS
#define SOLUTEADGEMS
#include <ThermoFun/ThermoProperties.h>

#include <vector>
#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

class Substance;

void Akinfiev_EOS_increments(real T, real P, real Gig, real Sig, real CPig,
        real Gw, real Sw, real CPw, real rho, real alp, real bet, real dalpT, std::vector<double> ADparam,
        real &Geos, real &Veos, real &Seos, real &CPeos, real &Heos );

/// Returns the thermodynamic properties of an aqueous solute using the Akinfiev and Diamond EOS
/// @ref Akinfiev and Diamond (2003)
/// @param T temparature (K)
/// @param P pressure (bar)
/// @param species aqueous species instance
/// @param wtp water thermo properties
/// @param wigp water ideal gas properties
/// @param wp water solvent properties
auto thermoPropertiesAqSoluteAD(real T, real P, Substance species, ThermoPropertiesSubstanceAD tps, const ThermoPropertiesSubstanceAD& wtp,
                                const ThermoPropertiesSubstanceAD&wigp, const PropertiesSolventAD&wp, const ThermoPropertiesSubstanceAD&wtpr, const ThermoPropertiesSubstanceAD&wigpr, const PropertiesSolventAD&wpr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEADGEMS

