//-------------------------------------------------------------------
// $Id: s_solmod.cpp 724 2012-10-02 14:25:25Z kulik $
//
/// \file s_solmod.cpp
/// Implementation of the TSolMod base class
//
// Copyright (c) 2007-2012  T.Wagner, D.Kulik, S.Dmitrieva
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GEMS4K code. If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------

#include <cmath>
#include "s_solmod_.h"
#include "Common/Exception.h"

namespace solmod
{

void SolutionData::set_def()
{
    NSpecies = 0;
    NParams = 0;
    NPcoefs = 0;
    MaxOrder = 0;
    NPperDC = 0;
    NSublat = 0;
    NMoiet = 0;
    NDQFpDC = 0;
    Mod_Code = 0;
    Mix_Code = 0;
    DC_Codes = 0;
    TP_Code = 0;
    arIPx = 0;
    arIPc = 0;
    arDCc = 0;
    arMoiSN = 0;
    arSitFr = 0;
    arSitFj = 0;
    arGEX = 0;
    DQFc = 0;
    arPparc = 0;
    arWx = 0;
    arlnGam = 0;
    arlnDQFt = 0;
    arlnRcpt = 0;
    arlnExet = 0;
    arlnCnft = 0;
    arVol = 0;
    aphVOL = 0;
    T_k = 0;
    P_bar = 0;
}


//=============================================================================================
// TSolMod base class for multicomponent solid, liquid, fluid and aqueous mixtures
// (c) DK/TW 2006 (version 1)
// (c) DK/TW November 2010 (version 2)
//=============================================================================================

/// generic constructor (new)
TSolMod::TSolMod( SolutionData *sd ):
        ModCode(sd->Mod_Code), MixCode(sd->Mix_Code), NComp(sd->NSpecies),  NPar(sd->NParams),
        NPcoef(sd->NPcoefs), MaxOrd(sd->MaxOrder),  NP_DC(sd->NPperDC), /*NPTP_DC(NPTPperDC),*/
        NSub(sd->NSublat), NMoi(sd->NMoiet), R_CONST(8.31451), Tk(sd->T_k), Pbar(sd->P_bar)
{
//   NlPh = sd->NlPhs;
//    NlPc = sd->NlPhC;
    NDQFpc = sd->NDQFpDC;
//    NrcPpc = sd->NrcPpDC;
//   lPhcf = sd->lPhc;
    DQFcf = sd->DQFc;  // read-only
//    rcpcf = sd->rcpc;  // read-only
//    if(rcpcf == NULL) NrcPpc = 0;
    //PhLin = sd->arPhLin;
//    PhLin = new long int[NlPh][2];
//    for (long int i=0; i<NlPh; i++)
//    {
//        PhLin[i][0] = sd->arPhLin[2*i];
//        PhLin[i][1] = sd->arPhLin[2*i+1];
//    }

    aSitFj = sd->arSitFj;
    // pointer assignments
    aIPx = sd->arIPx;   // Direct access to index list and parameter coeff arrays!
    aIPc = sd->arIPc;
    aIP = new double[NPar];
    aGEX = sd->arGEX;   // Reciprocal energies, Darken terms, pure fugacities of DC (corr. to TP)
    aPparc = sd->arPparc; // Partial pressures NComp
    aMoiSN = sd->arMoiSN; // End member moiety- site multiplicity number tables NComp x NSub x NMoi
    aSitFR = sd->arSitFr; // Tables of sublattice site fractions for moieties NSub x NMoi
    aDCc = sd->arDCc;
    DC_Codes = sd->DC_Codes;
    x = sd->arWx;
    phVOL = sd->aphVOL;
    aVol = sd->arVol;
    lnGamma = sd->arlnGam;
    lnGamConf = sd->arlnCnft;  // new double[NComp];
    lnGamRecip = sd->arlnRcpt; // new double[NComp];
    lnGamEx = sd->arlnExet;    // new double[NComp];
    lnGamDQF = sd->arlnDQFt;   // new double[NComp];
   // Arrays for lnGamma components - should we zero off?
    for (long int i=0; i<NComp; i++)
    {
       lnGamConf[i] = 0.0;
       lnGamRecip[i] = 0.0;
       lnGamEx[i] = 0.0;
       lnGamDQF[i] = 0.0;
    }
    // initialize phase properties
    Gex = 0.0; Hex = 0.0; Sex = 0.0; CPex = 0.0; Vex = 0.0; Aex = 0.0; Uex = 0.0;
    Gid = 0.0; Hid = 0.0; Sid = 0.0; CPid = 0.0; Vid = 0.0; Aid = 0.0; Uid = 0.0;
    Gdq = 0.0; Hdq = 0.0; Sdq = 0.0; CPdq = 0.0; Vdq = 0.0; Adq = 0.0; Udq = 0.0;
    Grs = 0.0; Hrs = 0.0; Srs = 0.0; CPrs = 0.0; Vrs = 0.0; Ars = 0.0; Urs = 0.0;
    alloc_multisite();
    init_multisite();
}

/// Get internal data  for read/write internal structure to files
void TSolMod::getSolutionData( SolutionData *sd )
{
    sd->Mod_Code = ModCode;
    sd->Mix_Code= MixCode;
    sd->NSpecies= NComp;
    sd->NParams= NPar;
    sd->NPcoefs= NPcoef;
    sd->MaxOrder= MaxOrd;
    sd->NPperDC= NP_DC;
    sd->NSublat= NSub;
    sd->NMoiet= NMoi;
    sd->NDQFpDC = NDQFpc;

    sd->arIPx = aIPx;
    sd->arIPc = aIPc;
    sd->arDCc = aDCc;
    sd->arMoiSN = aMoiSN;
    sd->arSitFr = aSitFR;
    sd->arSitFj = aSitFj;
    sd->DQFc = DQFcf;
    sd->arPparc = aPparc;

    sd->arlnDQFt = lnGamDQF;
    sd->arlnCnft = lnGamConf;
    sd->arlnRcpt = lnGamRecip;
    sd->arlnExet = lnGamEx;

    // from MULTI
    sd->DC_Codes = DC_Codes;
    sd->arWx = x;
    sd->arGEX = aGEX;
    sd->arlnGam = lnGamma;
    sd->aphVOL = phVOL;
    sd->arVol = aVol;
}


/// generic constructor (new) for calling from DComp/DCthermo calculations
TSolMod::TSolMod( long int NSpecies, char Mod_Code,  double T_k, double P_bar ):
        ModCode(Mod_Code), MixCode(0), NComp(NSpecies),  NPar(0),
        NPcoef(0), MaxOrd(0),  NP_DC(0), /*NPTP_DC(NPTPperDC),*/
        NSub(0), NMoi(0), R_CONST(8.31451), Tk(T_k), Pbar(P_bar)
{
    // pointer assignments (blocked direct access)
    aIPx = 0;   // Direct access to index list and parameter coeff arrays!
    aIPc = 0;
    aIP = 0;
    aGEX = 0;   // Reciprocal energies, Darken terms, pure fugacities of DC (corr. to TP)
    aPparc = 0; // Partial pressures NComp
    aMoiSN = 0; // End member moiety- site multiplicity number tables NComp x NSub x NMoi
    aSitFR = 0; // Tables of sublattice site fractions for moieties NSub x NMoi
    aSitFj =0;
    DQFcf =0;
    aDCc = 0;
    DC_Codes = 0;
    x = 0;
    phVOL = 0;
    aVol = 0;
    lnGamma = 0;
    lnGamConf = 0;
    lnGamRecip = 0;
    lnGamEx = 0;
    lnGamDQF = 0;

    // initialize phase properties
    Gex = 0.0; Hex = 0.0; Sex = 0.0; CPex = 0.0; Vex = 0.0; Aex = 0.0; Uex = 0.0;
    Gid = 0.0; Hid = 0.0; Sid = 0.0; CPid = 0.0; Vid = 0.0; Aid = 0.0; Uid = 0.0;
    Gdq = 0.0; Hdq = 0.0; Sdq = 0.0; CPdq = 0.0; Vdq = 0.0; Adq = 0.0; Udq = 0.0;
    Grs = 0.0; Hrs = 0.0; Srs = 0.0; CPrs = 0.0; Vrs = 0.0; Ars = 0.0; Urs = 0.0;
}


bool TSolMod::testSizes( SolutionData *sd )
{
  return( ( ModCode == sd->Mod_Code) && (NComp == sd->NSpecies) && ( NPar == sd->NParams) &&
          ( NPcoef == sd->NPcoefs) && (MaxOrd == sd->MaxOrder) &&  ( NP_DC == sd->NPperDC) &&
          ( NSub == sd->NSublat ) && ( NMoi == sd->NMoiet ) && (MixCode == sd->Mix_Code) );
}


TSolMod::~TSolMod()
{
   if( aIP )        // Bugfix 07.12.2010 DK
      delete[] aIP;

//   delete[] lnGamConf;
//   delete[] lnGamRecip;
//   delete[] lnGamEx;
   free_multisite();
   free_sdata();
}


/// allocates memory for multisite data
void TSolMod::alloc_multisite()
{
   long int j, s;
   if( !NSub || !NMoi )
       return;   // This is not a multi-site model
   // work array allocation
   y = new double *[NSub];
   for( s=0; s<NSub; s++)
   {
        y[s] = new double[NMoi];
   }
   mn = new double **[NComp];

   for(j=0; j<NComp; j++)
   {
        mn[j]   = new double *[NSub];
        for(s=0; s<NSub; s++)
        {
            mn[j][s] = new double [NMoi];
        }
   }
   mns = new double[NSub];
}


/// returns 0 if Ok or this is not a multi-site model;
/// returns j >=1 - end member index for which the total site multiplicity is different
/// from that for the previous end member (this is an error)
long int TSolMod::init_multisite()
{
    long int j, s, m, k=0;
    if( !NSub || !NMoi )
        return 0;   // This is not a multi-site model

    for( s=0; s<NSub; s++)
        for( m=0; m<NMoi; m++)
            y[s][m] = 0.0;
    // copying multiplicity numbers
    for( j=0; j<NComp; j++)
        for( s=0; s<NSub; s++)
           for( m=0; m<NMoi; m++)
           {  // extracting multiplicity numbers
              mn[j][s][m] = aMoiSN[k];
              k++;
           }
    // calculation of total site multiplicity numbers
    double mnsj;
    for( s=0; s<NSub; s++)
    {
       for( j=0; j<NComp; j++)
       {
          mnsj = 0.;
          for( m=0; m<NMoi; m++ )
             mnsj += mn[j][s][m];     // eq 5.1-6
          if( !j )
             mns[s] = mnsj;  // use NormDoubleRound(mnsj, 6)?
          else {  // comparing with mns for previous end member
              if( fabs( mns[s] - mnsj ) > 1e-6 )  // bugfix 06.06.2011 DK
              { // error - inconsistent multiplicity number in different end members
                 return j; // returns the end member index
              }
//              mns[s] = mnsj;
          }
       }
    }
    return 0;
}

/// frees memory for sublattice data
void TSolMod::free_multisite()
{
    long int j, s;
    if( !NSub || !NMoi )
        return;   // This is not a multi-site model
    for( s=0; s<NSub; s++)
    {
           delete[]y[s];
    }
    delete[]y;

    for(j=0; j<NComp; j++)
    {
           for(s=0; s<NSub; s++)
           {
               delete[]mn[j][s];
           }
    }
    for(j=0; j<NComp; j++)
    {
           delete[]mn[j];
    }
    delete[]mn;
    delete[]mns;
}

/// frees memory for inital arrays
void TSolMod::free_sdata()
{
  if( aIPx ) delete[] aIPx;
  if( aIPc ) delete[] aIPc;
  if( aDCc ) delete[] aDCc;
  if( aMoiSN ) delete[] aMoiSN;
  if( aSitFR ) delete[] aSitFR;
  if( aSitFj ) delete[] aSitFj;
  if( aPparc ) delete[] aPparc;
  if( DQFcf ) delete[] DQFcf;
  if( lnGamDQF ) delete[] lnGamDQF;
  if( lnGamRecip ) delete[] lnGamRecip;
  if( lnGamEx ) delete[] lnGamEx;
  if( lnGamConf ) delete[] lnGamConf;
}


/// site fractions
void TSolMod::return_sitefr()
{
    long int s, m, k=0;
    if( !NSub || !NMoi )
        return;   // This is not a multi-site model

    for( s=0; s<NSub; s++)
        for( m=0; m<NMoi; m++)
        {   // returning site fractions into TMulti
            aSitFR[k] = y[s][m]; k++;
        }
}


/// site fractions
void TSolMod::retrieve_sitefr()
{
    long int s, m, k=0;
    if( !NSub || !NMoi )
        return;   // This is not a multi-site model

    for( s=0; s<NSub; s++)
        for( m=0; m<NMoi; m++)
        {   // returning site fractions into TMulti
           y[s][m] = aSitFR[k];  k++;
        }
}


/// updates P and T in TSolMod if those have changed
long int TSolMod::UpdatePT ( double T_k, double P_bar )
{
	  Tk = T_k;
	  Pbar = P_bar;
	  return 0;
}


/// gets phase name for specific built-in models (class TModOther)
void TSolMod::GetPhaseName( const char *PhName )
{
	 strncpy( PhaseName, PhName, MAXPHASENAME );
	 PhaseName[MAXPHASENAME] = 0;
}

/// Calculation of configurational terms for the ideal mixing (c) DK, TW Nov. 2010
/// Based upon the formalism of Price (1985)
/// Returns 0 if calculated o.k., or 1 if this is not a multi-site model
long int TSolMod::IdealMixing()
{
    long int j,s,m;

    if( !NSub || !NMoi )
    {
        for( j=0; j<NComp; j++)
             lnGamConf[j] = 0.;
        return 1;   // this is not a multi-site model - bailing out
    }

    double mnsxj, lnaconj;
    // calculation of site fractions
    for( s=0; s<NSub; s++)
    {
        for( m=0; m<NMoi; m++)
        {
            mnsxj = 0.;
            for( j=0; j<NComp; j++)
               mnsxj += mn[j][s][m] * x[j];
            // calculation of site fraction (eq 5.1-10)
            y[s][m] = mnsxj/mns[s];
        }
    }

    return_sitefr(); // sending site fractions back to TMulti

    // Calculation of the ideal activity cnf term and fictive activity coefficient
    // for each end member
    for( j=0; j<NComp; j++)
    {
        lnaconj = 0.0;        // Eq 5.1-8
        for( s=0; s<NSub; s++)
            for( m=0; m<NMoi; m++)
            {
              if( mn[j][s][m] && y[s][m] > 1e-32 ) // check threshold
                lnaconj += mn[j][s][m] * log( y[s][m] / mn[j][s][m] * mns[s] );
            }
        // Calculation of the fictive activity coefficient  eq 5.1-14
        lnGamConf[j] = 0.;
        if(x[j] > 1e-32 )  // Check threshold
            lnGamConf[j] = lnaconj - log(x[j]);
                // lnGamRecip[j] = 0.;
                // lnGamEx[j] =0.;
    }

    return 0;
}

/// configurational entropy
double TSolMod::ideal_conf_entropy()
{
    long int j,s,m;

    if( !NSub || !NMoi )
    {   // This is default (simple mixing) model
        double si, Sid;
        si = 0.0;
        for(j=0; j<NComp; j++)
        {
           if ( x[j] > 1.0e-32 )
              si += x[j]*log(x[j]);
        }
        Sid = (-1.)*R_CONST*si;
        return Sid;
    }

    // calculation of the multi-site configurational entropy
    retrieve_sitefr();
    double sic, sis, Sicnf;
    sic = 0.0;
    for(s=0; s<NSub; s++)   // Eq 1 from Hillert 2001 (J.Alloys Compounds 320, 161-176)
    {              // see also eq 5.36 in the book by Lucas et al. 2007 (page 95)
        sis = 0.0;
        for( m=0; m<NMoi; m++ )
        {   // contribution per one s-th sublattice
            if ( y[s][m] > 1.0e-32 )
                sis += y[s][m] * log( y[s][m] );
        }
        sic += mns[s] * sis;
    }
    Sicnf = (-1.)*R_CONST*sic;
    return Sicnf;
}

// access from node
void TSolMod::Set_aIPc( const std::vector<double> aIPc_ )
{
  long int rc;
  if( (long)aIPc_.size() != (NPar*NPcoef) )
  {
      ThermoFun::thfun_logger->critical(
        "\nTNode::Set_aIPc() error: std::vector aIPc does not match the dimensions specified in the GEMS4K IPM file (NPar*NPcoef) !!!!"
        "\n aIPc.size() = {}, NPar*NPcoef = {} bailing out now ... \n", aIPc_.size(), NPar*NPcoef);
      exit(1);
  }
  for ( rc=0; rc<(NPar*NPcoef); rc++ )
      aIPc[ rc ] = aIPc_[ rc ];		// pointer to list of indices of interaction param coeffs, NPar * MaxOrd
}

void TSolMod::Get_aIPc ( std::vector<double> &aIPc_ )
{
  aIPc_.clear();
  aIPc_.resize( (NPar*NPcoef) );
  long int i = 0;
  while (i<(NPar*NPcoef))
  {
    aIPc_[ i ]   = aIPc[i];		// pointer to list of indices of interaction param coeffs, NPar * MaxOrd
    i++;
  }
}

void TSolMod::Set_aDCc( const std::vector<double> aDCc_ )
{
  long int rc;
  if( (long)aDCc_.size() != (NComp*NP_DC) )
  {
      ThermoFun::thfun_logger->critical(
        "\nTNode::Set_aDCc() error: std::vector aDCc does not match the dimensions specified in the GEMS4K IPM file (NComp*NP_DC) !!!!"
        "\n aDCc.size() = = {}, NComp*NP_DC = {} bailing out now ... \n", aDCc_.size(), NComp*NP_DC);
      exit(1);
  }
  for ( rc=0; rc<(NComp*NP_DC); rc++ )
  {
      (aDCc[ rc ]) = aDCc_[ rc ];		// end-member param coeffs, NComp * NP_DC
  }
}

void TSolMod::Get_aDCc( std::vector<double> &aDCc_ )
{
  aDCc_.clear();
  aDCc_.resize( (NComp*NP_DC) );
  long int i = 0;
  while (i<(NComp*NP_DC))
  {
    aDCc_[ i ]   = aDCc[ i ];  // pointer to list of indices of interaction param coeffs, NPar * MaxOrd
    i++;
  }
}

void TSolMod::Get_aIPx( std::vector<long int> &aIPx_ )
{
  long int i=0;
  aIPx_.clear();
  aIPx_.resize( (NPar*MaxOrd) );
  while (i<(NPar*MaxOrd))
  {
    aIPx_[ i ]   = aIPx[i];		// pointer to list of indices of interaction param coeffs, NPar * MaxOrd
    i++;
  }
}

void TSolMod::Get_NPar_NPcoef_MaxOrd_NComp_NP_DC ( long int &NPar_, long int &NPcoef_,
               long int &MaxOrd_,  long int &NComp_, long int &NP_DC_ )
{
  NPar_   = NPar;
  NPcoef_ = NPcoef;
  MaxOrd_ = MaxOrd;
  NComp_  = NComp;
  NP_DC_  = NP_DC;
}

}

//--------------------- End of s_solmod.cpp ----------------------------------------
