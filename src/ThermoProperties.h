// TCorPT includes
#include "tcorrpt_global.h"
#include "Common/ThermoScalar.hpp"

namespace TCorrPT {

/// Describe the thermodynamic state of a substance
struct ThermoPropertiesSubstance
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    Reaktoro::ThermoScalar entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    Reaktoro::ThermoScalar volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro::ThermoScalar heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro::ThermoScalar heat_capacity_cv;
};


/// Describes the thermodynamic state of a reaction
struct ThermoPropertiesReaction
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar reaction_gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar reaction_helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar reaction_internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro::ThermoScalar reaction_enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    Reaktoro::ThermoScalar reaction_entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    Reaktoro::ThermoScalar reaction_volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro::ThermoScalar reaction_heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro::ThermoScalar reaction_heat_capacity_cv;
};

/// Describes the thermodynamic properties specific to a solvent
struct PropertiesSolvent
{
    double Speed,        /// speed of sound
       Alpha,            /// constant pressure expansion (alpha)
       Beta,             /// constant temperature compressibility (beta)
       Visc,             /// dynamic viscosity
       Tcond,            /// thermal conductivity
       Surten,           /// surface tension
       Tdiff,            /// not clear (currently not used)
       Prndtl,           /// Prandtl number (currently not used)
       Visck,            /// kinetic viscosity (currently not used)
       Albe,             /// alpha/beta ratio
       dAldT;            /// T derivative of isobaric expansion

    /// ideal gas Gibbs energy
    Reaktoro::ThermoScalar gibbsIdealGas;

    /// ideal gas entropy
    Reaktoro::ThermoScalar entropyIdealGas;

    /// ideal gas isobaric heat capacity
    Reaktoro::ThermoScalar cpIdealGas;

    /// The specific density of solvent (in units of kg/m3)
    Reaktoro::ThermoScalar density;

    /// The first-order partial derivative of density with respect to temperature (in units of (kg/m3)/K)
    Reaktoro::ThermoScalar densityT;

    /// The first-order partial derivative of density with respect to pressure (in units of (kg/m3)/Pa)
    Reaktoro::ThermoScalar densityP;

    /// The second-order partial derivative of density with respect to temperature (in units of (kg/m3)/(K*K))
    Reaktoro::ThermoScalar densityTT;

    /// The second-order partial derivative of density with respect to temperature and pressure (in units of (kg/m3)/(K*Pa))
    Reaktoro::ThermoScalar densityTP;

    /// The second-order partial derivative of density with respect to pressure (in units of (kg/m3)/(Pa*Pa))
    Reaktoro::ThermoScalar densityPP;

    /// The pressure of solvent (in units of Pa)
    Reaktoro::ThermoScalar pressure;

    /// The first-order partial derivative of pressure with respect to temperature (in units of Pa/K)
    Reaktoro::ThermoScalar pressureT;

    /// The first-order partial derivative of pressure with respect to density (in units of Pa/(kg/m3))
    Reaktoro::ThermoScalar pressureD;

    /// The second-order partial derivative of pressure with respect to temperature (in units of Pa/(K*K))
    Reaktoro::ThermoScalar pressureTT;

    /// The second-order partial derivative of pressure with respect to temperature and density (in units of Pa/(K*kg/m3))
    Reaktoro::ThermoScalar pressureTD;

    /// The second-order partial derivative of pressure with respect to density (in units of Pa/((kg/m3)*(kg/m3)))
    Reaktoro::ThermoScalar pressureDD;

};

struct ElectroPropertiesSolvent
{
    /// The dielectric constant of water
    Reaktoro::ThermoScalar epsilon;

    /// The first-order partial derivative of the dielectric constant with respect to temperature
    Reaktoro::ThermoScalar epsilonT;

    /// The first-order partial derivative of the dielectric constant with respect to pressure
    Reaktoro::ThermoScalar epsilonP;

    /// The second-order partial derivative of the dielectric constant with respect to temperature
    Reaktoro::ThermoScalar epsilonTT;

    /// The second-order partial derivative of the dielectric constant with respect to temperature and pressure
    Reaktoro::ThermoScalar epsilonTP;

    /// The second-order partial derivative of the dielectric constant with respect to pressure
    Reaktoro::ThermoScalar epsilonPP;

    /// The Born function \f$ Z\equiv-\frac{1}{\epsilon} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornZ;

    /// The Born function \f$ Y\equiv\left[\frac{\partial Z}{\partial T}\right]_{P} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornY;

    /// The Born function \f$ Q\equiv\left[\frac{\partial Z}{\partial P}\right]_{T} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornQ;

    /// The Born function \f$ N\equiv\left[\frac{\partial Q}{\partial P}\right]_{T} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornN;

    /// The Born function \f$ U\equiv\left[\frac{\partial Q}{\partial T}\right]_{P} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornU;

    /// The Born function \f$ X\equiv\left[\frac{\partial Y}{\partial T}\right]_{P} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro::ThermoScalar bornX;
};

} // namespace TCorrPT

