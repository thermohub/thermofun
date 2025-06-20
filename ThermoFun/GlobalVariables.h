#ifndef ThermoFun_GLOBAL_H
#define ThermoFun_GLOBAL_H

#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <unordered_map>

#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#define OutputSTEAM_CONVENTION

///@cond INTERNAL

namespace ThermoFun {

void update_loggers( bool use_cout, const std::string& logfile_name, size_t log_level);
void clear_loggers( const std::string& logfile_name);

typedef std::vector<double> vd;
typedef std::vector<std::vector<double> > vvd;

static const double R_CONSTANT = 8.31451,
                    NA_CONSTANT = 6.0221367e23,
                    F_CONSTANT = 96485.309,
                    e_CONSTANT = 1.60217733e-19,
                    k_CONSTANT = 1.380658e-23,
// Conversion factors
                    cal_to_J = 4.184,
                    C_to_K = 273.15,
                    K_to_C =-273.15,
                    lg_to_ln = 2.302585093,
                    ln_to_lg = 0.434294481,
                    H2O_mol_to_kg = 55.50837344,
                    Min_phys_amount = 1.66e-24,
                    cm3_mol_to_J_bar = 0.1,
                    bar_to_Pa = 1e05,

                    TEMPER_PREC     = 0.5,
                    PRESSURE_PREC   = 0.1;

/// The molar mass of water in units of g/mol
static const double H2OMolarMass = 18.015268;

struct WaterTripleProperties {
    double Str; // J/(mol*K)
    double Gtr; // J/mol
    double Htr; // J/mol
    double Utr; // J/mol
    double Atr; // J/mol
};

// Create the map with source labels
static const std::unordered_map<std::string, WaterTripleProperties> waterTripleData = {
    // Auxiliary data from Helgeson and Kirkham (1974), on page 1098
    { "Helgeson_Kirkham_1974", {
        15.1320 * cal_to_J,
       -56290.0 * cal_to_J,
       -68767.0 * cal_to_J,
       -67887.0 * cal_to_J,
       -55415.0 * cal_to_J
    }},
    { "NEA_HGK", {
        15.1386 * cal_to_J,
       -56279.9 * cal_to_J,
       -68766.44 * cal_to_J,
       -67921.9 * cal_to_J,
       -55434.49 * cal_to_J
    }},
    { "NEA_IAPWS", {
        15.1374 * cal_to_J,
       -56279.9 * cal_to_J,
       -68766.76 * cal_to_J,
       -67922.22 * cal_to_J,
       -55434.49 * cal_to_J
    }}
    // Add more sources as needed
};

enum SubstanceTPMethodType {
    cp_ft_equation = 0,
    cp_ft_equation_saxena86 = 1,
    solute_hkf88_gems = 2,
    solute_hkf88_reaktoro = 3,
    solute_aknifiev_diamond03 = 4,
    landau_holland_powell98 = 5,
    landau_berman88 = 6,
    general_equation_of_state = 7,
    fug_critical_param = 8,
    fluid_prsv = 9,
    fluid_churakov_gottschalk = 10,
    fluid_soave_redlich_kwong = 11,
    fluid_sterner_pitzer = 12,
    fluid_peng_robinson78 = 13,
    fluid_comp_redlich_kwong_hp91 = 14,
    fluid_generic = 15,
    fluid_H2O = 16,
    fluid_CO2 = 17,
    fluid_CH4 = 18,
    fluid_N2 = 19,
    fluid_H2 = 20,
    fluid_O2 = 21,
    fluid_Ar = 22,
    fluid_polar = 23,
    fluid_nonpolar = 24,
    water_diel_jnort91_reaktoro = 25,
    water_diel_jnort91_gems = 26,
    water_diel_sverj14 = 27,
    water_diel_fern97 = 28,
    water_eos_hgk84_lvs83_gems = 29,
    water_eos_iapws95_gems = 30,
    water_eos_hgk84_reaktoro = 31,
    water_eos_iapws95_reaktoro = 32,
    water_pvt_zhang_duan05 = 33,
    mv_constant = 34,
    mv_equation_dorogokupets88 = 35,
    mv_equation_berman88 = 36,
    mv_eos_birch_murnaghan_gott97 = 37,
    mv_eos_murnaghan_hp98 = 38,
    mv_eos_tait_hp11 = 39,
    mv_pvnrt = 40,
    solute_holland_powell98 = 41,
    solute_anderson91 = 42,
    standard_entropy_cp_integration = 43
};

enum ReactionTPMethodType {
    logk_fpt_function = 0,
    adsor_ion_exchange = 1,
    iso_compounds_grichuk88 = 2,
    logk_nordstrom_munoz88 = 3,
    logk_1_term_extrap0 = 4,
    logk_1_term_extrap1 = 5,
    logk_2_term_extrap = 6,
    logk_3_term_extrap = 7,
    logk_lagrange_interp = 8,
    logk_marshall_frank78 = 9,
    solute_eos_ryzhenko_gems = 10,
    dr_heat_capacity_ft = 11,
    dr_volume_fpt = 12,
    dr_volume_constant = 13,
    logk_dolejs_manning10 = 14
};

const std::map<const unsigned int, const std::string> enum_method_substance = {
{0,"cp_ft_equation"},
{1,"cp_ft_equation_saxena86"},
{2,"solute_hkf88_gems"},
{3,"solute_hkf88_reaktoro"},
{4,"solute_aknifiev_diamond03"},
{5,"landau_holland_powell98"},
{6,"landau_berman88"},
//{7,"general_equation_of_state"},
{8,"fug_critical_param"},
{9,"fluid_prsv"},
{10,"fluid_churakov_gottschalk"},
{11,"fluid_soave_redlich_kwong"},
{12,"fluid_sterner_pitzer"},
{13,"fluid_peng_robinson78"},
{14,"fluid_comp_redlich_kwong_hp91"},
//{15,"fluid_generic"},
//{16,"fluid_H2O"},
//{17,"fluid_CO2"},
//{18,"fluid_CH4"},
//{19,"fluid_N2"},
//{20,"fluid_H2"},
//{21,"fluid_O2"},
//{22,"fluid_Ar"},
//{23,"fluid_polar"},
//{24,"fluid_nonpolar"},
{25,"water_diel_jnort91_reaktoro"},
{26,"water_diel_jnort91_gems"},
{27,"water_diel_sverj14"},
{28,"water_diel_fern97"},
{29,"water_eos_hgk84_lvs83_gems"},
{30,"water_eos_iapws95_gems"},
{31,"water_eos_hgk84_reaktoro"},
{32,"water_eos_iapws95_reaktoro"},
{33,"water_pvt_zhang_duan05"},
{34,"mv_constant"},
{35,"mv_equation_dorogokupets88"},
{36,"mv_equation_berman88"},
{37,"mv_eos_birch_murnaghan_gott97"},
{38,"mv_eos_murnaghan_hp98"},
{39,"mv_eos_tait_hp11"},
{40,"mv_pvnrt"},
{41,"solute_holland_powell98"},
{42,"solute_anderson91"},
{43,"standard_entropy_cp_integration"}
};

const std::map<const unsigned int, const std::string> enum_method_reaction = {
{0,"logk_fpt_function"},
//{1,"adsor_ion_exchange"},
//{2,"iso_compounds_grichuk88"},
{3,"logk_nordstrom_munoz88"},
{4,"logk_1_term_extrap0"},
{5,"logk_1_term_extrap1"},
{6,"logk_2_term_extrap"},
{7,"logk_3_term_extrap"},
{8,"logk_lagrange_interp"},
{9,"logk_marshall_frank78"},
{10,"solute_eos_ryzhenko_gems"},
{11,"dr_heat_capacity_ft"},
{12,"dr_volume_fpt"},
{13,"dr_volume_constant"},
{14,"logk_dolejs_manning10"}
};

const std::map<const ReactionTPMethodType, const std::vector<std::string>> reaction_method_parameters = {
{ReactionTPMethodType::logk_fpt_function, {"logk_ft_coeffs"}},
{ReactionTPMethodType::adsor_ion_exchange, {""}},
{ReactionTPMethodType::iso_compounds_grichuk88, {""}},
{ReactionTPMethodType::logk_nordstrom_munoz88, {"logk_ft_coeffs"}},
{ReactionTPMethodType::logk_1_term_extrap0, {"logk_ft_coeffs"}},
{ReactionTPMethodType::logk_1_term_extrap1, {"logk_ft_coeffs"}},
{ReactionTPMethodType::logk_2_term_extrap, {"logk_ft_coeffs"}},
{ReactionTPMethodType::logk_3_term_extrap, {"logk_ft_coeffs"}},
{ReactionTPMethodType::logk_lagrange_interp, {""}},
{ReactionTPMethodType::logk_marshall_frank78, {"dr_marshall_franck_coeffs"}},
{ReactionTPMethodType::solute_eos_ryzhenko_gems, {"dr_ryzhenko_coeffs"}},
{ReactionTPMethodType::dr_heat_capacity_ft, {"dr_heat_capacity_ft_coeffs"}},
{ReactionTPMethodType::dr_volume_fpt, {"dr_volume_fpt_coeffs"}},
{ReactionTPMethodType::dr_volume_constant, {""}},
{ReactionTPMethodType::logk_dolejs_manning10, {"dr_dolejs_manning10_coeffs"}},
};


const std::map<const SubstanceTPMethodType, const std::vector<std::string>> substance_method_parameters = {
{SubstanceTPMethodType::cp_ft_equation, {"m_heat_capacity_ft_coeffs"} },
{SubstanceTPMethodType::solute_hkf88_gems, {"eos_hkf_coeffs"} },
{SubstanceTPMethodType::solute_hkf88_reaktoro, {"eos_hkf_coeffs"} },
{SubstanceTPMethodType::water_diel_jnort91_reaktoro, {""} },
{SubstanceTPMethodType::water_diel_jnort91_gems, {""} },
{SubstanceTPMethodType::water_diel_sverj14, {""} },
{SubstanceTPMethodType::water_diel_fern97, {""} },
{SubstanceTPMethodType::landau_holland_powell98, {"m_landau_phase_trans_props"} },
{SubstanceTPMethodType::landau_berman88, {"m_berman_phase_trans_props"} },
{SubstanceTPMethodType::cp_ft_equation_saxena86, {"m_heat_capacity_ft_coeffs"} },
{SubstanceTPMethodType::water_eos_hgk84_lvs83_gems, {""} },
{SubstanceTPMethodType::water_eos_iapws95_gems, {""} },
{SubstanceTPMethodType::water_eos_hgk84_reaktoro, {""} },
{SubstanceTPMethodType::water_eos_iapws95_reaktoro, {""} },
{SubstanceTPMethodType::water_pvt_zhang_duan05, {""} },
{SubstanceTPMethodType::mv_constant, {""} },
{SubstanceTPMethodType::mv_pvnrt, {""} },
{SubstanceTPMethodType::mv_equation_dorogokupets88, {"m_volume_fpt_coeffs"} },
{SubstanceTPMethodType::mv_equation_berman88, {"m_volume_fpt_coeffs"} },
{SubstanceTPMethodType::mv_eos_birch_murnaghan_gott97, {"eos_birch_murnaghan_coeffs"} },
{SubstanceTPMethodType::mv_eos_murnaghan_hp98, {"eos_birch_murnaghan_coeffs"} },
{SubstanceTPMethodType::mv_eos_tait_hp11, {""} },
{SubstanceTPMethodType::fug_critical_param, {"eos_gas_crit_props"} },
{SubstanceTPMethodType::fluid_prsv, {""} },
{SubstanceTPMethodType::fluid_churakov_gottschalk, {"eos_churakov_gottschalk_coeffs"} },
{SubstanceTPMethodType::fluid_soave_redlich_kwong, {""} },
{SubstanceTPMethodType::fluid_sterner_pitzer, {""} },
{SubstanceTPMethodType::fluid_peng_robinson78, {""} },
{SubstanceTPMethodType::fluid_comp_redlich_kwong_hp91, {""} },
{SubstanceTPMethodType::solute_aknifiev_diamond03, {"eos_akinfiev_diamond_coeffs"} },
{SubstanceTPMethodType::solute_holland_powell98, {"solute_holland_powell98_coeffs"} },
{SubstanceTPMethodType::solute_anderson91, {"solute_anderson91_coeffs"} },
{SubstanceTPMethodType::standard_entropy_cp_integration, {""} }
};


auto availableSubstanceTPMethods() -> const std::string;
auto availableReactionTPMethods() -> const std::string;
auto availablePropertiesReaction() -> const std::string;
auto availablePropertiesSubstance() -> const std::string;

const std::unordered_map<std::string, WaterTripleProperties>& getWaterTripleData();


/// Indexes for species-dependent EoS subroutines used in thrift DOM and ThermoFun class
typedef struct {
  enum type {
    CTPM_CON = 99, // constant properties from ref T-P
    CTPM_CPT = 100,
    CTPM_HKF = 101,
    CTPM_REA = 102,
    CTPM_EOS = 103,
    CTPM_ISO = 104,
    CTPM_SOR = 105,
    CEM_OFF = 106,
    CEM_GAS = 107,
    CEM_H2O = 108,
    CEM_CO2 = 109,
    CEM_CH4 = 110,
    CEM_N2 = 111,
    CEM_H2 = 112,
    CEM_O2 = 113,
    CEM_AR = 114,
    CEM_PO = 115,
    CEM_NP = 116,
    CTPM_WJNR  = 117, /// calculation of the electro-chemical properties of H2O using the Johnson-Norton 1991 model as implemented in Reaktoro
    CTPM_WJNG  = 118,  /// calculation of the electro-chemical properties of H2O using the Johnson-Norton 1991 model as implemented in GEMS
    CTPM_HKFR  = 119, /// HKFreaktoro
    CTPM_WSV14 = 120, /// calculation of dielectric constant using the model of Sverjensky (2014)
    CTPM_WF97  = 121, /// calculation of dielectric constant using the model of Fernandez et al. (1997)
    CTPM_HP98 = 122, /// HP98 aq
    CTPM_AN91 = 123 /// AN91 aq
  };
}  MethodGenEoS_Thrift;
static const int MethodGenEoS_ndxThrift[] = {
  MethodGenEoS_Thrift::CTPM_CPT,
  MethodGenEoS_Thrift::CTPM_HKF,
  MethodGenEoS_Thrift::CTPM_REA,
  MethodGenEoS_Thrift::CTPM_EOS,
  MethodGenEoS_Thrift::CTPM_ISO,
  MethodGenEoS_Thrift::CTPM_SOR,
  MethodGenEoS_Thrift::CEM_OFF,
  MethodGenEoS_Thrift::CEM_GAS,
  MethodGenEoS_Thrift::CEM_H2O,
  MethodGenEoS_Thrift::CEM_CO2,
  MethodGenEoS_Thrift::CEM_CH4,
  MethodGenEoS_Thrift::CEM_N2,
  MethodGenEoS_Thrift::CEM_H2,
  MethodGenEoS_Thrift::CEM_O2,
  MethodGenEoS_Thrift::CEM_AR,
  MethodGenEoS_Thrift::CEM_PO,
  MethodGenEoS_Thrift::CEM_NP
};
/// Codes for species-dependent EoS subroutines in GEMS
//static const char* MethodGenEoS_GEMS[] = {
//  "C",          ///< CTPM_CPT integration of empirical heat capacity equation Cp=f(T); DComp and ReacDC
//  "H",          ///< CTPM_HKF Helgeson-Kirkham-Flowers (HKF) equation of state for aqueous species
//  "K",          ///< CTPM_REA calculation of logK of reactions as they depend functionally on T and P; ReacDC ####
//  "S",          ///< CTPM_EOS calculations via general equations of state (reserved)
//  "I",          ///< CTPM_ISO calculation of parameters for isotopic forms of compounds (Grichuk, 1988); ReacDC ####
//  "X",          ///< CTPM_SOR calculations via adsorption or ion exchange constants, using LFER correlations; ReacDC (reserved) ####
//  "N",          ///< CEM_OFF no fluid model routicompne
//  "G",          ///< CEM_GAS generic fluid model routine
//  "V",          ///< CEM_H2O subroutine for H2O fluid
//  "C",          ///< CEM_CO2 subroutine for CO2 fluid
//  "M",          ///< CEM_CH4 subroutine for CH4 fluid
//  "T",          ///< CEM_N2 subroutine for N2 fluid
//  "H",          ///< CEM_H2 subroutine for H2 fluid
//  "O",          ///< CEM_O2 subroutine for O2 fluid
//  "A",          ///< CEM_AR subroutine for Ar fluid
//  "P",          ///< CEM_PO subroutine for polar fluids
//  "Q"           ///< CEM_NP subroutine for nonpolar fluids
//};

/// Indexes for temperature correction methods used in thrift DOM and ThermoFun class
typedef struct {
  enum type {
    CTM_CST = 200,
    CTM_CHP = 201,
    CTM_BER = 202,
    CTM_FEI = 203,
    CTM_LGX = 204,
    CTM_HKF = 205,
    CTM_WAT = 206,
    CTM_WAS = 207,
    CTM_LGK = 208,
    CTM_EK0 = 209,
    CTM_EK1 = 210,
    CTM_EK2 = 211,
    CTM_EK3 = 212,
    CTM_IKZ = 213,
    CTM_DKR = 214,
    CTM_PPE = 215,
    CTM_MRB = 216,
    CTM_CPG = 217,
    CTM_IFG = 218,
    CTM_KAS = 219,
    CTM_DAS = 220,
    CTM_WAR = 221,
    CTM_WWP = 222,
    CTM_WZD = 223,
    CTM_DMD = 224,
  };
} MethodCorrT_Thrift;
static const int MethodCorrT_ndxThrift[] = {
  MethodCorrT_Thrift::CTM_CST,
  MethodCorrT_Thrift::CTM_CHP,
  MethodCorrT_Thrift::CTM_BER,
  MethodCorrT_Thrift::CTM_FEI,
  MethodCorrT_Thrift::CTM_LGX,
  MethodCorrT_Thrift::CTM_HKF,
  MethodCorrT_Thrift::CTM_WAT,
  MethodCorrT_Thrift::CTM_WAS,
  MethodCorrT_Thrift::CTM_LGK,
  MethodCorrT_Thrift::CTM_EK0,
  MethodCorrT_Thrift::CTM_EK1,
  MethodCorrT_Thrift::CTM_EK2,
  MethodCorrT_Thrift::CTM_EK3,
  MethodCorrT_Thrift::CTM_IKZ,
  MethodCorrT_Thrift::CTM_DKR,
  MethodCorrT_Thrift::CTM_PPE,
  MethodCorrT_Thrift::CTM_MRB,
  MethodCorrT_Thrift::CTM_CPG,
  MethodCorrT_Thrift::CTM_IFG,
  MethodCorrT_Thrift::CTM_KAS,
  MethodCorrT_Thrift::CTM_DAS,
  MethodCorrT_Thrift::CTM_WAR,      /// calculation of H2O water (steam) properties from Reaktoro (HGK implementation)
  MethodCorrT_Thrift::CTM_WWP,      /// Wagner and Pruss 1995 H2O EOS as implemented in reaktoro
  MethodCorrT_Thrift::CTM_WZD,       /// calculation of water proeprties using the Zhang and Duan (2005) EOS
  MethodCorrT_Thrift::CTM_DMD
};
/// Codes for temperature correction methods used in GEMS
//static const char* MethodCorrT_GEMS[] = {
//  "S",            ///< CTM_CST calculation of ordinary integral using 11-term Cp=f(T) equations (up to 3 phase transitions)
//  "H",            ///< CTM_CHP modifications for Landau phase transitions (Holland and Powell, 1998)
//  "B",            ///< CTM_BER modifications for Landau phase transitions (Berman, 1988) (reserved)
//  "F",            ///< CTM_FEI Cp=f(T) equation by Fei and Saxena (1986) (reserved)
//  "X",            ///< CTM_LGX calculation of reaction properties from empirical from 5-term Cp(T); ReacDC
//  "K",            ///< CTM_HKF calculation with HKF EOS (for aqueous species) as in SUPCRT92
//  "W",            ///< CTM_WAT calculation of H2O water(steam) properties from HGK and LVS equations of state
//  "V",            ///< CTM_WAS calculation of H2O water (steam) properties from IAPWS-95 equation of state (reserved)
//  "L",            ///< CTM_LGK calculation from empirical function logK=f(T) (Nordstrom and Munoz, 1988); ReacDC only
//  "0",            ///< CTM_EK0 one-term extrapolation assuming dHr=0 and logK=const; ReacDC
//  "1",            ///< CTM_EK1 one-term extrapolation assuming dGr=const (Gu et al., 1994); ReacDC
//  "2",            ///< CTM_EK2 two-term extrapolation; ReacDC only
//  "3",            ///< CTM_EK3 three-term extrapolation assuming dCpr=const; ReacDC
//  "Z",            ///< CTM_IKZ Lagrange polynomial interpolation over logK(TP) array; ReacDC
//  "R",            ///< CTM_DKR calculation of logK=f(T,P) from density equation (Marshall and Franck, 1978); ReacDC
//  "M",            ///< CTM_DMD
//  "E",            ///< CTM_PPE prediction of properties of aqueous hydroxides using Pronsprep-OH (Shock et al. 1997)
//  "Y",            ///< CTM_MRB calculation of logK=f(T,P) with modified Ryzhenko-Bryzgalin model
//  "C",            ///< CTM_CPG reserved (FGL)
//  "G",            ///< CTM_IFG calculation of parameters for isotopic forms of compounds (Grichuk, 1988); ReacDC
//  "K",            ///< CTM_KAS reserved
//  "D",            ///< CTM_DAS reserved
//  "T"             ///< CTM_WAR calculation of H2O water (steam) properties from Reaktoro (HGK implementation)
//};

/// Indexes for pressure correction methods used in thrift DOM and ThermoFun class
typedef struct {
  enum type {
    CPM_OFF = 300,
    CPM_NUL = 301,
    CPM_ONE = 302,
    CPM_TWO = 303,
    CPM_TRI = 304,
    CPM_CON = 305,
    CPM_VKE = 306,
    CPM_VBE = 307,
    CPM_VBM = 308,
    CPM_CEH = 309,
    CPM_VTA = 310,
    CPM_GAS = 311,
    CPM_PRSV = 312,
    CPM_EMP = 313,
    CPM_SRK = 314,
    CPM_STP = 315,
    CPM_PR78 = 316,
    CPM_CORK = 317,
    CPM_REFL = 318,
    CPM_HKF = 319,
    CPM_AKI = 320,
    CPM_PCR = 321,
    CPM_INK = 322,
  };
} MethodCorrP_Thrift;
static const int MethodCorrP_ndxThrift[] = {
  MethodCorrP_Thrift::CPM_OFF,
  MethodCorrP_Thrift::CPM_NUL,
  MethodCorrP_Thrift::CPM_ONE,
  MethodCorrP_Thrift::CPM_TWO,
  MethodCorrP_Thrift::CPM_TRI,
  MethodCorrP_Thrift::CPM_CON,
  MethodCorrP_Thrift::CPM_VKE,
  MethodCorrP_Thrift::CPM_VBE,
  MethodCorrP_Thrift::CPM_VBM,
  MethodCorrP_Thrift::CPM_CEH,
  MethodCorrP_Thrift::CPM_VTA,
  MethodCorrP_Thrift::CPM_GAS,
  MethodCorrP_Thrift::CPM_PRSV,
  MethodCorrP_Thrift::CPM_EMP,
  MethodCorrP_Thrift::CPM_SRK,
  MethodCorrP_Thrift::CPM_STP,
  MethodCorrP_Thrift::CPM_PR78,
  MethodCorrP_Thrift::CPM_CORK,
  MethodCorrP_Thrift::CPM_REFL,
  MethodCorrP_Thrift::CPM_HKF,
  MethodCorrP_Thrift::CPM_AKI,
  MethodCorrP_Thrift::CPM_PCR,
  MethodCorrP_Thrift::CPM_INK
};
/// Codes for pressure correction methods used in GEMS
//static const char* MethodCorrP_GEMS[] = {
//  "N",            ///< CPM_OFF no account for pressure dependence
//  "0",            ///< CPM_NUL account for pressure dependence is already specified as inherent to a method
//  "1",            ///< CPM_ONE V0
//  "2",            ///< CPM_TWO
//  "3",            ///< CPM_TRI
//  "C",            ///< CPM_CON molar volume of DC or dVr are assumed independent of P and T
//  "K",            ///< CPM_VKE molar volume from empirical V=f(T,P) equation (Dorogokupets et al., 1988)
//  "V",            ///< CPM_VBE molar volume from empirical  V=f(T,P) equation (Berman, 1988)
//  "B",            ///< CPM_VBM molar volume from Birch-Murnaghan EoS (Gottschalk, 1997)
//  "E",            ///< CPM_CEH molar volume V(T,P) from Murnaghan EoS (Holland and Powell, 1998)
//  "H",            ///< CPM_VTA molar volume V(T,P) from Tait EoS (Holland and Powell, 2011)
//  "G",            ///< CPM_GAS calculation of fugacities of gases from their critical parameters (reserved)
//  "R",            ///< CPM_PRSV Peng-Robinson-Stryjek-Vera (PRSV) EoS for fluids (nonelectrolytes)
//  "S",            ///< CPM_EMP Churakov-Gottschalk (CG) EoS for fluids (nonelectrolytes
//  "T",            ///< CPM_SRK Soave-Redlich-Kwong (SRK) EoS for fluids (nonelectrolytes)
//  "6",            ///< CPM_STP Sterner-Pitzer (SP) EoS for fluids (nonelectrolytes) (reserved)
//  "7",            ///< CPM_PR78 Peng-Robinson (PR78) EoS for fluids (nonelectrolytes)
//  "8",            ///< CPM_CORK Compensated Redlich-Kwong (CORK) EoS for fluids (nonelectrolytes)
//  "9",            ///< CPM_REFL reference EoS model for fluids (nonelectrolytes) (reserved)
//  "F",            ///< CPM_HKF account for pressure up to 5 kbar by HKF (Tanger and Helgeson, 1988)
//  "A",            ///< CPM_AKI partial molal volumes of aqueous nonelectrolyte species (Akinfiev and Diamond, 2003)
//  "P",            ///< CPM_PCR PARCOR estimation of HKF EoS parameters
//  "Z"             ///< CPM_INK Lagrange polynomial interpolation over logK(TP) array; ReacDC
//};

static const std::map<const SubstanceTPMethodType, const int> new_old_s_methodtype = {
  {SubstanceTPMethodType::cp_ft_equation, 100},
  {SubstanceTPMethodType::solute_hkf88_gems, 101},
  {SubstanceTPMethodType::solute_hkf88_reaktoro, 119},
  {SubstanceTPMethodType::water_diel_jnort91_reaktoro, 117},
  {SubstanceTPMethodType::water_diel_jnort91_gems, 118},
  {SubstanceTPMethodType::water_diel_sverj14, 120},
  {SubstanceTPMethodType::water_diel_fern97, 121},
  {SubstanceTPMethodType::landau_holland_powell98, 201},
  {SubstanceTPMethodType::landau_berman88, 202},
  {SubstanceTPMethodType::cp_ft_equation_saxena86, 203},
  {SubstanceTPMethodType::water_eos_hgk84_lvs83_gems, 206},
  {SubstanceTPMethodType::water_eos_iapws95_gems, 207},
  {SubstanceTPMethodType::water_eos_hgk84_reaktoro, 221},
  {SubstanceTPMethodType::water_eos_iapws95_reaktoro, 222},
  {SubstanceTPMethodType::water_pvt_zhang_duan05, 223},
  {SubstanceTPMethodType::mv_constant, 305},
  {SubstanceTPMethodType::mv_equation_dorogokupets88, 306},
  {SubstanceTPMethodType::mv_equation_berman88, 307},
  {SubstanceTPMethodType::mv_eos_birch_murnaghan_gott97, 308},
  {SubstanceTPMethodType::mv_eos_murnaghan_hp98, 309},
  {SubstanceTPMethodType::mv_eos_tait_hp11, 310},
  {SubstanceTPMethodType::fug_critical_param, 311},
  {SubstanceTPMethodType::fluid_prsv, 312},
  {SubstanceTPMethodType::fluid_churakov_gottschalk, 313},
  {SubstanceTPMethodType::fluid_soave_redlich_kwong, 314},
  {SubstanceTPMethodType::fluid_sterner_pitzer, 315},
  {SubstanceTPMethodType::fluid_peng_robinson78, 316},
  {SubstanceTPMethodType::fluid_comp_redlich_kwong_hp91, 317},
  {SubstanceTPMethodType::solute_aknifiev_diamond03, 320},
  {SubstanceTPMethodType::mv_pvnrt, 300},
  {SubstanceTPMethodType::standard_entropy_cp_integration, 200}
};

static const std::map<const ReactionTPMethodType, const int> new_old_r_methodtype = {
  {ReactionTPMethodType::dr_heat_capacity_ft, 100},
  {ReactionTPMethodType::adsor_ion_exchange, 105},
  {ReactionTPMethodType::logk_fpt_function, 102},
  {ReactionTPMethodType::iso_compounds_grichuk88, 104},
  {ReactionTPMethodType::logk_nordstrom_munoz88, 208},
  {ReactionTPMethodType::logk_1_term_extrap0, 209},
  {ReactionTPMethodType::logk_1_term_extrap1, 210},
  {ReactionTPMethodType::logk_2_term_extrap, 211},
  {ReactionTPMethodType::logk_3_term_extrap, 212},
  {ReactionTPMethodType::logk_lagrange_interp, 213},
  {ReactionTPMethodType::logk_marshall_frank78, 214},
  {ReactionTPMethodType::solute_eos_ryzhenko_gems, 216},
  {ReactionTPMethodType::dr_volume_fpt, 306},
  {ReactionTPMethodType::dr_volume_constant, 305 },
  {ReactionTPMethodType::logk_dolejs_manning10, 224 }
};

typedef struct {
  enum type {
    C_MOLFRACTION = 0,
    C_PARTIALPRESSURE = 1,
    C_MOLALITY = 2,
    C_MOLARITY = 3,
    C_MASSFRACTION = 4,
    C_SURFACEFRACTION = 5,
    C_SURFACEDENSITY = 6,
    C_SITEFRACTION = 7,
    C_OTHER = 101
  };
} ConcentrationScales;

static const int ConcentrationScales_ndxThrift[] = {
  ConcentrationScales::C_MOLFRACTION,
  ConcentrationScales::C_PARTIALPRESSURE,
  ConcentrationScales::C_MOLALITY,
  ConcentrationScales::C_MOLARITY,
  ConcentrationScales::C_MASSFRACTION,
  ConcentrationScales::C_SURFACEFRACTION,
  ConcentrationScales::C_SURFACEDENSITY,
  ConcentrationScales::C_SITEFRACTION,
  ConcentrationScales::C_OTHER
};

//const char* ConcentrationScales_GEMS[] = {
//  "C_MOLFRACTION",
//  "C_PARTIALPRESSURE",
//  "C_MOLALITY",
//  "C_MOLARITY",
//  "C_MASSFRACTION",
//  "C_SURFACEFRACTION",
//  "C_SURFACEDENSITY",
//  "C_SITEFRACTION",
//  "C_OTHER"
//};

typedef struct {
  enum type {
    COMPONENT = 0,
    GASFLUID = 1,
    AQSOLUTE = 2,
    AQSOLVENT = 3,
    SURFSPECIES = 4,
    SITEMOIETY = 5,
    OTHER_SC = 101
  };
} SubstanceClass;

static const int SubstanceClass_ndxThrift[] = {
  SubstanceClass::COMPONENT,
  SubstanceClass::GASFLUID,
  SubstanceClass::AQSOLUTE,
  SubstanceClass::AQSOLVENT,
  SubstanceClass::SURFSPECIES,
  SubstanceClass::SITEMOIETY,
  SubstanceClass::OTHER_SC
};

//const char* SubstanceClass_GEMS[] = {
//  "COMPONENT",
//  "GASFLUID",
//  "AQSOLUTE",
//  "AQSOLVENT",
//  "SURFSPECIES",
//  "SITEMOIETY",
//  "OTHER_SC"
//};

typedef struct {
  enum type {
    GAS = 0,              ///< Gaseous aggregate state (symbol g)
    LIQUID = 1,           ///< Condensed liquid aggregate state (symbol l)
    GLASS = 2,            ///< Glass/Vitreous state (symbol vit)
    CRYSTAL = 3,          ///< Crystalline solid aggregate state (symbol cr)
    AQUEOUS = 4,          ///< Aqueous electrolyte (symbol aq)
    SURFACE = 5,          ///< Surface layer, adsorbed species (symbol ads)
    IONEX = 6,            ///< Ion exchange (Donnan etc.) (eymbol ex)
    PLASMA = 8,           ///< Plasma (symbol pl)
    SOLID = 9,            ///< Solid (symbol s)
    CONDENSED = 10,       ///< CondensedPhase (symbol cd)
    FLUID = 11,           ///< Fluid (symbol fl)
    LIQUIDCRYSTAL = 12,   ///< LiquidCrystal (symbol lc)
    AMORPHOUSSOLID = 13,  ///< AmorphousSolid (symbol am)
    MONOMERIC = 14,       ///< Monomeric (symbol mon)
    POLYMERIC = 15,       ///< Polymeric (symbol pol)
    SOLIDSOLUTION = 16,   ///< SolidSolution (symbol ss)    
    OTHER = 101           ///< Other states, undefined
  };
} AggregateState;

static const int AggregateState_ndxThrift[] = {
  AggregateState::GAS,
  AggregateState::LIQUID,
  AggregateState::GLASS,
  AggregateState::CRYSTAL,
  AggregateState::AQUEOUS,
  AggregateState::SURFACE,
  AggregateState::IONEX,
  AggregateState::PLASMA,
  AggregateState::SOLID,
  AggregateState::CONDENSED,
  AggregateState::FLUID,
  AggregateState::LIQUIDCRYSTAL,
  AggregateState::AMORPHOUSSOLID,
  AggregateState::MONOMERIC,
  AggregateState::POLYMERIC,
  AggregateState::SOLIDSOLUTION,  
  AggregateState::OTHER
};

//const char* AggregateState_GEMS[] = {
//  "GAS",
//  "LIQUID",
//  "GLASS",
//  "CRYSTAL",
//  "AQUEOUS",
//  "SURFACE",
//  "OTHER_AS"
//};


typedef struct {
  enum type {
    REACTANT = 0,           ///< Component properties available
    PRODUCT = 1,            ///< Component properties to be defined via this reaction properties
    CATALYST = 2,           ///< Catalyzes the reaction but is not consumed in it
    OTHER_RC = 101            ///< Other type of reaction component
  };
} ReactionComponentType;


typedef struct {
  enum type {
    DCOMP = 0,            ///< Dependent component, properties calculated using a PT model for the substance
    REACDC = 1            ///< Reaction dependent component, properties calculated from the reaction properties
  };
} SubstanceThermoCalculationType;

typedef struct {
  enum type {
    REACTION = 0,            ///< Reaction, properties calculated using a PT model for the reaction
    REACTANTS = 1            ///< Properties calculated from reactants
  };
} RectionThermoCalculationType;


/// Key for reading substance / reaction data from input files
static const char * label                   = "_label";
static const char * _id                     = "_id";

/// Element
static const char * elemName               = "properties.name";
static const char * elemSymbol             = "properties.symbol";
static const char * elemMolarMass          = "properties.atomic_mass";
static const char * elemEntropy            = "properties.entropy";
static const char * elemHeatCapacity       = "properties.heat_capacity";
static const char * elemClass              = "properties.class_";
static const char * elemIsotopeMass        = "properties.isotope_mass";

/// Substance
static const char * substName               = "properties.name";
static const char * substSymbol             = "properties.symbol";
static const char * substFormula            = "properties.formula";
static const char * substCharge             = "properties.formula_charge";
static const char * substMolarMass          = "properties.mass_per_mole";
static const char * substClass              = "properties.class_";
static const char * substAggState           = "properties.aggregate_state";
static const char * substSolventNname       = "";
static const char * substMethodEOS          = "properties.method_genEoS";
static const char * substMethodT            = "properties.method_corrT";
static const char * substMethodP            = "properties.method_corrP";
static const char * substRefT               = "properties.Tst";
static const char * substRefP               = "properties.Pst";

/// Reference properties
static const char * substRefG0_              = "properties.sm_gibbs_energy";
static const char * substRefS0_              = "properties.sm_entropy_abs";
static const char * substRefH0_              = "properties.sm_enthalpy";
static const char * substRefV0_              = "properties.sm_volume";
static const char * substRefCp0_             = "properties.sm_heat_capacity_p";

static const char * substExpans_             = "properties.m_expansivity";
static const char * substCompres_            = "properties.m_compressibility";

/// Model parameters
static const char * substEOShkf             = "eos_hkf_coeffs.values"; //
static const char * substEOSad              = "eos_akinfiev_diamond_coeffs.values"; //
static const char * substEOSbm              = "eos_birch_murnaghan_coeffs.values"; //
static const char * substEOScg              = "eos_churakov_gottschalk_coeffs.values"; //
static const char * substEOSgasCrit         = "eos_gas_crit_props.values"; //
static const char * substCpParam            = "m_heat_capacity_ft_coeffs.values"; //
static const char * substTransProp          = "m_phase_trans_props.values"; //
static const char * substTransPropLa        = "m_landau_phase_trans_props.values"; //
static const char * substTransPropBm        = "phase_transition_prop_Berman.values";

static const char * lowerT                  = "limitsTP.lowerT";
static const char * upperT                  = "limitsTP.upperT";
static const char * lowerP                  = "limitsTP.lowerP";
static const char * upperP                  = "limitsTP.upperP";

static const char * plowerT                  = "properties.limitsTP.lowerT";
static const char * pupperT                  = "properties.limitsTP.upperT";
static const char * plowerP                  = "properties.limitsTP.lowerP";
static const char * pupperP                  = "properties.limitsTP.upperP";

/// Key for reading reaction data from input files
static const char * reacName               = "properties.name";
static const char * reacSymbol             = "properties.symbol";
static const char * reacEquation           = "properties.equation";

static const char * reacMethodEOS          = "properties.method_genEoS";
static const char * reacMethodT            = "properties.method_corrT";
static const char * reacMethodP            = "properties.method_corrP";
static const char * reacRefT               = "properties.Tst";
static const char * reacRefP               = "properties.Pst";

static const char * TPMethods              = "properties.TPMethods";

/// Reference properties
static const char * reacRefLogK0_           = "properties.logKr";
static const char * reacRefG0_              = "properties.drsm_gibbs_energy";
static const char * reacRefS0_              = "properties.drsm_entropy";
static const char * reacRefH0_              = "properties.drsm_enthalpy";
static const char * reacRefV0_              = "properties.drsm_volume";
static const char * reacRefCp0_             = "properties.drsm_heat_capacity_p";

/// Model parameters
static const char * reacLogKfT             = "logk_ft_coeffs.values"; //
static const char * reacLogKPT             = "logk_pt_values.pptv"; //
static const char * reacDrCpfT             = "dr_heat_capacity_ft_coeffs.values"; //
static const char * reacDrVfT              = "dr_volume_fpt_coeffs.values"; //
static const char * reacRBcoeff            = "dr_ryzhenko_coeffs.values"; //
static const char * reacFMcoeff            = "dr_marshall_franck_coeffs.values"; //


//typedef std::vector<struct SubstanceData*>  Substances;



//typedef std::vector<struct ReactionData>  Reactions;

/// Output a array instance
template<typename V>
auto operator<<(std::ostream& out, const std::vector<V>& array) -> std::ostream&
{
    out << std::string("[ ");
    for(const auto& val: array) {
      out << val << " ";
    }
    out << std::string("]");
    return out;
}

}
///@endcond

#endif // ThermoFun_GLOBAL_H
