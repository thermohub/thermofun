#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <functional>
typedef std::vector<double> vd;
typedef std::vector<std::vector<double>> vvd;


namespace ThermoFun {

struct TPlimit {

    /// upper, lower temperature and pressure limits in degrees K
    double upperT, lowerT, upperP, lowerP;

    /// hard limit, calculation not executed beyond the limit; soft limit calculation executed
    bool isHard = false;
};

struct Method {

    /// Method name
    std::string name;

    /// Parameter name and coefficients
    std::unordered_map<std::string,std::vector<double>> parameter_coefficients;

    /// Temperature and pressure limit of the method
    TPlimit tplimit;
};

typedef std::vector<std::pair<std::string,Method>> listmethods;

/// A type for storing the parameters of the HKF equation of state for a aqueous species
//struct ParamsHKF
//{
//    /// The apparent standard molal Gibbs free energy of formation of the species from its elements (in units of cal/mol)
//    double Gf;

//    /// The apparent standard molal enthalpy of formation of the species from its elements (in units of cal/mol)
//    double Hf;

//    /// The standard molal entropy of the species at reference temperature and pressure (in units of cal/(mol*K))
//    double Sr;

//    /// The coefficient a1 of the HKF equation of state of the aqueous species (in units of cal/(mol*bar))
//    double a1;

//    /// The coefficient a2 of the HKF equation of state of the aqueous species (in units of cal/mol)
//    double a2;

//    /// The coefficient a3 of the HKF equation of state of the aqueous species (in units of (cal*K)/(mol*bar))
//    double a3;

//    /// The coefficient a4 of the HKF equation of state of the aqueous species (in units of (cal*K)/mol)
//    double a4;

//    /// The coefficient c1 of the HKF equation of state of the aqueous species (in units of cal/(mol*K))
//    double c1;

//    /// The coefficient c2 of the HKF equation of state of the aqueous species (in units of (cal*K)/mol)
//    double c2;

//    /// The conventional Born coefficient of the aqueous species at reference temperature 298.15 K and pressure 1 bar (in units of cal/mol)
//    double wref;

//    double Tmax, Pmax;
//};


/// Describes the thermodynamic parameters of a substance used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersSubstance
{
    /// Isothermal compressibility (for condensed substances)
    double isothermal_compresibility = 0.0;

    /// Isobaric expansivity (for condensed substances)
    double isobaric_expansivity = 0.0;

    /// Lower and upper T limits for Cp=f(T) equation
    vvd temperature_intervals;

    /// Lower and upper P limits for pressure correction
    vvd pressure_intervals;

    /// Coeffs of Cp=f(T) (J,mole,K), one column per equation
    vvd Cp_coeff;

    /// Empirical coefficients for nonelectrolyte aqueous solutes in Akinfiev etc.
    vd Cp_nonElectrolyte_coeff; // vvd Cp_nonElectrolyte_coeff;

    /// Column: TCf- at Pr; DltS,DltH,DltV; dT/dP of phase transitions
    vvd phase_transition_prop;

    /// Properties of phase transition (Berman): Tr; Tft; tilt; l1,l2 (reserved)
    vvd phase_transition_prop_Berman;

    // HP Landau
    vd m_landau_phase_trans_props;

//    ParamsHKF HKF_param;

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

    /// coefficients for Holland and Powell 1998 aq solute model
    vd solute_holland_powell98_coeff; //

    /// Optional uncertainties (standard errors) of the coefficients, by the key of the coefficients in the database
    /// record (for example "eos_hkf_coeffs"); one row per coefficient vector (the rows of Cp_coeff and
    /// phase_transition_prop are the equations / intervals). A missing row or entry means no uncertainty.
    std::map<std::string, vvd> coefficient_errors;
};

/// Describes the thermodynamic parameters of a reaction used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersReaction
{
    /// Lower and upper T limits for Cp=f(T) equation
    vvd temperature_intervals;

    /// Lower and upper P limits for pressure correction
    vvd pressure_intervals;

    /// Reaction logK as a function of T coefficients
    vd reaction_logK_fT_coeff;

    /// Reaction logK at T and P points for interpolation
    vd logK_TP_array;

    /// Reaction heat capacity as a function of T coefficients
    vd reaction_Cp_fT_coeff;

    /// Reaction volume as a function of T coefficients
    vd reaction_V_fT_coeff;

    /// Coefficients of Ryzhenko-Bryzgalin model
    vd reaction_RB_coeff;

    /// Coefficients of Frantz & Marshall density model
    vd reaction_FM_coeff;

    /// Coefficients of Dolejs and Maning 2010 density model
    vd reaction_DM10_coeff;

    /// Optional uncertainties (standard errors) of the coefficients, by the key of the coefficients in the database
    /// record (for example "logk_ft_coeffs"). A missing row or entry means no uncertainty.
    std::map<std::string, vvd> coefficient_errors;
};

/// Describes the thermodynamic parameters of a solvent used in specific models to calculate
/// its thermodynamic properties at a given temperature and pressure
struct ThermoParametersSolvent
{ };

/// A coefficient of a model with an uncertainty: the key of the coefficients in the database record, a pointer to its value
/// and its standard error
struct UncertainCoefficient
{
    std::string key;
    double* value;
    double error;
};

namespace detail {

inline auto collectCoefficients(std::vector<UncertainCoefficient>& out, const std::map<std::string, vvd>& errors,
                                const std::string& key, vd& values, size_t row) -> void
{
    auto it = errors.find(key);
    if (it == errors.end() || row >= it->second.size()) return;
    const auto& e = it->second[row];
    for (size_t i = 0; i < values.size() && i < e.size(); ++i)
        if (e[i] > 0.0) out.push_back({key + "[" + std::to_string(row) + "][" + std::to_string(i) + "]", &values[i], e[i]});
}

inline auto collectCoefficients(std::vector<UncertainCoefficient>& out, const std::map<std::string, vvd>& errors,
                                const std::string& key, vvd& values) -> void
{
    for (size_t r = 0; r < values.size(); ++r) collectCoefficients(out, errors, key, values[r], r);
}

} // namespace detail

/// The coefficients of the parameters that have an uncertainty (the pointers refer to the given parameters)
inline auto uncertainCoefficients(ThermoParametersSubstance& p) -> std::vector<UncertainCoefficient>
{
    std::vector<UncertainCoefficient> out;
    const auto& e = p.coefficient_errors;
    detail::collectCoefficients(out, e, "eos_akinfiev_diamond_coeffs", p.Cp_nonElectrolyte_coeff, 0);
    detail::collectCoefficients(out, e, "eos_gas_crit_props", p.critical_parameters, 0);
    detail::collectCoefficients(out, e, "eos_hkf_coeffs", p.HKF_parameters, 0);
    detail::collectCoefficients(out, e, "m_heat_capacity_ft_coeffs", p.Cp_coeff);
    detail::collectCoefficients(out, e, "m_phase_trans_props", p.phase_transition_prop);
    detail::collectCoefficients(out, e, "m_landau_phase_trans_props", p.m_landau_phase_trans_props, 0);
    detail::collectCoefficients(out, e, "solute_holland_powell98_coeff", p.solute_holland_powell98_coeff, 0);
    return out;
}

inline auto uncertainCoefficients(ThermoParametersReaction& p) -> std::vector<UncertainCoefficient>
{
    std::vector<UncertainCoefficient> out;
    const auto& e = p.coefficient_errors;
    detail::collectCoefficients(out, e, "logk_ft_coeffs", p.reaction_logK_fT_coeff, 0);
    detail::collectCoefficients(out, e, "dr_heat_capacity_ft_coeffs", p.reaction_Cp_fT_coeff, 0);
    detail::collectCoefficients(out, e, "dr_volume_fpt_coeffs", p.reaction_V_fT_coeff, 0);
    detail::collectCoefficients(out, e, "dr_ryzhenko_coeffs", p.reaction_RB_coeff, 0);
    detail::collectCoefficients(out, e, "dr_marshall_franck_coeffs", p.reaction_FM_coeff, 0);
    detail::collectCoefficients(out, e, "dr_dolejs_manning10_coeffs", p.reaction_DM10_coeff, 0);
    return out;
}

} // namespace ThermoFun

#endif // PARAMETERS_H
