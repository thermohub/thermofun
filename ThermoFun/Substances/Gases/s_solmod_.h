//-------------------------------------------------------------------
// $Id: s_solmod.h 725 2012-10-02 15:43:37Z kulik $
//
/// \file s_solmod.h
/// Declarations of TSolMod and derived classes implementing built-in models
/// of mixing in fluid, liquid, aqueous and solid-solution phases

// Copyright (C) 2003-2014  T.Wagner, D.Kulik, S.Dmitrieva, F.Hingerl, S.Churakov
// <GEMS Development Team, mailto:gems2.support@psi.ch>
//
// This file is part of the GEMS4K code for thermodynamic modelling
// by Gibbs energy minimization <http://gems.web.psi.ch/GEMS4K/>
//
// GEMS4K is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// GEMS4K is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GEMS4K code. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------
//

#ifndef _s_solmod_h_
#define _s_solmod_h_

#include <vector>
#include "Common/Real.hpp"

namespace solmod
 {
using ThermoFun::real;
using ThermoFun::fabs;
using ThermoFun::pow;


// re-declaration of enums below required for GEMS4K
// dc_class_codes for fluids will be replaced by tp_codes
enum fluid_mix_rules {  /// codes for mixing rules in EoS models (see m_phase.h)
    MR_UNDEF_ = 'N',
    MR_WAAL_ = 'W',
    MR_CONST_ = 'C',
    MR_TEMP_ = 'T',
    MR_LJ_ = 'J',
    MR_KW1_ = 'K',
    MR_PITZ5_ = '5',
    MR_PITZ6_ = '6',
    MR_PITZ8_ = '8',
    MR_B_RCPT_ = 'R'
};

enum dc_class_codes {  /// codes for fluid types in EoS models (see v_mod.h)
    DC_GAS_H2O_ = 'V',
    DC_GAS_CO2_ = 'C',
    DC_GAS_H2_ = 'H',
    DC_GAS_N2_ = 'N',
    DC_GAS_COMP_ = 'G'
};

enum tp_codes {  /// codes for fluid subroutines in EoS models (see v_mod.h)
    CEM_OFF_ = 'N',
    CEM_GAS_ = 'G',
    CEM_H2O_ = 'V',
    CEM_CO2_ = 'C',
    CEM_CH4_ = 'M',
    CEM_N2_ = 'T',
    CEM_H2_ = 'H',
    CEM_O2_ = 'O',
    CEM_AR_ = 'A',
    CEM_PO_ = 'P',
    CEM_NP_ = 'Q'
};


// ------------------------------------------------------------------

#define MAXPHASENAME 16

/// Base class for subclasses of built-in mixing models.
/// (c) March 2007 DK/TW
struct SolutionData {
    long int NSpecies;  ///< Number of species (end members) in the phase
    long int NParams;   ///< Total number of non-zero interaction parameters
    long int NPcoefs;   ///< Number of coefficients per interaction parameter
    long int MaxOrder;  ///< Maximum order of interaction parameters
    long int NPperDC;   ///< Number of parameters per species (DC)
    long int NSublat;   ///< number of sublattices nS
    long int NMoiet;    ///< number of moieties nM

//    long int NlPhs;     ///< new: Number of linked phases
//    long int NlPhC;     ///< new: Number of linked phase parameter coefficient per link (default 0)
    long int NDQFpDC;   ///< new: Number of DQF parameters per species (end member)
//    long int NrcPpDC;   ///< new: Number of reciprocal parameters per species (end member)

    char Mod_Code;      ///< Code of the mixing model
    char Mix_Code;      ///< Code for specific EoS mixing rule
    char *DC_Codes;     ///< DC class codes for species -> NSpecies
    char (*TP_Code)[6]; ///< Codes for TP correction methods for species ->NSpecies
    long int *arIPx;    ///< Pointer to list of indexes of non-zero interaction parameters

//    long int *arPhLin;  ///< new: indexes of linked phase and link type codes [NlPhs*2] read-only

    real *arIPc;      ///< Table of interaction parameter coefficients
    real *arDCc;      ///< End-member properties coefficients
    real *arMoiSN;    ///< End member moiety- site multiplicity number tables -> NSpecies x NSublat x NMoiet
    real *arSitFr;    ///< Tables of sublattice site fractions for moieties -> NSublat x NMoiet
    real *arSitFj;    ///< new: Table of end member sublattice activity coefficients -> NSpecies x NSublat
    real *arGEX;      ///< Pure-species fugacities, G0 increment terms  -> NSpecies

//    real *lPhc;  ///< new: array of phase link parameters -> NlPhs x NlPhC (read-only)
    real *DQFc;  ///< new: array of DQF parameters for DCs in phases ->  NSpecies x NDQFpDC; (read-only)
//    real *rcpc;  ///< new: array of reciprocal parameters for DCs in phases -> NSpecies x NrcPpDC; (read-only)

    real *arPparc;    ///< Partial pressures -> NSpecies
    real *arWx;       ///< Species (end member) mole fractions ->NSpecies
    real *arlnGam;    ///< Output: activity coefficients of species (end members)   

    // Detailed output on terms of partial end-member properties, allocated in MULTI
    real *arlnDQFt; ///< new: DQF terms adding to overall activity coefficients [Ls_]
    real *arlnRcpt; ///< new: reciprocal terms adding to overall activity coefficients [Ls_]
    real *arlnExet; ///< new: excess energy terms adding to overall activity coefficients [Ls_]
    real *arlnCnft; ///< new: configurational terms adding to overall activity [Ls_]

    real *arVol;      ///< molar volumes of end-members (species) cm3/mol ->NSpecies
    real *aphVOL;     ///< phase volumes, cm3/mol (now obsolete) !!!!!!! check usage!
    real T_k;         ///< Temperature, K (initial)
    real P_bar;       ///< Pressure, bar (initial)

    void set_def();
};


class TSolMod
{
	protected:
        char ModCode;   ///< Code of the mixing model
        char MixCode;	///< Code for specific EoS mixing rules
        char *DC_Codes; ///< Class codes of end members (species) ->NComp

        char PhaseName[MAXPHASENAME+1];    ///< Phase name (for specific built-in models)

        long int NComp;   ///< Number of components in the solution phase
        long int NPar;     ///< Number of non-zero interaction parameters
        long int NPcoef;   ///< Number of coeffs per parameter (columns in the aIPc table)
        long int MaxOrd;   ///< max. parameter order (or number of columns in aIPx)
        long int NP_DC;    ///< Number of coeffs per one DC in the phase (columns in aDCc)
        long int NSub;     ///< number of sublattices nS
        long int NMoi;     ///< number of moieties nM

//   long int NlPh;     ///< new: Number of linked phases
//   long int NlPc;     ///< new: Number of linked phase parameter coefficient per link (default 0)
   long int NDQFpc;   ///< new: Number of DQF parameters per species (end member), 0 or 4
//   long int NrcPpc;   ///< new: Number of reciprocal parameters per species (end member)

        //        long int NPTP_DC;  // Number of properties per one DC at T,P of interest (columns in aDC)  !!!! Move to CG EOS subclass
      long int *aIPx;    // Pointer to list of indexes of non-zero interaction parameters
//   long int (*PhLin)[2];  ///< new: indexes of linked phase and link type codes [NlPhs][2] read-only

        real R_CONST; ///< R constant
        real Tk;    	///< Temperature, K
        real Pbar;  	///< Pressure, bar

        real *aIPc;   ///< Table of interaction parameter coefficients
        real *aIP;    ///< Vector of interaction parameters corrected to T,P of interest
        real *aDCc;   ///< End-member properties coefficients
        real *aGEX;   ///< Reciprocal energies, DQF terms, pure fugacities of DC (corrected to TP)
        real *aPparc;  ///< Output partial pressures (activities, fugacities) -> NComp
        real **aDC;   ///< Table of corrected end member properties at T,P of interest  !!!!!! Move to GC EOS subclass!
        real *aMoiSN; ///< End member moiety- site multiplicity number tables -> NComp x NSub x NMoi
        real *aSitFR; ///< Table of sublattice site fractions for moieties -> NSub x NMoi

//    real *lPhcf;  ///< new: array of phase link parameters -> NlPh x NlPc (read-only)
    real *DQFcf;  ///< new: array of DQF parameters for DCs in phases ->  NComp x NDQFpc; (read-only)
                    ///< x_DQF[j]: mole fraction at transition; a, b, c - coefficients of T,P correction
                    ///< according to the equation aGEX[j] = A + B*T + C*P (so far only binary Margules)
//    real *rcpcf;  ///< new: array of reciprocal parameters for DCs in phases -> NComp x NrcPpc; (read-only)

        real *x;      ///< Pointer to mole fractions of end members (provided)
        real *aVol;   ///< molar volumes of species (end members)
        real *phVOL;  ///< phase volume, cm3/mol (now obsolete) !!!!!!!!!!!! Check usage!

        // Results
        // real Gam;   	///< work cell for activity coefficient of end member
        // real lnGamRT;
        // real lnGam;
        real Gex, Hex, Sex, CPex, Vex, Aex, Uex;   ///< molar excess properties of the phase
        real Gid, Hid, Sid, CPid, Vid, Aid, Uid;   ///< molar ideal mixing properties
        real Gdq, Hdq, Sdq, CPdq, Vdq, Adq, Udq;   ///< molar Darken quadratic terms
        real Grs, Hrs, Srs, CPrs, Vrs, Ars, Urs;   ///< molar residual functions (fluids)
        real *lnGamConf, *lnGamRecip, *lnGamEx, *lnGamDQF;    ///< Work pointers for lnGamma components
        real *lnGamma;   ///< Pointer to ln activity coefficients of end members (check that it is collected from three above arrays)

        real **y;       ///< table of moiety site fractions [NSub][NMoi]
        real ***mn;     ///< array of end member moiety-site multiplicity numbers [NComp][NSub][NMoi]
        real *mns;      ///< array of total site multiplicities [NSub]
   real **fjs;     ///< array of site activity coefficients [NComp][NSub]
   real *aSitFj; ///< new: pointer to return table of site activity coefficients NComp x NSub

        // functions for calculation of configurational term for multisite ideal mixing
        void alloc_multisite();
        long int init_multisite();
        void free_multisite();
        void free_sdata();


        /// Functions for calculation of configurational term for multisite ideal mixing
        long int IdealMixing();
        real ideal_conf_entropy();
        void return_sitefr();
        void retrieve_sitefr();


        public:

        /// Generic constructor
        TSolMod( SolutionData *sd );

         /// Generic constructor for DComp/DCthermo
        TSolMod( long int NSpecies,  char Mod_Code,  real T_k, real P_bar );

        /// Destructor
		virtual ~TSolMod();

		virtual long int PureSpecies()
		{
			return 0;
        }

		virtual long int PTparam()
		{
			return 0;
        }

		virtual long int MixMod()
		{
			return 0;
        }

        virtual long int ExcessProp( real */*Zex*/ )
		{
			return 0;
        }

        virtual long int IdealProp( real */*Zid*/ )
		{
			return 0;
        }

        virtual long int Set_Felect_bc (long int /*Flagelect*/, real /*Bc*/, real /*Ac*/)
        {
            return 0;
        }


        /// Set new system state
		long int UpdatePT ( real T_k, real P_bar );

        bool testSizes( SolutionData *sd );

        /// Getting phase name
		void GetPhaseName( const char *PhName );

		
        // copy activity coefficients into provided array lngamma
		inline void Get_lnGamma( real* lngamma )		
		{ 
			for( int i=0; i<NComp; i++ )
				lngamma[i] = lnGamma[i]; 
		}

        void getSolutionData( SolutionData *sd );

        // access from node
        void Set_aIPc( const std::vector<real> aIPc_ );
        void Get_aIPc ( std::vector<real> &aIPc_ );

        void Set_aDCc( const std::vector<real> aDCc_ );
        void Get_aDCc( std::vector<real> &aDCc_ );

        void Get_aIPx( std::vector<long int> &aIPx_ );

        void Get_NPar_NPcoef_MaxOrd_NComp_NP_DC ( long int &NPar_, long int &NPcoef_,
                       long int &MaxOrd_,  long int &NComp_, long int &NP_DC_ );

};


/// Subclass for the ideal model (both simple and multi-site)
class TIdeal: public TSolMod
{
            private:

            public:

                    /// Constructor
                    TIdeal( SolutionData *sd );

                    /// Destructor
                    ~TIdeal();

                    /// Calculates T,P corrected interaction parameters
                    long int PTparam();

                    /// Calculates (fictive) activity coefficients
                    long int MixMod();

                    /// Calculates excess properties
                    long int ExcessProp( real *Zex );

                    /// Calculates ideal mixing properties
                    long int IdealProp( real *Zid );

};



/// Churakov & Gottschalk (2003) EOS calculations
/// declaration of EOSPARAM class (used by the TCGFcalc class)
class EOSPARAM
{
	private:
		//static real Told;
		// unsigned long int isize;  // int isize = NComp;
		long int NComp;
		real emix, s3mix;
		real *epspar,*sig3par;
		real *XX;
		real *eps;
		real *eps05;
		real *sigpar;
		real *mpar;
		real *apar;
		real *aredpar;
		real *m2par;
		real **mixpar;

		void allocate();
		void free();

	public:

		real *XX0;

		//EOSPARAM():isize(0),emix(0),s3mix(0),NComp(0){};
		//EOSPARAM(real*data, unsigned nn):isize(0){allocate(nn);init(data,nn);};

		EOSPARAM( real *Xtmp, real *data, long int nn )
			:NComp(nn), emix(0),s3mix(0)
                        { allocate(); init(Xtmp,data,nn); }

		~EOSPARAM()
			{ free(); }

		void init( real*,real *, long int );
                long int NCmp()   {return NComp; }

		real EPS05( long int i)
                        { return eps05[i]; }
		real X( long int i)
                        { return XX[i]; }
		real EPS( long int i)
                        { return eps[i]; }
		real EMIX(void)
                        { return emix; }
		real S3MIX(void)
                        { return s3mix; }

		real MIXS3( long int i, long int j)
		{
			if (i==j) return sig3par[i];
                        if (i<j) return mixpar[i][j];
                            else return mixpar[j][i];
         }

		real MIXES3( long int i, long int j)
		{
			if ( i==j ) return epspar[i];
                        if (i<j) return mixpar[j][i];
                            else return mixpar[i][j];
        }

                real SIG3( long int i){ return sig3par[i]; }
                real M2R( long int i) { return m2par[i]; }
                real A( long int i)   { return apar[i]; }

		long int ParamMix( real *Xin);
};



// -------------------------------------------------------------------------------------
/// Churakov and Gottschalk (2003) EOS calculations.
/// Added 09 May 2003
/// Declaration of a class for CG EOS calculations for fluids
/// Incorporates a C++ program written by Sergey Churakov (CSCS ETHZ)
/// implementing papers by Churakov and Gottschalk (2003a, 2003b)
class TCGFcalc: public TSolMod
{
	private:

                real
                PI_1,    ///< pi
                TWOPI,    ///< 2.*pi
                PISIX,    ///< pi/6.
                TWOPOW1SIX,   ///< 2^(1/6)
                DELTA,
                DELTAMOLLIM,
                R,  NA,  P1,
                PP2, P3, P4,
                P5,  P6, P7,
                P8,  P9, P10,
                AA1, AA2, AA3,
                A4, A5, A6,
                BB1, BB2, BB3,
                B4,  B5,  B6,
                A00, A01, A10,
                A11, A12, A21,
                A22, A23, A31,
                A32, A33, A34;

                //  real PhVol;  // phase volume in cm3
                real *Pparc;     ///< DC partial pressures (pure fugacities)
                real *phWGT;
                real *aX;        ///< DC quantities at eqstate x_j (moles)
                    // real *aGEX;      // Increments to molar G0 values of DCs from pure fugacities
                    // real *aVol;      // DC molar volumes, cm3/mol [L]

                // main work arrays
                EOSPARAM *paar;
                EOSPARAM *paar1;
                real *FugCoefs;
                real *EoSparam;
                real *EoSparam1;
                real (*Cf)[8];   ///< corrected EoS coefficients

                // internal functions
                void alloc_internal();
                void free_internal();
                void set_internal();

                void choose( real *pres, real P,unsigned long int &x1,unsigned long int &x2 );
                real Melt2( real T );
                real Melt( real T );
                void copy( real* sours,real *dest,unsigned long int num );
                void norm( real *X,unsigned long int mNum );
                real RPA( real beta,real nuw );
                real dHS( real beta,real ro );

                inline real fI1_6( real nuw )
                {
                    return (1.+(A4+(A5+A6*nuw)*nuw)*nuw)/
                        ((1.+(AA1+(AA2+AA3*nuw)*nuw)*nuw)*3.);
                }

                inline real fI1_12( real nuw )
                {
                    return (1.+(B4+(B5+B6*nuw)*nuw)*nuw)/
                        ((1.+(BB1+(BB2+BB3*nuw)*nuw)*nuw)*9.);
                }

                inline real fa0( real nuw ,real nu1w2 )
                {
                    return (A00 + A01*nuw)/nu1w2;
                }

                inline real fa1( real nuw ,real nu1w3 )
                {
                    return (A10+(A11+A12*nuw)*nuw)/nu1w3;
                }

                inline real fa2( real nuw ,real nu1w4 )
                {
                    return ((A21+(A22+A23*nuw)*nuw)*nuw)/nu1w4;
                }

                inline real fa3( real nuw ,real nu1w5 )
                {
                    return ((A31+(A32+(A33+A34*nuw)*nuw)*nuw)*nuw)/nu1w5;
                }

                real DIntegral( real T, real ro, unsigned long int IType ); // not used
                real LIntegral( real T, real ro, unsigned long int IType ); // not used
                real KIntegral( real T, real ro, unsigned long int IType ); // not used
                real K23_13( real T, real ro );
                real J6LJ( real T,real ro );
                real FDipPair( real T,real ro,real m2 ); // not used
                real UWCANum( real T,real ro );
                real ZWCANum( real T,real ro );

                real FWCA( real T,real ro );
		real FTOTALMIX( real T_Real,real ro_Real,EOSPARAM* param );
		real UTOTALMIX( real T_Real,real ro_Real,EOSPARAM* param ); // not used
		real ZTOTALMIX( real T_Real,real ro_Real,EOSPARAM* param );
		real PTOTALMIX( real T_Real,real ro_Real,EOSPARAM* param );
		real ROTOTALMIX( real P,real TT,EOSPARAM* param );

		real PRESSURE( real *X, real *param, unsigned long int NN, real ro, real T ); // not used
		real DENSITY( real *X,real *param, unsigned long int NN ,real Pbar, real T );
		long int CGActivCoefRhoT( real *X,real *param, real *act, unsigned long int NN,
				real ro, real T ); // not used

		long int CGResidualFunctPure( const real *coeff, real ro, real T );

		long int CGActivCoefPT(real *X,real *param,real *act, unsigned long int NN,
				real Pbar, real T, real &roro );

	public:

        /// Constructor
		TCGFcalc( long int NCmp, real Pp, real Tkp );
                TCGFcalc( SolutionData *sd, real *aphWGT, real *arX );

        /// Destructor
		~TCGFcalc();

        /// Calculates of pure species properties (pure fugacities)
		long int PureSpecies( );

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        ///<  calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// CGofPureGases, calculates fugacity for 1 species at (X=1)
        long int CGcalcFugPure( real Tmin, real *Cemp, real *FugProps );  // called from DCthermo
		long int CGFugacityPT( real *EoSparam, real *EoSparPT, real &Fugacity,
				real &Volume, real P, real T, real &roro );

        /// Calculates departure functions
		long int CGResidualFunct( real *X, real *param, real *param1, unsigned long int NN,
				real ro, real T );

		real GetDELTA( void )
		{
			return DELTA;
        }
};



// -------------------------------------------------------------------------------------
/// Peng-Robinson-Stryjek-Vera (PRSV) model for fluid mixtures.
/// References: Stryjek and Vera (1986)
/// (c) TW July 2006
class TPRSVcalc: public TSolMod

{
	private:

        real PhVol;   ///< phase volume in cm3
                real *Pparc;  ///< DC partial pressures (pure fugacities)
                    // real *aGEX;   // Increments to molar G0 values of DCs from pure fugacities
                    // real *aVol;   // DC molar volumes, cm3/mol [L]

		// main work arrays
        real (*Eosparm)[6];   ///< EoS parameters
        real (*Pureparm)[4];  ///< Parameters a, b, da/dT, d2a/dT2 for cubic EoS
        real (*Fugpure)[6];   ///< fugacity parameters of pure gas species
        real (*Fugci)[4];     ///< fugacity parameters of species in the mixture

        real **a;		///< arrays of generic parameters
		real **b;
        real **KK;     ///< binary interaction parameter
        real **dKK;    ///< derivative of interaction parameter
        real **d2KK;   ///< second derivative
        real **AA;     ///< binary a terms in the mixture



		// internal functions
		void alloc_internal();
		void free_internal();
		long int AB( real Tcrit, real Pcrit, real omg, real k1, real k2, real k3,
				real &apure, real &bpure, real &da, real &d2a );
		long int FugacityPT( long int i, real *EoSparam );
		long int FugacityPure( long int j ); // Calculates the fugacity of pure species
		long int Cardano( real a2, real a1, real a0, real &z1, real &z2, real &z3 );
		long int MixParam( real &amix, real &bmix );
		long int FugacityMix( real amix, real bmix, real &fugmix, real &zmix, real &vmix );
		long int FugacitySpec( real *fugpure );
		long int ResidualFunct( real *fugpure );
		long int MixingWaals();
		long int MixingConst();
		long int MixingTemp();

	public:

        /// Constructor
		TPRSVcalc( long int NCmp, real Pp, real Tkp );
                TPRSVcalc( SolutionData *sd );

        /// Destructor
		~TPRSVcalc();

        /// Calculates pure species properties (pure fugacities)
		long int PureSpecies();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// Calculates pure species properties (called from DCthermo)
        long int PRSVCalcFugPure( real Tmin, real *Cpg, real *FugProps );

};



// -------------------------------------------------------------------------------------
/// Soave-Redlich-Kwong (SRK) model for fluid mixtures.
/// References: Soave (1972); Soave (1993)
/// (c) TW December 2008
class TSRKcalc: public TSolMod

{
	private:

        real PhVol;   ///< phase volume in cm3
                real *Pparc;  ///< DC partial pressures (pure fugacities)
                    // real *aGEX;   // Increments to molar G0 values of DCs from pure fugacities
                    // real *aVol;   // DC molar volumes, cm3/mol [L]

		// main work arrays
        real (*Eosparm)[4];   ///< EoS parameters
        real (*Pureparm)[4];  ///< Parameters a, b, da/dT, d2a/dT2 for cubic EoS
        real (*Fugpure)[6];   ///< Fugacity parameters of pure gas species
        real (*Fugci)[4];     ///< Fugacity parameters of species in the mixture

        real **a;		///< arrays of generic parameters
		real **b;
        real **KK;    ///< binary interaction parameter
        real **dKK;   ///< derivative of interaction parameter
        real **d2KK;  ///< second derivative
        real **AA;    ///< binary a terms in the mixture

		// internal functions
		void alloc_internal();
		void free_internal();
		long int AB( real Tcrit, real Pcrit, real omg, real N,
				real &apure, real &bpure, real &da, real &d2a );
		long int FugacityPT( long int i, real *EoSparam );
		long int FugacityPure( long int j ); // Calculates the fugacity of pure species
		long int Cardano( real a2, real a1, real a0, real &z1, real &z2, real &z3 );
		long int MixParam( real &amix, real &bmix );
		long int FugacityMix( real amix, real bmix, real &fugmix, real &zmix, real &vmix );
		long int FugacitySpec( real *fugpure );
		long int ResidualFunct( real *fugpure );
		long int MixingWaals();
		long int MixingConst();
		long int MixingTemp();

	public:

        /// Constructor
		TSRKcalc( long int NCmp, real Pp, real Tkp );
                TSRKcalc( SolutionData *sd );

        /// Destructor
		~TSRKcalc();

        /// Calculates pure species properties (pure fugacities)
		long int PureSpecies();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// Calculates pure species properties (called from DCthermo)
        long int SRKCalcFugPure( real Tmin, real *Cpg, real *FugProps );

};



// -------------------------------------------------------------------------------------
/// Peng-Robinson (PR78) model for fluid mixtures.
/// References: Peng and Robinson (1976); Peng and Robinson (1978)
/// (c) TW July 2009
class TPR78calc: public TSolMod

{
	private:

        real PhVol;   ///< phase volume in cm3
                real *Pparc;  ///< DC partial pressures (pure fugacities)
                    // real *aGEX;   // Increments to molar G0 values of DCs from pure fugacities
                    // real *aVol;   // DC molar volumes, cm3/mol [L]

		// main work arrays
        real (*Eosparm)[4];   ///< EoS parameters
        real (*Pureparm)[4];  ///< Parameters a, b, da/dT, d2a/dT2 for cubic EoS
        real (*Fugpure)[6];   ///< Fugacity parameters of pure gas species
        real (*Fugci)[4];     ///< Fugacity parameters of species in the mixture

        real **a;		///< arrays of generic parameters
		real **b;
        real **KK;    ///< binary interaction parameter
        real **dKK;   ///< derivative of interaction parameter
        real **d2KK;  ///< second derivative
        real **AA;    ///< binary a terms in the mixture

		// internal functions
		void alloc_internal();
		void free_internal();
		long int AB( real Tcrit, real Pcrit, real omg, real N,
				real &apure, real &bpure, real &da, real &d2a );
		long int FugacityPT( long int i, real *EoSparam );
		long int FugacityPure( long int j ); // Calculates the fugacity of pure species
		long int Cardano( real a2, real a1, real a0, real &z1, real &z2, real &z3 );
		long int MixParam( real &amix, real &bmix );
		long int FugacityMix( real amix, real bmix, real &fugmix, real &zmix, real &vmix );
		long int FugacitySpec( real *fugpure );
		long int ResidualFunct( real *fugpure );
		long int MixingWaals();
		long int MixingConst();
		long int MixingTemp();

	public:

        /// Constructor
		TPR78calc( long int NCmp, real Pp, real Tkp );
                TPR78calc( SolutionData *sd );

        /// Destructor
		~TPR78calc();

        /// Calculates pure species properties (pure fugacities)
		long int PureSpecies();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// Calculates pure species properties (called from DCthermo)
        long int PR78CalcFugPure( real Tmin, real *Cpg, real *FugProps );

};



// -------------------------------------------------------------------------------------
/// Compensated Redlich-Kwong (CORK) model for fluid mixtures.
/// References: Holland and Powell (1991)
/// (c) TW May 2010
class TCORKcalc: public TSolMod

{
        private:

                // constants and external parameters
                real RR;    ///< gas constant in kbar
                real Pkb;   ///< pressure in kbar
                real PhVol;   ///< phase volume in cm3
                real *Pparc;  ///< DC partial pressures (pure fugacities)
                    // real *aGEX;   // Increments to molar G0 values of DCs from pure fugacities
                    // real *aVol;   // DC molar volumes, cm3/mol [L]

                // internal work data
                real (*Eosparm)[2];   ///< EoS parameters
                real (*Fugpure)[6];   ///< Fugacity parameters of pure gas species
                real (*Fugci)[4];     ///< Fugacity parameters of species in the mixture
                real (*Rho)[11];      ///< density parameters
                char *EosCode;    ///< identifier of EoS routine
                real *phi;
                real *dphi;
                real *d2phi;
                real *dphip;
                real **A;         ///< binary interaction parameters
                real **W;         ///< volume scaled interaction parameters (derivatives)
                real **B;
                real **dB;
                real **d2B;
                real **dBp;

                // internal functions
                void alloc_internal();
                void free_internal();
                long int FugacityPT( long int j, real *EoSparam );
                long int FugacityH2O( long int j );
                long int FugacityCO2( long int j );
                long int FugacityCorresponding( long int j );
                long int VolumeFugacity( long int phState, real pp, real p0, real a, real b, real c,
                        real d, real e, real &vol, real &fc );
                long int Cardano( real cb, real cc, real cd, real &v1, real &v2, real &v3 );
                long int FugacityMix();
                long int ResidualFunct();

        public:

                /// Constructor
                TCORKcalc( long int NCmp, real Pp, real Tkp, char Eos_Code );
                TCORKcalc( SolutionData *sd );

                /// Destructor
                ~TCORKcalc();

                /// Calculates pure species properties (pure fugacities)
                long int PureSpecies();

                /// Calculates T,P corrected interaction parameters
                long int PTparam();

                /// Calculates activity coefficients
                long int MixMod();

                /// Calculates excess properties
                long int ExcessProp( real *Zex );

                /// Calculates ideal mixing properties
                long int IdealProp( real *Zid );

                /// Calculates pure species properties (called from DCthermo)
                long int CORKCalcFugPure( real Tmin, /*float*/ real *Cpg, real *FugProps );

};



// -------------------------------------------------------------------------------------
/// Sterner-Pitzer (STP) model for fluid mixtures.
/// References: Sterner and Pitzer (1994)
/// (c) TW December 2010
class TSTPcalc: public TSolMod

{
        private:

                // constants and external parameters
                real RC, RR, TMIN, TMAX, PMIN, PMAX;
                real Pkbar, Pkb, Pmpa;
                real PhVol;   ///< phase volume in cm3
                real *Pparc;  ///< DC partial pressures (pure fugacities)

                // internal work data
                char *EosCode;
                real *Tc;
                real *Pc;
                real *Psat;
                real *Rhol;
                real *Rhov;
                real *Mw;
                real *Phi;
                real *dPhiD;
                real *dPhiDD;
                real *dPhiT;
                real *dPhiTT;
                real *dPhiDT;
                real *dPhiDDD;
                real *dPhiDDT;
                real *dPhiDTT;
                real (*Fugpure)[7];
                real (*Rho)[11];
                real *phi;
                real *dphi;
                real *d2phi;
                real *dphip;
                real *lng;
                real **cfh;
                real **cfc;
                real **A;
                real **W;
                real **B;
                real **dB;
                real **d2B;
                real **dBp;

                // internal functions
                void alloc_internal();
                void free_internal();
                void set_internal();
                long int UpdateTauP();
                long int FugacityPT( long int j, real *EoSparam );
                long int FugacityH2O( long int j );
                long int FugacityCO2( long int j );
                long int FugacityCorresponding( long int j );
                long int DensityGuess( long int j, real &Delguess );
                long int PsatH2O( long int j );
                long int PsatCO2( long int j );
                long int Pressure( real rho, real &p, real &dpdrho, real **cf );
                long int Helmholtz( long int j, real rho, real **cf );
                long int ResidualFunct();

        public:

                /// Constructor
                TSTPcalc ( long int NCmp, real Pp, real Tkp, char Eos_Code );
                TSTPcalc ( SolutionData *sd );

                /// Destructor
                ~TSTPcalc();

                /// Calculates pure species properties (pure fugacities)
                long int PureSpecies();

                /// Calculates T,P corrected interaction parameters
                long int PTparam();

                /// Calculates activity coefficients
                long int MixMod();

                /// Calculates excess properties
                long int ExcessProp( real *Zex );

                /// Calculates ideal mixing properties
                long int IdealProp( real *Zid );

                /// Calculates pure species properties (called from DCthermo)
                long int STPCalcFugPure( real Tmin, real *Cpg, real *FugProps );

};



// -------------------------------------------------------------------------------------
/// Van Laar model for solid solutions.
/// References:  Holland and Powell (2003)
/// (c) TW March 2007

class TVanLaar: public TSolMod
{
	private:
		real *Wu;
		real *Ws;
		real *Wv;
        real *Wpt;   ///< Interaction coeffs at P-T
        real *Phi;   ///< Mixing terms
        real *PsVol; ///< End member volume parameters

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TVanLaar( SolutionData *sd );

        /// Destructor
		~TVanLaar();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates of activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Regular model for multicomponent solid solutions.
/// References: Holland and Powell (1993)
/// (c) TW March 2007
class TRegular: public TSolMod
{
	private:
		real *Wu;
		real *Ws;
		real *Wv;
        real *Wpt;   ///< Interaction coeffs at P-T

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TRegular( SolutionData *sd );

        /// Destructor
		~TRegular();

        /// Calculates T,P corrected interaction parameters
        long int PTparam( );

        /// Calculates of activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Redlich-Kister model for multicomponent solid solutions.
/// References: Hillert (1998)
/// (c) TW March 2007
class TRedlichKister: public TSolMod
{
	private:
		real (*Lu)[4];
		real (*Ls)[4];
		real (*Lcp)[4];
		real (*Lv)[4];
		real (*Lpt)[4];

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TRedlichKister( SolutionData *sd );

        /// Destructor
		~TRedlichKister();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Non-random two liquid (NRTL) model for liquid solutions.
/// References: Renon and Prausnitz (1968), Prausnitz et al. (1997)
/// (c) TW June 2008
class TNRTL: public TSolMod
{
	private:
		real **Tau;
		real **dTau;
		real **d2Tau;
		real **Alp;
		real **dAlp;
		real **d2Alp;
		real **G;
		real **dG;
		real **d2G;

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TNRTL( SolutionData *sd );

        /// Destructor
		~TNRTL();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Wilson model for liquid solutions.
/// References: Prausnitz et al. (1997)
/// (c) TW June 2008
class TWilson: public TSolMod
{
	private:
		real **Lam;
		real **dLam;
		real **d2Lam;

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TWilson( SolutionData *sd );

        /// Destructor
		~TWilson();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Berman model for multi-component sublattice solid solutions.
/// To be extended with reciprocal terms.
/// References: Wood and Nicholls (1978); Berman and Brown (1993)
/// (c) DK/TW December 2010, June 2011
class TBerman: public TSolMod
{
        private:
                long int NrcR;   ///< max. possible number of reciprocal reactions (allocated)
                long int Nrc;    ///< number of reciprocal reactions (actual)
                long int *NmoS;  ///< number of different moieties (in end members) on each sublattice
            long int ***XrcM;  ///< Table of indexes of end members, sublattices and moieties involved in
                               ///< reciprocal reactions [NrecR][4][2], two left and two right side.
                               ///< for each of 4 reaction components: j, mark, // s1, m1, s2, m2.

                real *Wu;    ///< Interaction parameter coefficients a
                real *Ws;    ///< Interaction parameter coefficients b (f(T))
                real *Wv;    ///< Interaction parameter coefficients c (f(P))
                real *Wpt;   ///< Interaction parameters corrected at P-T of interest
            real **fjs;      ///< array of site activity coefficients for end members [NComp][NSub]

                real *Grc;  ///< standard molar reciprocal energies (constant)
                real *oGf;   ///< molar Gibbs energies of end-member compounds
                real *G0f;   ///< standard molar Gibbs energies of end members (constant)
            real *DGrc; ///< molar effects of reciprocal reactions [NrecR]
            real *pyp;  ///< Products of site fractions for end members (CEF mod.) [NComp]
//            real *pyn;  // Products of site fractions for sites not in the end member [NComp]
                void alloc_internal();
                void free_internal();
                long int choose( const long int n, const long int k );
                bool CheckThisReciprocalReaction( const long int r, const long int j, long int *xm );
                long int CollectReciprocalReactions2( void );
//                long int CollectReciprocalReactions3( void );
                long int FindIdenticalSublatticeRow(const long int si, const long int ji, const long jp,
                                                    const long int jb, const long int je );
                                              //      long int &nsx, long int *sx, long int *mx );
                long int ExcessPart();
                               ///< Arrays for ideal conf part must exist in base TSolMod instance
                real PYproduct( const long int j );
                long int em_which(const long int s, const long int m , const long int jb, const long int je);
                long int em_howmany( long int s, long int m );
                real ysigma( const long int j, const long int s );
                real KronDelta( const long int j, const long int s, const long int m );
                real dGref_dysigma(const long int l, const long int s, const long int ex_j );
                real dGref_dysm( const long int s, const long m, const long int ex_j );
                real RefFrameTerm( const long int j, real G_ref );
                long int ReciprocalPart();   ///< Calculation of reciprocal contributions to activity coefficients

        public:

                /// Constructor
                TBerman( SolutionData *sd, real *G0 );

                /// Destructor
                ~TBerman();

                /// Calculates T,P corrected interaction parameters
                long int PTparam();

                /// Calculates activity coefficients
                long int MixMod();

                /// Calculates excess properties
                long int ExcessProp( real *Zex );

                /// Calculates ideal mixing properties
                long int IdealProp( real *Zid );

};


// -------------------------------------------------------------------------------------
/// CEF (Calphad) model for multi-component sublattice solid solutions with reciprocal terms
/// References: Sundman & Agren (1981); Lucas et al. (2006); Hillert (1998).
/// (c) DK/SN since August 2014 (still to change the excess Gibbs energy terms).
class TCEFmod: public TSolMod
{
        private:
                long int *NmoS;  ///< number of different moieties (in end members) on each sublattic

                real *Wu;    ///< Interaction parameter coefficients a
                real *Ws;    ///< Interaction parameter coefficients b (f(T))
                real *Wc;    ///< Interaction parameter coefficients b (f(TlnT))
                real *Wv;    ///< Interaction parameter coefficients c (f(P))
                real *Wpt;   ///< Interaction parameters corrected at P-T of interest
                real **fjs;      ///< array of site activity coefficients for end members [NComp][NSub]

                real *Grc;  ///< standard molar reciprocal energies (constant)
                real *oGf;   ///< molar Gibbs energies of end-member compounds
                real *G0f;   ///< standard molar Gibbs energies of end members (constant)
                real *pyp;  ///< Products of site fractions for end members (CEF mod.) [NComp]
//            real *pyn;  // Products of site fractions for sites not in the end member [NComp]
                void alloc_internal();
                void free_internal();
                long int ExcessPart();
                               ///< Arrays for ideal conf part must exist in base TSolMod instance
                real PYproduct( const long int j );
                long int em_which(const long int s, const long int m , const long int jb, const long int je);
                long int em_howmany( long int s, long int m );
                real ysm( const long int j, const long int s );
                real KronDelta( const long int j, const long int s, const long int m );
                real dGref_dysigma(const long int l, const long int s );
                real dGref_dysm(const long int s, const long m );
                real dGm_dysm(const long int s, const long m ); // added by Nichenko
                real RefFrameTerm( const long int j, real G_ref );
                long int ReciprocalPart();   ///< Calculation of reciprocal contributions to activity coefficients

                long int IdealMixing(); // NSergii: added by Nichenko to rewrite the ideal part contribution
                long int CalcSiteFractions(); // NSergii:
                real dGrefdnNum(const long int i); // NSergii:
                real dGidmixdnNum(const long int i); // NSergii:
                real dGexcdnNum(const long int i); // NSergii:
                real Gmix(); // NSergii:
                real Gexc();
                real Gref();
                real Gidmix();
        public:

                /// Constructor
                TCEFmod( SolutionData *sd, real *G0 );

                /// Destructor
                ~TCEFmod();

                /// Calculates T,P corrected interaction parameters
                long int PTparam();

                /// Calculates activity coefficients
                long int MixMod();

                /// Calculates excess properties
                long int ExcessProp( real *Zex );

                /// Calculates ideal mixing properties
                long int IdealProp( real *Zid );

};


// -------------------------------------------------------------------------------------
/// SIT model reimplementation for aqueous electrolyte solutions.
/// (c) DK/TW June 2009
class TSIT: public TSolMod
{
	private:

        // data objects copied from MULTI
        real *z;    ///< std::vector of species charges (for aqueous models)
        real *m;    ///< std::vector of species molalities (for aqueous models)
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties

        // internal work objects
        real I;	///< ionic strength
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real **E0;  ///< interaction parameter
		real **E1;
		real **dE0;
		real **dE1;
		real **d2E0;
		real **d2E1;

        // internal functions
		real IonicStrength();
		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TSIT( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TSIT();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// Calculation of internal tables (at each GEM iteration)
		long int PTparam();

};



// -------------------------------------------------------------------------------------
/// Pitzer model, Harvie-Moller-Weare (HMW) version, with explicit temperature dependence.
/// References:
/// (c) SD/FH February 2009
class TPitzer: public TSolMod
{

private:
    long int Nc;	 ///< Number of cations
    long int Na;     ///< Number of anions
    long int Nn;     ///< Number of neutral species
    long int Ns;     ///< Total number of aqueous species (without H2O); index of H2O in aq phase
                     ///< Conversion of species indexes between aq phase and Pitzer parameter tables
    long int *xcx;   ///< list of indexes of Nc cations in aqueous phase
    long int *xax;   ///< list of indexes of Na anions in aq phase
    long int *xnx;   ///< list of indexes of Nn neutral species in aq phase
    real *aZ;    ///< Vector of species charges (for aqueous models)
	real *zc;
	real *za;
    real *aM;    ///< Vector of species molality (for aqueous models)
	real *mc;
	real *ma;
	real *mn;
    real *RhoW;  ///< water density properties
    real *EpsW;  ///< water dielectrical properties

        real Aphi, dAphidT, d2AphidT2, dAphidP;  ///< Computing A-Factor
    real I;  ///< Ionic Strength
    real Is;  ///< Ionic Strength square root
    real Ffac; ///< F-Factor
    real Zfac; ///< Z-Term

    // Input parameter arrays
            //for Gex and activity coefficient calculation
    real **Bet0;     ///< Beta0 table for cation-anion interactions [Nc][Na]
    real **Bet1;	   ///< Beta1 table for cation-anion interactions [Nc][Na]
    real **Bet2;	   ///< Beta2 table for cation-anion interactions [Nc][Na]
    real **Cphi;     ///< Cphi  table for cation-anion interactions [Nc][Na]
    real **Lam;      ///< Lam table for neutral-cation interactions [Nn][Nc]
    real **Lam1;     ///< Lam1 table for neutral-anion interactions [Nn][Na]
    real **Theta;    ///< Theta table for cation-cation interactions [Nc][Nc]
    real **Theta1;   ///< Theta1 table for anion-anion interactions [Na][Na]
    real ***Psi;     ///< Psi array for cation-cation-anion interactions [Nc][Nc][Na]
    real ***Psi1;    ///< Psi1 array for anion-anion-cation interactions [Na][Na][Nc]
    real ***Zeta;    ///< Zeta array for neutral-cation-anion interactions [Nn][Nc][Na]


            // Work parameter arrays
            // real *B1;      /// B' table for cation-anion interactions corrected for IS [Nc][Na]
            // real *B2;      /// B table for cation-anion interactions corrected for IS [Nc][Na]
            // real *B3;      /// B_phi table for cation-anion interactions corrected for IS [Nc][Na]
            // real *Phi1;    /// Phi' table for anion-anion interactions corrected for IS [Na][Na]
            // real *Phi2;    /// Phi table for cation-cation interactions corrected for IS [Nc][Nc]
            // real *Phi3;    /// PhiPhi table for anion-anion interactions corrected for IS [Na][Na]
            // real *C;       /// C table for cation-anion interactions corrected for charge [Nc][Na]
            // real *Etheta;  /// Etheta table for cation-cation interactions [Nc][Nc]
            // real *Ethetap; /// Etheta' table for anion-anion interactions [Na][Na]
            // real bk[21];   /// work space
            // real dk[21];   /// work space

    /// McInnes parameter array and gamma values
	real *McI_PT_array;
	real *GammaMcI;

	enum eTableType
	{
		bet0_ = -10, bet1_ = -11, bet2_ = -12, Cphi_ = -20, Lam_ = -30, Lam1_ = -31,
		Theta_ = -40,  Theta1_ = -41, Psi_ = -50, Psi1_ = -51, Zeta_ = -60
	};

    // internal setup
	void calcSizes();
	void alloc_internal();
	void free_internal();

    /// build conversion of species indexes between aq phase and Pitzer parameter tables
	void setIndexes();
	real setvalue(long int ii, int Gex_or_Sex);

    // internal calculations
    /// Calculation of Etheta and Ethetap values
	void Ecalc( real z, real z1, real I, real DH_term,
					real& Etheta, real& Ethetap );
	inline long int getN() const
	{
		return Nc+Na+Nn;
	}

	real Z_Term( );
	real IonicStr( real& I );
	void getAlp( long int c, long int a, real& alp, real& alp1 );
	real get_g( real x_alp );
	real get_gp( real x_alp );
	real G_ex_par5( long int ii );
	real G_ex_par8( long int ii );
	real S_ex_par5( long int ii );
	real S_ex_par8( long int ii );
	real CP_ex_par5( long int ii );
	real CP_ex_par8( long int ii );
	real F_Factor( real DH_term );
	real lnGammaN( long int N );
	real lnGammaM( long int M, real DH_term );
	real lnGammaX( long int X, real DH_term );
	real lnGammaH2O( real DH_term );

    /// Calc std::vector of interaction parameters corrected to T,P of interest
	void PTcalc( int Gex_or_Sex );

    /// Calculation KCl activity coefficients for McInnes scaling
	real McInnes_KCl();

	inline long int getIc( long int jj )
    {
		for( long int ic=0; ic<Nc; ic++ )
			if( xcx[ic] == jj )
				return ic;
		return -1;
    }

	inline long int getIa( long int jj )
    {
		for( long int ia=0; ia<Na; ia++ )
			if( xax[ia] == jj )
				return ia;
		return -1;
    }

	inline long int getIn( long int jj )
    {
		for( long int in=0; in<Nn; in++ )
			if( xnx[in] == jj )
				return in;
		return -1;
    }

    inline real p_sum( real* arr, long int *xx, long int Narr )
    {
		real sum_ =0.;
		for( long int i=0; i<Narr; i++ )
          sum_ += arr[xx[i]];
		return sum_;
    }

public:

    /// Constructor
        TPitzer( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

    /// Destructor
	~TPitzer();

    /// Calculation of T,P corrected interaction parameters
	long int PTparam();


    long int MixMod();

    /// Calculates activity coefficients
	long int Pitzer_calc_Gamma();
	long int Pitzer_McInnes_KCl();

    /// Calculates excess properties
    long int ExcessProp( real *Zex );

    /// Calculates ideal mixing properties
	long int IdealProp( real *Zid );

	void Pitzer_test_out( const char *path, real Y );

};



// -------------------------------------------------------------------------------------
/// Extended universal quasi-chemical (EUNIQUAC) model for aqueous electrolyte solutions.
/// References: Nicolaisen et al. (1993), Thomsen et al. (1996), Thomsen (2005)
/// (c) TW/FH May 2009
class TEUNIQUAC: public TSolMod
{
	private:

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties

        // internal work objects
        real *R;   ///< volume parameter
        real *Q;   ///< surface parameter
		real *Phi;
		real *Theta;
        real **U;   ///< interaction energies
        real **dU;   ///< first derivative
        real **d2U;   ///< second derivative
		real **Psi;
		real **dPsi;
		real **d2Psi;
        real IS;  ///< ionic strength
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)

        ///< objects needed for debugging output
		real gammaDH[200];
		real gammaC[200];
		real gammaR[200];

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();

	public:

        /// Constructor
                TEUNIQUAC( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TEUNIQUAC();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

		void Euniquac_test_out( const char *path );

};

// -------------------------------------------------------------------------------------
// ELVIS activity model for aqueous electrolyte solutions
// (c) FFH Aug 2011

class TELVIS: public TSolMod
{
        private:
                // data objects copied from MULTI
                real *z;   							// species charges
                real *m;   							// species molalities
                real *RhoW;  							// water density properties
                real *EpsW;  							// water dielectrical properties
                real aDH;								// averaged ion size term in DH term
                real A, dAdT, d2AdT2, dAdP;  			// A term of DH equation (and derivatives)
                real B, dBdT, d2BdT2, dBdP;  			// B term of DH equation (and derivatives)

                real **beta0;
                real **beta1;
                real **alpha;

                real **coord;         // coordinaiton number parameter

                real **RA;
                real **RC;
                real **QA;
                real **QC;

                real CN;

#ifdef ELVIS_SPEED
#define ELVIS_NCOMP 10
                // internal work objects
                real R[ELVIS_NCOMP];
                real Q[ELVIS_NCOMP];
                real Phi[ELVIS_NCOMP];
                real Theta[ELVIS_NCOMP];

                real EffRad[ELVIS_NCOMP];

                real dRdP[ELVIS_NCOMP];
                real dRdT[ELVIS_NCOMP];
                real d2RdT2[ELVIS_NCOMP];
                real dQdP[ELVIS_NCOMP];
                real dQdT[ELVIS_NCOMP];
                real d2QdT2[ELVIS_NCOMP];

                real WEps[ELVIS_NCOMP][ELVIS_NCOMP];
                real U[ELVIS_NCOMP][ELVIS_NCOMP];
                real dU[ELVIS_NCOMP][ELVIS_NCOMP];
                real d2U[ELVIS_NCOMP][ELVIS_NCOMP];
                real Psi[ELVIS_NCOMP][ELVIS_NCOMP];
                real dPsi[ELVIS_NCOMP][ELVIS_NCOMP];
                real d2Psi[ELVIS_NCOMP][ELVIS_NCOMP];
                real TR[ELVIS_NCOMP][4];

                real U[ELVIS_NCOMP][ELVIS_NCOMP];
                real dUdP[ELVIS_NCOMP][ELVIS_NCOMP];
                real dUdT[ELVIS_NCOMP][ELVIS_NCOMP];
                real d2UdT2[ELVIS_NCOMP][ELVIS_NCOMP];

                real ELVIS_lnGam_DH[ELVIS_NCOMP];
                real ELVIS_lnGam_Born[ELVIS_NCOMP];
                real ELVIS_OsmCoeff_DH[ELVIS_NCOMP];
                real ELVIS_lnGam_UNIQUAC[ELVIS_NCOMP];

#endif

#ifndef ELVIS_SPEED
                // internal work objects
                real *R;   							// volume parameter
                real *Q;   							// surface parameter
                real *Phi;
                real *Theta;
                real *EffRad; 						// effective ionic radii
                real **U;   							// interaction energies
                real **dU;   							// first derivative
                real **d2U;   						// second derivative
                real **Psi;
                real **dPsi;
                real **d2Psi;
                real **TR; 							// TR interpolation parameter array
                real **WEps;							// indices for electrolyte specific permittivity calculation

                real* dRdP;
                real* dRdT;
                real* d2RdT2;
                real* dQdP;
                real* dQdT;
                real* d2QdT2;

                real** dUdP;
                real** dUdT;
                real** d2UdT2;

                real* ELVIS_lnGam_DH;
                real* ELVIS_lnGam_Born;
                real* ELVIS_OsmCoeff_DH;
                real* ELVIS_lnGam_UNIQUAC;
#endif

                real IS;  							// ionic strength
                real molT;  							// total molality of aqueous species (except water solvent)
                real molZ;  							// total molality of charged species


                // objects needed for debugging output
                real gammaDH[200];
                real gammaBorn[200];
                real gammaQUAC[200];
                real gammaC[200];
                real gammaR[200];

                // internal functions
                void alloc_internal();
                void free_internal();

                long int IonicStrength();

                // activity coefficient contributions
                void ELVIS_DH(real* ELVIS_lnGam_DH, real* ELVIS_OsmCoeff_DH);
                void ELVIS_Born(real* ELVIS_lnGam_Born);
                void ELVIS_UNIQUAC(real* ELVIS_lnGam_UNIQUAC);

                // Osmotic coefficient
                real Int_OsmCoeff();
                void molfrac_update();
                real FinDiff( real m_j, int j  ); 	// Finite Difference of lnGam of electrolyte 'j' with respect to its molality 'm[j]';
                real CalcWaterAct();

                // Apparent molar volume
//                real FinDiffVol( real m_j, void* params ); 					// Finite differences of mean lnGam with respect to pressure

                real trapzd( const real lower_bound, const real upper_bound, int& n, long int& species, int select ); 	// from Numerical Recipes in C, 2nd Ed.
                real qsimp( const real lower_bound, const real upper_bound, long int& species, int select ); 			// from Numerical Recipes in C, 2nd Ed.


        public:
                // Constructor
                TELVIS( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

                // Destructor
                ~TELVIS();

                // calculates T,P corrected interaction parameters
                long int PTparam();

                // calculates activity coefficients and osmotic coefficient by
                // numerical Integration of Bjerrum Relation
                long int MixMod();
                long int CalcAct();

                // Compute apparent molar volume of electrolyte
                real App_molar_volume();

                // calculates excess properties
                long int ExcessProp( real *Zex );

                // calculates ideal mixing properties
                long int IdealProp( real *Zid );

                // plot debug results
                void TELVIS_test_out( const char *path, const real M ) const;

                // ELVIS_FIT: get lnGamma array
                void get_lnGamma( std::vector<real>& ln_gamma );

                real FinDiffVol( real m_j, int j ); 					// Finite differences of mean lnGam with respect to pressure

};


// -------------------------------------------------------------------------------------
/// Extended Debye-Hueckel (EDH) model for aqueous electrolyte solutions, Helgesons variant.
/// References: Helgeson et al. (1981); Oelkers and Helgeson (1990); Pokrovskii and Helgeson (1995; 1997a; 1997b)
/// (c) TW July 2009
class THelgeson: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< flag for water
        long int flagNeut;  ///< flag for neutral species
        long int flagElect;  ///< flag for selection of background electrolyte model

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties
        real *an;  ///< individual ion size-parameters
        real *bg;  ///< individual extended-term parameters
        real ac;  ///< common ion size parameters
        real bc;  ///< common extended-term parameter

        // internal work objects
        real ao, daodT, d2aodT2, daodP;  ///< ion-size parameter (TP corrected)
        real bgam, dbgdT, d2bgdT2, dbgdP;  ///< extended-term parameter (TP corrected)
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real molZ;  ///< total molality of charged species
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)
        real B, dBdT, d2BdT2, dBdP;  ///< B term of DH equation (and derivatives)
        real Gf, dGfdT, d2GfdT2, dGfdP;  ///< g function (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();
		long int BgammaTP();
		long int IonsizeTP();
		long int Gfunction();
		long int GShok2( real T, real P, real D, real beta,
				real alpha, real daldT, real &g, real &dgdP,
				real &dgdT, real &d2gdT2 );

	public:

        /// Constructor
                THelgeson( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~THelgeson();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

        /// Set function used in GEMSFITS for mixed electrolites DM 15.08.2014
        long int Set_Felect_bc (long int Flagelect, real Bc, real Ac);

};



// -------------------------------------------------------------------------------------
/// Extended Debye-Hueckel (EDH) model for aqueous electrolyte solutions, Davies variant.
/// References: Langmuir (1997)
/// (c) TW July 2009
class TDavies: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< flag for water
        long int flagNeut;  ///< flag for neutral species
        long int flagMol;  ///< flag for molality correction

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties

        // internal work objects
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();

	public:

        /// Constructor
                TDavies( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TDavies();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Debye-Hueckel (DH) limiting law for aqueous electrolyte solutions.
/// References: Langmuir (1997)
/// (c) TW July 2009
class TLimitingLaw: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< flag for water
        long int flagNeut;  ///< flag for neutral species

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties

        // internal work objects
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();

	public:

        /// Constructor
                TLimitingLaw( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TLimitingLaw();

        /// calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Two-term Debye-Hueckel (DH) model for aqueous electrolyte solutions.
/// References: Helgeson et al. (1981)
/// uses individual ion-size parameters, optionally individual salting-out coefficients
/// (c) TW July 2009
class TDebyeHueckel: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< flag for water
        long int flagNeut;  ///< flag for neutral species

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties
        real *an;  ///< individual ion size-parameters
        real *bg;  ///< individual extended-term parameters
        real ac;  ///< common ion size parameters
        real bc;  ///< common extended-term parameter

        // internal work objects
        real ao;  ///< average ion-size parameter
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)
        real B, dBdT, d2BdT2, dBdP;  ///< B term of DH equation (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();

	public:

        /// Constructor
                TDebyeHueckel( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TDebyeHueckel();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Extended Debye-Hueckel (EDH) model for aqueous electrolyte solutions, Karpovs variant.
/// References: Karpov et al. (1997); Helgeson et al. (1981); Oelkers and Helgeson (1990);
/// Pokrovskii and Helgeson (1995; 1997a; 1997b)
/// (c) TW July 2009
class TKarpov: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< flag for water
        long int flagNeut;  ///< flag for neutral species
        long int flagElect;  ///< flag for selection of background electrolyte model

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties
        real *an;  ///< individual ion size-parameters at T,P
        real *bg;  ///< individual extended-term parameters
        real ac;  ///< common ion size parameters
        real bc;  ///< common extended-term parameter

        // internal work objects
        real ao;  ///< average ion-size parameter
        real bgam, dbgdT, d2bgdT2, dbgdP;  ///< extended-term parameter (TP corrected)
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real molZ;  ///< total molality of charged species
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)
        real B, dBdT, d2BdT2, dBdP;  ///< B term of DH equation (and derivatives)
        real Gf, dGfdT, d2GfdT2, dGfdP;  ///< g function (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();
		long int BgammaTP();
		long int IonsizeTP();
		long int Gfunction();
		long int GShok2( real T, real P, real D, real beta,
				real alpha, real daldT, real &g, real &dgdP,
				real &dgdT, real &d2gdT2 );

	public:

        /// Constructor
                TKarpov( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TKarpov();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Extended Debye-Hueckel (EDH) model for aqueous electrolyte solutions, Shvarov variant.
/// References: Shvarov (2007); Oelkers and Helgeson (1990);
/// Pokrovskii and Helgeson (1995; 1997a; 1997b)
/// (c) TW July 2009
class TShvarov: public TSolMod
{
	private:

        // status flags copied from MULTI
        long int flagH2O;  ///< new flag for water
        long int flagNeut;  ///< new flag for neutral species
        long int flagElect;  ///< flag for selection of background electrolyte model

        // data objects copied from MULTI
        real *z;   ///< species charges
        real *m;   ///< species molalities
        real *RhoW;  ///< water density properties
        real *EpsW;  ///< water dielectrical properties
        real *bj;  ///< individual ion parameters
        real ac;  ///< common ion size parameters
        real bc;  ///< common extended-term parameter

        // internal work objects
        real ao, daodT, d2aodT2, daodP;  ///< ion-size parameter (TP corrected)
        real bgam, dbgdT, d2bgdT2, dbgdP;  ///< extended-term parameter (TP corrected)
        real *LnG;  ///< activity coefficient
        real *dLnGdT;  ///< derivatives
		real *d2LnGdT2;
		real *dLnGdP;
        real IS;  ///< ionic strength
        real molT;  ///< total molality of aqueous species (except water solvent)
        real A, dAdT, d2AdT2, dAdP;  ///< A term of DH equation (and derivatives)
        real B, dBdT, d2BdT2, dBdP;  ///< B term of DH equation (and derivatives)
        real Gf, dGfdT, d2GfdT2, dGfdP;  ///< g function (and derivatives)

        // internal functions
		void alloc_internal();
		void free_internal();
		long int IonicStrength();
		long int BgammaTP();
		long int IonsizeTP();
		long int Gfunction();
		long int GShok2( real T, real P, real D, real beta,
				real alpha, real daldT, real &g, real &dgdP,
				real &dgdT, real &d2gdT2 );

	public:

        /// Constructor
                TShvarov( SolutionData *sd, real *arM, real *arZ, real *dW, real *eW );

        /// Destructor
		~TShvarov();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Class for hardcoded models for solid solutions.
/// (c) TW January 2009
class TModOther: public TSolMod
{
	private:
        real PhVol;   ///< phase volume in cm3
                    // real *Pparc;  /// DC partial pressures/ pure fugacities, bar (Pc by default) [0:L-1]
                    // real *aGEX;   /// Increments to molar G0 values of DCs from pure fugacities or DQF terms, normalized [L]
                    // real *aVol;   /// DC molar volumes, cm3/mol [L]
        real *Gdqf;	///< DQF correction terms
		real *Hdqf;
		real *Sdqf;
		real *CPdqf;
		real *Vdqf;

		void alloc_internal();
		void free_internal();

	public:

        /// Constructor
                TModOther( SolutionData *sd, real *dW, real *eW );

        /// Destructor
		~TModOther();

        /// Calculates pure species properties (pure fugacities, DQF corrections)
		long int PureSpecies();

        /// Calculates T,P corrected interaction parameters
		long int PTparam();

        /// Calculates activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

                // functions for individual models (under construction)
                long int Amphibole1();
                long int Biotite1();
                long int Chlorite1();
                long int Clinopyroxene1();
                long int Feldspar1();
                long int Feldspar2();
                long int Garnet1();
                long int Muscovite1();
                long int Orthopyroxene1();
                long int Staurolite1();
                long int Talc();

};



// -------------------------------------------------------------------------------------
/// Ternary Margules (regular) model for solid solutions.
/// References: Anderson and Crerar (1993); Anderson (2006)
/// (c) TW/DK June 2009
class TMargules: public TSolMod
{
	private:

		real WU12, WS12, WV12, WG12;
		real WU13, WS13, WV13, WG13;
		real WU23, WS23, WV23, WG23;
		real WU123, WS123, WV123, WG123;

	public:

        /// Constructor
                TMargules( SolutionData *sd );

        /// Destructor
		~TMargules();

        /// Calculates T,P corrected interaction parameters
		long int PTparam( );

        /// Calculates of activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Binary Margules (subregular) model for solid solutions.
/// References: Anderson and Crerar (1993); Anderson (2006)
/// (c) TW/DK June 2009, DQF part added by DK on April 5, 2015
class TSubregular: public TSolMod
{
	private:

		real WU12, WS12, WV12, WG12;
		real WU21, WS21, WV21, WG21;
        real DQFX1, DQFG1, DQFS1, DQFV1, DQF1;
        real DQFX2, DQFG2, DQFS2, DQFV2, DQF2;

	public:

        /// Constructor
                TSubregular( SolutionData *sd );

        /// Destructor
		~TSubregular();

        /// Calculates T,P corrected interaction parameters
		long int PTparam( );

        /// Calculates of activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};



// -------------------------------------------------------------------------------------
/// Binary Guggenheim (Redlich-Kister) model for solid solutions.
/// References: Anderson and Crerar (1993); Anderson (2006)
/// uses normalized (by RT) interaction parameters
/// (c) TW/DK June 2009
class TGuggenheim: public TSolMod
{
	private:

		real a0, a1, a2;

	public:

        /// Constructor
                TGuggenheim( SolutionData *sd );

        /// Destructor
		~TGuggenheim();

        /// Calculates T,P corrected interaction parameters
		long int PTparam( );

        /// Calculates of activity coefficients
		long int MixMod();

        /// Calculates excess properties
		long int ExcessProp( real *Zex );

        /// Calculates ideal mixing properties
		long int IdealProp( real *Zid );

};

#endif
}



/// _s_solmod_h
