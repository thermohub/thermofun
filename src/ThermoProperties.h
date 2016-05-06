// TCorPT includes
#include "tcorrpt_global.h"
//#include "Common/ThermoScalar.hpp"

namespace TCorrPT {

/// Describe the thermodynamic state of a substance
struct ThermoPropertiesSubstance
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    double gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    double helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    double internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    double enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    double entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of m3/mol)
    double volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    double heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    double heat_capacity_cv;
};


/// Describes the thermodynamic state of a reaction
struct ThermoPropertiesReaction
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    double reaction_gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    double reaction_helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    double reaction_internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    double reaction_enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    double reaction_entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of m3/mol)
    double reaction_volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    double reaction_heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    double reaction_heat_capacity_cv;
};

/// Describes the thermodynamic properties specific to a solvent
struct ThermoPropertiesSolvent
{

    vd Speed[2],            /// speed of sound
       Alpha[2],            /// constant pressure expansion (alpha)
       Beta[2],             /// constant temperature compressibility (beta)
       Diel[2],             /// dielectrical constant
       Visc[2],             /// dynamic viscosity
       Tcond[2],            /// thermal conductivity
       Surten[2],           /// surface tension
       Tdiff[2],            /// not clear (currently not used)
       Prndtl[2],           /// Prandtl number (currently not used)
       Visck[2],            /// kinetic viscosity (currently not used)
       Albe[2],             /// alpha/beta ratio
       ZBorn[2],            /// Born function Z
       YBorn[2],            /// Born function Y
       QBorn[2],            /// Born function Q
       dAldT[2],            /// T derivative of isobaric expansion
       XBorn[2],            /// Born function X
       GidealGas[2],        /// ideal gas Gibbs energy
       SidealGas[2],        /// ideal gas entropy
       CpidealGas[2];       /// ideal gas isobaric heat capacity
};

} // namespace TCorrPT
