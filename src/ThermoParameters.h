typedef std::vector<double> vd;
typedef std::vector<std::vector<double> > vvd;

namespace TCorrPT {

/// A type for storing the parameters of the HKF equation of state for a aqueous species
struct ParamsHKF
{
    /// The apparent standard molal Gibbs free energy of formation of the species from its elements (in units of cal/mol)
    double Gf;

    /// The apparent standard molal enthalpy of formation of the species from its elements (in units of cal/mol)
    double Hf;

    /// The standard molal entropy of the species at reference temperature and pressure (in units of cal/(mol*K))
    double Sr;

    /// The coefficient a1 of the HKF equation of state of the aqueous species (in units of cal/(mol*bar))
    double a1;

    /// The coefficient a2 of the HKF equation of state of the aqueous species (in units of cal/mol)
    double a2;

    /// The coefficient a3 of the HKF equation of state of the aqueous species (in units of (cal*K)/(mol*bar))
    double a3;

    /// The coefficient a4 of the HKF equation of state of the aqueous species (in units of (cal*K)/mol)
    double a4;

    /// The coefficient c1 of the HKF equation of state of the aqueous species (in units of cal/(mol*K))
    double c1;

    /// The coefficient c2 of the HKF equation of state of the aqueous species (in units of (cal*K)/mol)
    double c2;

    /// The conventional Born coefficient of the aqueous species at reference temperature 298.15 K and pressure 1 bar (in units of cal/mol)
    double wref;

    double Tmax, Pmax;
};


/// Describes the thermodynamic parameters of a substance used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersSubstance
{
    /// Isothermal compressibility (for condensed substances)
    double isothermal_compresibility;
    /// Isobaric expansivity (for condensed substances)
    double isobaric_expansivity;

    /// Lower and upper T limits for Cp=f(T) equation
    vvd temperature_intervals;

    /// Lower and upper P limits for pressure correction
    vvd pressure_intervals;

    /// Coeffs of Cp=f(T) (J,mole,K), one column per equation
    vvd Cp_coeff;

    /// Empirical coefficients for nonelectrolyte aqueous solutes in Akinfiev etc.
    vvd Cp_nonElectrolyte_coeff;

    /// Column: TCf- at Pr; DltS,DltH,DltV; dT/dP of phase transitions
    vvd phase_transition_prop;

    /// Properties of phase transition (Berman): Tr; Tft; tilt; l1,l2 (reserved)
    vvd phase_transition_prop_Berman;

    /// Empirical coefficients of HKF EOS (a1, a2, a3, a4, c1, c2, w0)
    ParamsHKF HKF_parameters;

    /// Coefficients of mV=f(T,P)
    vd volume_coeff;

    /// Critical parameters (for FGL)?
    vd critical_parameters;

    /// Coeffs of V(T,P) Birch-Murnaghan 1947 Gottschalk
    vd volume_BirchM_coeff;

    /// Array of empirical EOS coefficients (CG EOS: MAXCGPARAM = 13)
    vd empirical_coeff;
};

/// Describes the thermodynamic parameters of a reaction used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersReaction
{


};

/// Describes the thermodynamic parameters of a solvent used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersSolvent
{


};


} // namespace TCorrPT
