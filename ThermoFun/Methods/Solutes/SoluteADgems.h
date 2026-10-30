#ifndef SOLUTEADGEMS
#define SOLUTEADGEMS
#include <ThermoFun/Common/ThermoProperties.h>
#include <string>

#include <vector>
#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

void Akinfiev_EOS_increments(real T, real P, real Gig, real Sig, real CPig,
        real Gw, real Sw, real CPw, real rho, real alp, real bet, real dalpT, const std::vector<real>& ADparam,
        real &Geos, real &Veos, real &Seos, real &CPeos, real &Heos );

/// Returns the thermodynamic properties of an aqueous solute using the Akinfiev and Diamond EOS
/// @ref Akinfiev and Diamond (2003)
/// @param T temparature (K)
/// @param P pressure (bar)
/// @param parameters the parameters of the model
/// @param symbol of the species, for the messages
/// @param wtp water thermo properties
/// @param wigp water ideal gas properties
/// @param wp water solvent properties
auto thermoPropertiesAqSoluteAD(real T, real P, const std::vector<real>& parameters, const std::string& symbol, ThermoPropertiesSubstanceAD tps, const ThermoPropertiesSubstanceAD& wtp,
                                const ThermoPropertiesSubstanceAD&wigp, const PropertiesSolventAD&wp, const ThermoPropertiesSubstanceAD&wtpr, const ThermoPropertiesSubstanceAD&wigpr, const PropertiesSolventAD&wpr) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEADGEMS

