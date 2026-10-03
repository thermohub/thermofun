#ifndef THERMOPROPERTIES_H
#define THERMOPROPERTIES_H

// TCorPT includes
#include "Common/ThermoProperty.hpp"

namespace ThermoFun {

/// Describe the thermodynamic state of a substance
template<class S>
struct ThermoPropertiesSubstanceT
{
    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    S gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    S helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    S internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    S enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/(mol*K))
    S entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    S volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol*K))
    S heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol*K))
    S heat_capacity_cv;
};


/// Describes the thermodynamic state of a reaction
template<class S>
struct ThermoPropertiesReactionT
{
    /// The natural logarithm of the equilibirum constant of the reaction
    S ln_equilibrium_constant;

    /// The logarithm of the equilibirum constant of the reaction in base 10
    S log_equilibrium_constant;

    /// The apparent standard molar Gibbs free energy @f$\Delta G_{f}^{\circ}@f$ of the species (in units of J/mol)
    S reaction_gibbs_energy;

    /// The apparent standard molar Helmholtz free energy @f$\Delta A_{f}^{\circ}@f$ of the species (in units of J/mol)
    S reaction_helmholtz_energy;

    /// The apparent standard molar internal energy @f$\Delta U_{f}^{\circ}@f$ of the species (in units of J/mol)
    S reaction_internal_energy;

    /// The apparent standard molar enthalpy @f$\Delta H_{f}^{\circ}@f$ of the species (in units of J/mol)
    S reaction_enthalpy;

    /// The standard molar entropy @f$ S^{\circ}@f$ of the species (in units of J/(mol*K))
    S reaction_entropy;

    /// The standard molar volume @f$ V^{\circ}@f$ of the species (in units of J/bar)
    S reaction_volume;

    /// The standard molar isobaric heat capacity @f$ C_{P}^{\circ}@f$ of the species (in units of J/(mol*K))
    S reaction_heat_capacity_cp;

    /// The standard molar isochoric heat capacity @f$ C_{V}^{\circ}@f$ of the species (in units of J/(mol*K))
    S reaction_heat_capacity_cv;
};

/// Describes the thermodynamic properties specific to a solvent
template<class S>
struct PropertiesSolventT
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
    S Alpha;

    /// first order derivative of alpha with T
    S dAldT;

    /// constant temperature compressibility (beta) (in units of (1/Pa)
    S Beta;

    /// alpha/beta ratio (in units of K/Pa)
    S Albe;

//    /// ideal gas Gibbs energy (in units of J/mol)
//    S gibbsIdealGas;

//    /// ideal gas entropy (in units of J/(mol K))
//    S entropyIdealGas;

//    /// ideal gas isobaric heat capacity (in units of J/(mol*K))
//    S cpIdealGas;

    /// The specific density of solvent (in units of kg/m3)
    S density;

    /// The first-order partial derivative of density with respect to temperature (in units of (kg/m3)/K)
    S densityT;

    /// The first-order partial derivative of density with respect to pressure (in units of (kg/m3)/Pa)
    S densityP;

    /// The second-order partial derivative of density with respect to temperature (in units of (kg/m3)/(K*K))
    S densityTT;

    /// The second-order partial derivative of density with respect to temperature and pressure (in units of (kg/m3)/(K*Pa))
    S densityTP;

    /// The second-order partial derivative of density with respect to pressure (in units of (kg/m3)/(Pa*Pa))
    S densityPP;

    /// The pressure of solvent (in units of Pa)
    S pressure;

    /// The first-order partial derivative of pressure with respect to temperature (in units of Pa/K)
    S pressureT;

    /// The first-order partial derivative of pressure with respect to density (in units of Pa/(kg/m3))
    S pressureD;

    /// The second-order partial derivative of pressure with respect to temperature (in units of Pa/(K*K))
    S pressureTT;

    /// The second-order partial derivative of pressure with respect to temperature and density (in units of Pa/(K*kg/m3))
    S pressureTD;

    /// The second-order partial derivative of pressure with respect to density (in units of Pa/((kg/m3)*(kg/m3)))
    S pressureDD;

};

/**
 * @brief The ElectroPropertiesSolvent struct holds the electro-chemical properties of a solvent
 */
template<class S>
struct ElectroPropertiesSolventT
{
    /// The dielectric constant of water
    S epsilon;

    /// The first-order partial derivative of the dielectric constant with respect to temperature
    S epsilonT;

    /// The first-order partial derivative of the dielectric constant with respect to pressure
    S epsilonP;

    /// The second-order partial derivative of the dielectric constant with respect to temperature
    S epsilonTT;

    /// The second-order partial derivative of the dielectric constant with respect to temperature and pressure
    S epsilonTP;

    /// The second-order partial derivative of the dielectric constant with respect to pressure
    S epsilonPP;

    /// The Born function \f$ Z\equiv-\frac{1}{\epsilon} \f$ (see Helgeson and Kirkham, 1974)
    S bornZ;

    /// The Born function \f$ Y\equiv\left[\frac{\partial Z}{\partial T}\right]_{P} \f$ in units of 1/K (see Helgeson and Kirkham, 1974)
    S bornY;

    /// The Born function \f$ Q\equiv\left[\frac{\partial Z}{\partial P}\right]_{T} \f$ in units of 1/Pa (see Helgeson and Kirkham, 1974)
    S bornQ;

    /// The Born function \f$ N\equiv\left[\frac{\partial Q}{\partial P}\right]_{T} \f$ in units of 1/Pa*Pa (see Helgeson and Kirkham, 1974)
    S bornN;

    /// The Born function \f$ U\equiv\left[\frac{\partial Q}{\partial T}\right]_{P} \f$ in units of 1/Pa*K (see Helgeson and Kirkham, 1974)
    S bornU;

    /// The Born function \f$ X\equiv\left[\frac{\partial Y}{\partial T}\right]_{P} \f$ in units of 1/K*K (see Helgeson and Kirkham, 1974)
    S bornX;
};

/**
 * @brief The ElectroPropertiesSubstance struct holds the electro-chemical properties of an solute
 */
struct ElectroPropertiesSubstance
{
    /// The effective electrostatic radius of the solute species at referente temperature 298.15 K and pressure 1 bar
    real reref;

    /// The effective electrostatic radius of the solute species
    real re;

    /// The Born coefficient of the solute species
    real w;

    /// The first-order partial derivative of the Born coefficient of the solute species with respect to temperature
    real wT;

    /// The first-order partial derivative of the Born coefficient of the solute species with respect to pressure
    real wP;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to temperature
    real wTT;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to temperature and pressure
    real wTP;

    /// The second-order partial derivative of the Born coefficient of the solute species with respect to pressure
    real wPP;
};

/// A type used to describe the function g of the HKF model and its partial temperature and pressure derivatives
struct FunctionG
{
    /// The function g at temperature T and pressure P
    real g;

    /// The first-order partial derivative of function g with respect to temperature
    real gT;

    /// The first-order partial derivative of function g with respect to pressure
    real gP;

    /// The second-order partial derivative of function g with respect to temperature
    real gTT;

    /// The second-order partial derivative of function g with respect to temperature and pressure
    real gTP;

    /// The second-order partial derivative of function g with respect to pressure
    real gPP;
};

/// The thermodynamic properties as results of the engine: value, derivatives with respect to T and P, error and status
using ThermoPropertiesSubstance = ThermoPropertiesSubstanceT<Reaktoro_::ThermoProperty>;
using ThermoPropertiesReaction  = ThermoPropertiesReactionT<Reaktoro_::ThermoProperty>;
using PropertiesSolvent         = PropertiesSolventT<Reaktoro_::ThermoProperty>;
using ElectroPropertiesSolvent  = ElectroPropertiesSolventT<Reaktoro_::ThermoProperty>;

/// The same properties as autodiff numbers, used inside the models while calculating
using ThermoPropertiesSubstanceAD = ThermoPropertiesSubstanceT<real>;
using ThermoPropertiesReactionAD  = ThermoPropertiesReactionT<real>;
using PropertiesSolventAD         = PropertiesSolventT<real>;
using ElectroPropertiesSolventAD  = ElectroPropertiesSolventT<real>;

// The properties as autodiff numbers in one pass (see Pass) and back as properties with the derivatives of both passes.
// The error and status are not set by these functions: they are propagated by the models.

/// The properties with the derivatives of the pass
inline auto lift(const Reaktoro_::Pass& pass, const ThermoPropertiesSubstance& x) -> ThermoPropertiesSubstanceAD
{
    ThermoPropertiesSubstanceAD r;
    r.gibbs_energy = pass(x.gibbs_energy);
    r.helmholtz_energy = pass(x.helmholtz_energy);
    r.internal_energy = pass(x.internal_energy);
    r.enthalpy = pass(x.enthalpy);
    r.entropy = pass(x.entropy);
    r.volume = pass(x.volume);
    r.heat_capacity_cp = pass(x.heat_capacity_cp);
    r.heat_capacity_cv = pass(x.heat_capacity_cv);
    return r;
}

/// The properties from the autodiff numbers calculated in the pass seeded with T and in the pass seeded with P
inline auto toProperties(const ThermoPropertiesSubstanceAD& wrtT, const ThermoPropertiesSubstanceAD& wrtP) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance r;
    r.gibbs_energy = Reaktoro_::toProperty(wrtT.gibbs_energy, wrtP.gibbs_energy);
    r.helmholtz_energy = Reaktoro_::toProperty(wrtT.helmholtz_energy, wrtP.helmholtz_energy);
    r.internal_energy = Reaktoro_::toProperty(wrtT.internal_energy, wrtP.internal_energy);
    r.enthalpy = Reaktoro_::toProperty(wrtT.enthalpy, wrtP.enthalpy);
    r.entropy = Reaktoro_::toProperty(wrtT.entropy, wrtP.entropy);
    r.volume = Reaktoro_::toProperty(wrtT.volume, wrtP.volume);
    r.heat_capacity_cp = Reaktoro_::toProperty(wrtT.heat_capacity_cp, wrtP.heat_capacity_cp);
    r.heat_capacity_cv = Reaktoro_::toProperty(wrtT.heat_capacity_cv, wrtP.heat_capacity_cv);
    return r;
}

/// The properties with the derivatives of the pass
inline auto lift(const Reaktoro_::Pass& pass, const ThermoPropertiesReaction& x) -> ThermoPropertiesReactionAD
{
    ThermoPropertiesReactionAD r;
    r.ln_equilibrium_constant = pass(x.ln_equilibrium_constant);
    r.log_equilibrium_constant = pass(x.log_equilibrium_constant);
    r.reaction_gibbs_energy = pass(x.reaction_gibbs_energy);
    r.reaction_helmholtz_energy = pass(x.reaction_helmholtz_energy);
    r.reaction_internal_energy = pass(x.reaction_internal_energy);
    r.reaction_enthalpy = pass(x.reaction_enthalpy);
    r.reaction_entropy = pass(x.reaction_entropy);
    r.reaction_volume = pass(x.reaction_volume);
    r.reaction_heat_capacity_cp = pass(x.reaction_heat_capacity_cp);
    r.reaction_heat_capacity_cv = pass(x.reaction_heat_capacity_cv);
    return r;
}

/// The properties from the autodiff numbers calculated in the pass seeded with T and in the pass seeded with P
inline auto toProperties(const ThermoPropertiesReactionAD& wrtT, const ThermoPropertiesReactionAD& wrtP) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction r;
    r.ln_equilibrium_constant = Reaktoro_::toProperty(wrtT.ln_equilibrium_constant, wrtP.ln_equilibrium_constant);
    r.log_equilibrium_constant = Reaktoro_::toProperty(wrtT.log_equilibrium_constant, wrtP.log_equilibrium_constant);
    r.reaction_gibbs_energy = Reaktoro_::toProperty(wrtT.reaction_gibbs_energy, wrtP.reaction_gibbs_energy);
    r.reaction_helmholtz_energy = Reaktoro_::toProperty(wrtT.reaction_helmholtz_energy, wrtP.reaction_helmholtz_energy);
    r.reaction_internal_energy = Reaktoro_::toProperty(wrtT.reaction_internal_energy, wrtP.reaction_internal_energy);
    r.reaction_enthalpy = Reaktoro_::toProperty(wrtT.reaction_enthalpy, wrtP.reaction_enthalpy);
    r.reaction_entropy = Reaktoro_::toProperty(wrtT.reaction_entropy, wrtP.reaction_entropy);
    r.reaction_volume = Reaktoro_::toProperty(wrtT.reaction_volume, wrtP.reaction_volume);
    r.reaction_heat_capacity_cp = Reaktoro_::toProperty(wrtT.reaction_heat_capacity_cp, wrtP.reaction_heat_capacity_cp);
    r.reaction_heat_capacity_cv = Reaktoro_::toProperty(wrtT.reaction_heat_capacity_cv, wrtP.reaction_heat_capacity_cv);
    return r;
}

/// The properties with the derivatives of the pass
inline auto lift(const Reaktoro_::Pass& pass, const PropertiesSolvent& x) -> PropertiesSolventAD
{
    PropertiesSolventAD r;
    r.Alpha = pass(x.Alpha);
    r.dAldT = pass(x.dAldT);
    r.Beta = pass(x.Beta);
    r.Albe = pass(x.Albe);
    r.density = pass(x.density);
    r.densityT = pass(x.densityT);
    r.densityP = pass(x.densityP);
    r.densityTT = pass(x.densityTT);
    r.densityTP = pass(x.densityTP);
    r.densityPP = pass(x.densityPP);
    r.pressure = pass(x.pressure);
    r.pressureT = pass(x.pressureT);
    r.pressureD = pass(x.pressureD);
    r.pressureTT = pass(x.pressureTT);
    r.pressureTD = pass(x.pressureTD);
    r.pressureDD = pass(x.pressureDD);
    r.speed_of_sound = x.speed_of_sound;
    r.dynamic_viscosity = x.dynamic_viscosity;
    r.thermal_conductivity = x.thermal_conductivity;
    r.surface_tension = x.surface_tension;
    r.Tdiff = x.Tdiff;
    r.Prndtl = x.Prndtl;
    r.Visck = x.Visck;
    return r;
}

/// The properties from the autodiff numbers calculated in the pass seeded with T and in the pass seeded with P
inline auto toProperties(const PropertiesSolventAD& wrtT, const PropertiesSolventAD& wrtP) -> PropertiesSolvent
{
    PropertiesSolvent r;
    r.Alpha = Reaktoro_::toProperty(wrtT.Alpha, wrtP.Alpha);
    r.dAldT = Reaktoro_::toProperty(wrtT.dAldT, wrtP.dAldT);
    r.Beta = Reaktoro_::toProperty(wrtT.Beta, wrtP.Beta);
    r.Albe = Reaktoro_::toProperty(wrtT.Albe, wrtP.Albe);
    r.density = Reaktoro_::toProperty(wrtT.density, wrtP.density);
    r.densityT = Reaktoro_::toProperty(wrtT.densityT, wrtP.densityT);
    r.densityP = Reaktoro_::toProperty(wrtT.densityP, wrtP.densityP);
    r.densityTT = Reaktoro_::toProperty(wrtT.densityTT, wrtP.densityTT);
    r.densityTP = Reaktoro_::toProperty(wrtT.densityTP, wrtP.densityTP);
    r.densityPP = Reaktoro_::toProperty(wrtT.densityPP, wrtP.densityPP);
    r.pressure = Reaktoro_::toProperty(wrtT.pressure, wrtP.pressure);
    r.pressureT = Reaktoro_::toProperty(wrtT.pressureT, wrtP.pressureT);
    r.pressureD = Reaktoro_::toProperty(wrtT.pressureD, wrtP.pressureD);
    r.pressureTT = Reaktoro_::toProperty(wrtT.pressureTT, wrtP.pressureTT);
    r.pressureTD = Reaktoro_::toProperty(wrtT.pressureTD, wrtP.pressureTD);
    r.pressureDD = Reaktoro_::toProperty(wrtT.pressureDD, wrtP.pressureDD);
    r.speed_of_sound = wrtT.speed_of_sound;
    r.dynamic_viscosity = wrtT.dynamic_viscosity;
    r.thermal_conductivity = wrtT.thermal_conductivity;
    r.surface_tension = wrtT.surface_tension;
    r.Tdiff = wrtT.Tdiff;
    r.Prndtl = wrtT.Prndtl;
    r.Visck = wrtT.Visck;
    return r;
}

/// The properties with the derivatives of the pass
inline auto lift(const Reaktoro_::Pass& pass, const ElectroPropertiesSolvent& x) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD r;
    r.epsilon = pass(x.epsilon);
    r.epsilonT = pass(x.epsilonT);
    r.epsilonP = pass(x.epsilonP);
    r.epsilonTT = pass(x.epsilonTT);
    r.epsilonTP = pass(x.epsilonTP);
    r.epsilonPP = pass(x.epsilonPP);
    r.bornZ = pass(x.bornZ);
    r.bornY = pass(x.bornY);
    r.bornQ = pass(x.bornQ);
    r.bornN = pass(x.bornN);
    r.bornU = pass(x.bornU);
    r.bornX = pass(x.bornX);
    return r;
}

/// The properties from the autodiff numbers calculated in the pass seeded with T and in the pass seeded with P
inline auto toProperties(const ElectroPropertiesSolventAD& wrtT, const ElectroPropertiesSolventAD& wrtP) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent r;
    r.epsilon = Reaktoro_::toProperty(wrtT.epsilon, wrtP.epsilon);
    r.epsilonT = Reaktoro_::toProperty(wrtT.epsilonT, wrtP.epsilonT);
    r.epsilonP = Reaktoro_::toProperty(wrtT.epsilonP, wrtP.epsilonP);
    r.epsilonTT = Reaktoro_::toProperty(wrtT.epsilonTT, wrtP.epsilonTT);
    r.epsilonTP = Reaktoro_::toProperty(wrtT.epsilonTP, wrtP.epsilonTP);
    r.epsilonPP = Reaktoro_::toProperty(wrtT.epsilonPP, wrtP.epsilonPP);
    r.bornZ = Reaktoro_::toProperty(wrtT.bornZ, wrtP.bornZ);
    r.bornY = Reaktoro_::toProperty(wrtT.bornY, wrtP.bornY);
    r.bornQ = Reaktoro_::toProperty(wrtT.bornQ, wrtP.bornQ);
    r.bornN = Reaktoro_::toProperty(wrtT.bornN, wrtP.bornN);
    r.bornU = Reaktoro_::toProperty(wrtT.bornU, wrtP.bornU);
    r.bornX = Reaktoro_::toProperty(wrtT.bornX, wrtP.bornX);
    return r;
}

/// Evaluate f(pass) in the autodiff pass seeded with temperature and in the pass seeded with pressure, and combine
/// the results in properties with the value and the derivatives with respect to T and P
template<typename F>
inline auto twoPass(double T, double P, F&& f)
{
    const auto wrtT = f(Reaktoro_::Pass(T, P, Reaktoro_::Wrt::T));
    const auto wrtP = f(Reaktoro_::Pass(T, P, Reaktoro_::Wrt::P));
    return toProperties(wrtT, wrtP);
}

} // namespace ThermoFun

#endif // THERMOPROPERTIES_H

