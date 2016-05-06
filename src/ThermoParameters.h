typedef std::vector<double> vd;
typedef std::vector<std::vector<double> > vvd;

namespace TCorrPT {

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
    vd HKF_parameters;

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
