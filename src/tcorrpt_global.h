#ifndef TCORRPT_GLOBAL_H
#define TCORRPT_GLOBAL_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>

#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wsign-compare"

using namespace std;

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
                    K_to_C =-273.17,
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

struct STATES
{
    double Temp;    // C
    double Pres;    // bar
    double Dens[2]; // g/cm3
    STATES()
    {
        Temp = -274.;
        Pres = -1;
        Dens[0] =Dens[1] = 0.;
    }
    void setdef()
    {
        Temp = -274.;
        Pres = -1;
        Dens[0] =Dens[1] = 0.;
    }
};



/// Indexes for species-dependent EoS subroutines used in thrift DOM and TCorrPT class
typedef struct {
  enum type {
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
    CEM_NP = 116
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
//  "N",          ///< CEM_OFF no fluid model routine
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

/// Indexes for temperature correction methods used in thrift DOM and TCorrPT class
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
    CTM_WAR = 221
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
  MethodCorrT_Thrift::CTM_WAR
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
//  "E",            ///< CTM_PPE prediction of properties of aqueous hydroxides using Pronsprep-OH (Shock et al. 1997)
//  "Y",            ///< CTM_MRB calculation of logK=f(T,P) with modified Ryzhenko-Bryzgalin model
//  "C",            ///< CTM_CPG reserved (FGL)
//  "G",            ///< CTM_IFG calculation of parameters for isotopic forms of compounds (Grichuk, 1988); ReacDC
//  "K",            ///< CTM_KAS reserved
//  "D",            ///< CTM_DAS reserved
//  "T"             ///< CTM_WAR calculation of H2O water (steam) properties from Reaktoro (HGK implementation)
//};

/// Indexes for pressure correction methods used in thrift DOM and TCorrPT class
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
    CPM_INK = 322
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
    C_OTHER = 8
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
    OTHER_SC = 6
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
    GAS = 0,
    LIQUID = 1,
    GLASS = 2,
    CRYSTAL = 3,
    AQUEOUS = 4,
    SURFACE = 5,
    OTHER_AS = 6
  };
} AggregateState;

static const int AggregateState_ndxThrift[] = {
  AggregateState::GAS,
  AggregateState::LIQUID,
  AggregateState::GLASS,
  AggregateState::CRYSTAL,
  AggregateState::AQUEOUS,
  AggregateState::SURFACE,
  AggregateState::OTHER_AS
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
    OTHER_RC = 3            ///< Other type of reaction component
  };
} ReactionComponentType;


typedef struct {
  enum type {
    DCOMP = 0,            ///< Dependent component, properties calculated using a PT model for the substance
    REACDC = 1            ///< Reaction dependent component, proeprties claculated from the reaction properties
  };
} SubstanceThermoCalculationType;



//typedef std::vector<struct SubstanceData*>  Substances;



//typedef std::vector<struct ReactionData>  Reactions;


#endif // TCORRPT_GLOBAL_H
