#ifndef THERMOPROPERTIES_H
#define THERMOPROPERTIES_H

// TCorPT includes
#include "tcorrpt_global.h"
#include "Common/ThermoScalar.hpp"

namespace TCorrPT {

struct ThermoVariables
{
    /// the temperature T (in units of C)
    Reaktoro_::Temperature temperature;

    /// the pressure P (in units of bar)
    Reaktoro_::Pressure pressure;
};

/// Describe the thermodynamic state of a substance
struct ThermoPropertiesSubstance
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    Reaktoro_::ThermoScalar entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    Reaktoro_::ThermoScalar volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro_::ThermoScalar heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro_::ThermoScalar heat_capacity_cv;
};


/// Describes the thermodynamic state of a reaction
struct ThermoPropertiesReaction
{
    /// The natural logarithm of the equilibirum constant of the reaction
    Reaktoro_::ThermoScalar ln_equilibrium_constant;

    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar reaction_gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar reaction_helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar reaction_internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    Reaktoro_::ThermoScalar reaction_enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/K)
    Reaktoro_::ThermoScalar reaction_entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    Reaktoro_::ThermoScalar reaction_volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro_::ThermoScalar reaction_heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol K))
    Reaktoro_::ThermoScalar reaction_heat_capacity_cv;
};

/// Describes the thermodynamic properties specific to a solvent
struct PropertiesSolvent
{
    /// speed of sound
    double speed_of_sound,
//       Alpha,            /// constant pressure expansion (alpha)
//       Beta,             /// constant temperature compressibility (beta)
    /// dynamic viscosity
       dynamic_viscosity,
    /// thermal conductivity
       thermal_conductivity,
    /// surface tension
       surface_tension,
    /// not clear (currently not used)
       Tdiff,
    /// Prandtl number (currently not used)
       Prndtl,
    /// kinetic viscosity (currently not used)
       Visck;
//       Albe,             /// alpha/beta ratio
//       dAldT;            /// T derivative of isobaric expansion
    /// constant pressure expansion (alpha) (in units of 1/K)
    Reaktoro_::ThermoScalar Alpha;

    /// first order derivative of alpha with T
    Reaktoro_::ThermoScalar dAldT;

    /// constant temperature compressibility (beta) (in units of (1/Pa)
    Reaktoro_::ThermoScalar Beta;

    /// alpha/beta ratio (in units of K/Pa)
    Reaktoro_::ThermoScalar Albe;

    /// ideal gas Gibbs energy
    Reaktoro_::ThermoScalar gibbsIdealGas;

    /// ideal gas entropy
    Reaktoro_::ThermoScalar entropyIdealGas;

    /// ideal gas isobaric heat capacity
    Reaktoro_::ThermoScalar cpIdealGas;

    /// The specific density of solvent (in units of kg/m3)
    Reaktoro_::ThermoScalar density;

    /// The first-order partial derivative of density with respect to temperature (in units of (kg/m3)/K)
    Reaktoro_::ThermoScalar densityT;

    /// The first-order partial derivative of density with respect to pressure (in units of (kg/m3)/Pa)
    Reaktoro_::ThermoScalar densityP;

    /// The second-order partial derivative of density with respect to temperature (in units of (kg/m3)/(K*K))
    Reaktoro_::ThermoScalar densityTT;

    /// The second-order partial derivative of density with respect to temperature and pressure (in units of (kg/m3)/(K*Pa))
    Reaktoro_::ThermoScalar densityTP;

    /// The second-order partial derivative of density with respect to pressure (in units of (kg/m3)/(Pa*Pa))
    Reaktoro_::ThermoScalar densityPP;

    /// The pressure of solvent (in units of Pa)
    Reaktoro_::ThermoScalar pressure;

    /// The first-order partial derivative of pressure with respect to temperature (in units of Pa/K)
    Reaktoro_::ThermoScalar pressureT;

    /// The first-order partial derivative of pressure with respect to density (in units of Pa/(kg/m3))
    Reaktoro_::ThermoScalar pressureD;

    /// The second-order partial derivative of pressure with respect to temperature (in units of Pa/(K*K))
    Reaktoro_::ThermoScalar pressureTT;

    /// The second-order partial derivative of pressure with respect to temperature and density (in units of Pa/(K*kg/m3))
    Reaktoro_::ThermoScalar pressureTD;

    /// The second-order partial derivative of pressure with respect to density (in units of Pa/((kg/m3)*(kg/m3)))
    Reaktoro_::ThermoScalar pressureDD;

};

/**
 * @brief The ElectroPropertiesSolvent struct holds the electro-chemical properties of a solvent
 */
struct ElectroPropertiesSolvent
{
    /// The dielectric constant of water
    Reaktoro_::ThermoScalar epsilon;

    /// The first-order partial derivative of the dielectric constant with respect to temperature
    Reaktoro_::ThermoScalar epsilonT;

    /// The first-order partial derivative of the dielectric constant with respect to pressure
    Reaktoro_::ThermoScalar epsilonP;

    /// The second-order partial derivative of the dielectric constant with respect to temperature
    Reaktoro_::ThermoScalar epsilonTT;

    /// The second-order partial derivative of the dielectric constant with respect to temperature and pressure
    Reaktoro_::ThermoScalar epsilonTP;

    /// The second-order partial derivative of the dielectric constant with respect to pressure
    Reaktoro_::ThermoScalar epsilonPP;

    /// The Born function \f$ Z\equiv-\frac{1}{\epsilon} \f$ (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornZ;

    /// The Born function \f$ Y\equiv\left[\frac{\partial Z}{\partial T}\right]_{P} \f$ in units of 1/K (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornY;

    /// The Born function \f$ Q\equiv\left[\frac{\partial Z}{\partial P}\right]_{T} \f$ in units of 1/Pa (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornQ;

    /// The Born function \f$ N\equiv\left[\frac{\partial Q}{\partial P}\right]_{T} \f$ in units of 1/Pa*Pa (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornN;

    /// The Born function \f$ U\equiv\left[\frac{\partial Q}{\partial T}\right]_{P} \f$ in units of 1/Pa*K (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornU;

    /// The Born function \f$ X\equiv\left[\frac{\partial Y}{\partial T}\right]_{P} \f$ in units of 1/K*K (see Helgeson and Kirkham, 1974)
    Reaktoro_::ThermoScalar bornX;
};

/**
 * @brief The ElectroPropertiesSubstance struct holds the electro-chemical properties of an solute
 */
struct ElectroPropertiesSubstance
{
    /// The effective electrostatic radius of the solute species at referente temperature 298.15 K and pressure 1 bar
    Reaktoro_::ThermoScalar reref;

    /// The effective electrostatic radius of the solute species
    Reaktoro_::ThermoScalar re;

    /// The Born coefficient of the solute species
    Reaktoro_::ThermoScalar w;

    /// The first-order partial derivative of the Born coefficient of the solute species with respect to temperature
    Reaktoro_::ThermoScalar wT;

    /// The first-order partial derivative of the Born coefficient of the solute species with respect to pressure
    Reaktoro_::ThermoScalar wP;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to temperature
    Reaktoro_::ThermoScalar wTT;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to temperature and pressure
    Reaktoro_::ThermoScalar wTP;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to pressure
    Reaktoro_::ThermoScalar wPP;
};

/// A type used to describe the function g of the HKF model and its partial temperature and pressure derivatives
struct FunctionG
{
    /// The function g at temperature T and pressure P
    Reaktoro_::ThermoScalar g;

    /// The first-order partial derivative of function g with respect to temperature
    Reaktoro_::ThermoScalar gT;

    /// The first-order partial derivative of function g with respect to pressure
    Reaktoro_::ThermoScalar gP;

    /// The second-order partial derivative of function g with respect to temperature
    Reaktoro_::ThermoScalar gTT;

    /// The second-order partial derivative of function g with respect to temperature and pressure
    Reaktoro_::ThermoScalar gTP;

    /// The second-order partial derivative of function g with respect to pressure
    Reaktoro_::ThermoScalar gPP;
};

} // namespace TCorrPT

#endif // THERMOPROPERTIES_H

