// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// The formulas of this file come from s_solmod2.cpp of the GEMS3K code, (c) T. Wagner, D. Kulik, S. Dmitrieva, F. Hingerl,
// S. Churakov, A. Rysin and others, under the GNU Lesser General Public License version 3 or later.

#include "Methods/Fluids/FluidCGF.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Fluids/CGFpure.hpp>

namespace ThermoFun {

namespace {

/// The constants of the model.
const real DELTA = 0.00001;
const real R = R_CONSTANT;
constexpr int FIRSTSEED = 15;
constexpr int NPOINT = 5;
const real ROMIN = 1.E-2;



/// The parameters of the fluid at T.
struct PureFluid
{
    real sig, eps, m, a;  ///< the diameter, the depth of the well, the dipole moment and the polarizability
    real s3mix, emix;     ///< sig^3, and the energy that scales T
    const ThermoFun::cgf::Constants* k = nullptr;
};

auto pureFluidOf(const real* data, const ThermoFun::cgf::Constants& k) -> PureFluid
{
    PureFluid out{data[0], data[1], data[2], data[3], 0.0, 0.0, &k};
    real sig3 = out.sig;
    sig3 = sig3*sig3*sig3;
    out.s3mix = sig3;
    out.emix = (sig3*out.eps)/sig3;
    return out;
}

/// The state of one calculation.
struct Work
{
    real Tk, Pbar, R_CONST = R_CONSTANT;
    bool gems = false;  ///< the formulas of GEMS: S and H of the residual at the density, not at the pressure
    real Grs, Hrs, Srs, CPrs, Vrs;
    const ThermoFun::cgf::Constants* k = nullptr;
};


long int CGcalcFugPure(Work& w, real Tmin, real *Cemp, real *FugProps);
long int CGResidualFunctPure(Work& w, const real *coeff, real ro, real T);
long int CGFugacityPT(const ThermoFun::cgf::Constants& k, real *EoSparam, real *EoSparPT, real &Fugacity, real &Volume, real P, real T, real &roro);
real DENSITY(const PureFluid& fluid, real Pbar, real T);
real ROTOTALMIX(real P,real TT,const PureFluid& fluid);
void choose(real *pres, real P,unsigned long int &x1,unsigned long int &x2);
real ZTOTALMIX(real T_Real,real ro_Real,const PureFluid& fluid);
real FTOTALMIX(real T_Real,real ro_Real,const PureFluid& fluid);
real Melt2(real T);
real Melt(real T);
long int CGFugacityOfPure(const PureFluid& fluid, real &fugacity, real Pbar, real T, real &roro);

long int CGcalcFugPure(Work& w, real Tmin, real *Cemp, real *FugProps)
{
    long int retCode = 0;
    real T, P, Fugacity = 0.1, Volume = 0.0;
    real X[1] = {1.};
    real roro = 1.;  // added 21.06.2008 (TW)
    real Coeff[12];  // MAXEOSPARAM = 20;
    real Eos4parPT[4] = { 0.0, 0.0, 0.0, 0.0 },
        Eos4parPT1[4] = { 0.0, 0.0, 0.0, 0.0 } ;

    T = w.Tk;
    P = w.Pbar;

    for(long int ii=0; ii<12; ii++ )
        Coeff[ii] = Cemp[ii];

    // Calling CG EoS functions here
    if( (w.Tk >= Tmin) && (w.Tk < 1e4) && (w.Pbar >= 1e-5) && (w.Pbar < 1e5) )
    {
        retCode = CGFugacityPT(*w.k, Coeff, Eos4parPT, Fugacity, Volume, P, T, roro );
        FugProps[0] = Fugacity/w.Pbar;
        FugProps[1] = R_CONSTANT * w.Tk * log( Fugacity / P );
        FugProps[4] = Volume;
        CGResidualFunctPure(w, Coeff, roro, T );
        FugProps[2] = w.Hrs;
        FugProps[3] = w.Srs;
        return retCode;
    }

    else
    {
        for( int i=1; i<6; i++ )
            FugProps[i] = 0.;
        FugProps[0] = 1.;
        FugProps[4] = R_CONSTANT*w.Tk/w.Pbar;
        return -1;
    }
}

long int CGResidualFunctPure(Work& w, const real *coeff, real ro, real T)
{
    ThermoFun::cgf::PureFluidDerivatives o;
    ThermoFun::cgf::pureFluid(T, ro, coeff, *w.k, o);

    w.Srs = - ( T*o.FT + o.F ) * w.R_CONST;   // the residual at (T, V)
    w.Hrs = (o.F*w.Tk*w.R_CONST + w.Tk*w.Srs) + (1. + ro*o.Frho)*w.R_CONST*w.Tk;
    if (!w.gems)
    {
        // at (T, P), as G = RT ln(phi): S has R ln Z, and H has (Z - 1) RT, not Z RT. The residual Cp needs the second
        // derivatives of F, which CGFanalytic.hpp does not give: it stays 0
        const real Z = 1. + ro*o.Frho;
        w.Srs += w.R_CONST*log(Z);
        w.Hrs -= w.R_CONST*w.Tk;
    }
    w.Grs = w.Hrs - w.Tk*w.Srs;
    w.CPrs = 0.;
    w.Vrs = (1. + ro*o.Frho)*w.R_CONST*w.Tk/w.Pbar;

    return 0;
}

long int CGFugacityPT(const ThermoFun::cgf::Constants& k, real *EoSparam, real *EoSparPT, real &Fugacity, real &Volume, real P, real T, real &roro)
{
    long int iRet = 0;

    // modification to simplify CG database structure, 20.03.2007 (TW)
    EoSparPT[0] = EoSparam[0] + EoSparam[4]*exp(T*EoSparam[5]);
    EoSparPT[1] = EoSparam[1] + EoSparam[6]*exp(T*EoSparam[7]);
    EoSparPT[2] = EoSparam[2] + EoSparam[8]/(T+EoSparam[9]);
    EoSparPT[3] = EoSparam[3] + EoSparam[10]/(T+EoSparam[11]);
    const PureFluid fluid = pureFluidOf(EoSparPT, k);

    // returns density
    real fugacity = 0.;
    CGFugacityOfPure(fluid, fugacity, P, T, roro);  // changed, 21.06.2008 (TW)
    if( roro < 0.  )
    {
        return -1;
    };

    Fugacity = fugacity;
    roro = DENSITY(fluid, P, T);

    if( roro < 0 )
    {  // error - density could not be calculated
        iRet = -2;
        roro = 1.0;
    }

    Volume = 0.1/roro;  // in J/bar
    // roro = ro;  // added, 21.06.2008 (TW)

    return iRet;
}

real DENSITY(const PureFluid& fluid, real Pbar, real T)
{
    real P = Pbar * 0.1;
    real ro = ROTOTALMIX(P, T, fluid);
    if( ro < 0. )
        funError("Churakov and Gottschalk fluid model", "The density cannot be found at this T, P.", __LINE__, __FILE__);
    return ro;
}

real ROTOTALMIX(real P,real TT,const PureFluid& fluid)
{
     unsigned long int i;
     real T /*,ro*/;
     real fact, fact0, romax, dro, roarr[FIRSTSEED];
     real Ptmp[FIRSTSEED], ro0, ro1, rotest, PP0, PP1 /* ,Ptest */;
     real a,b;
     real inttofloat;
     real f[4],x[4],ff,dens[5],pres[5];
     unsigned long int x1=0L,x2=0L;
// real ptmp;

     T = TT/fluid.emix;
     fact0 = 1./(fluid.s3mix*fluid.k->NA);
     fact = R*TT*fact0;

     romax = Melt(T);
     inttofloat = FIRSTSEED-1;
     dro = (romax-ROMIN)/inttofloat;
     roarr[0] = ROMIN;
     roarr[1] = 2.*ROMIN;

     for ( i=2; i<FIRSTSEED; i++)
     {
       inttofloat = i;
       roarr[i] = ROMIN+inttofloat*dro;
     }

     for ( i=0; i<FIRSTSEED; i++)
     {
      Ptmp[i] = ZTOTALMIX(TT,roarr[i]*fact0, fluid);
      Ptmp[i] *= roarr[i] * fact;
      if ( Ptmp[i] > P )
      {
        break;
      }
     }

     if ( i==0 )  // Uses aproximation of ideal gas
     {
            return P/(R*TT);
     }

     // additional high pressure inteval
     if ( i==FIRSTSEED )
     {

     // roarr[0]=romax-0.0001;
     roarr[0] = roarr[FIRSTSEED-1];
     Ptmp[0] = Ptmp[FIRSTSEED-1];

     romax = Melt2(T);
     inttofloat = FIRSTSEED-1;
     dro = (romax-ROMIN)/inttofloat;
     for ( i=1; i<FIRSTSEED; i++)
     {
       inttofloat = i;
       roarr[i] = ROMIN+inttofloat*dro;
     }

     for ( i=1; i<FIRSTSEED; i++)
     {
      Ptmp[i] = ZTOTALMIX(TT,roarr[i]*fact0, fluid)*roarr[i]*fact;
      if ( Ptmp[i]>P )
      {
        break;
      }
     }

     if ( i==FIRSTSEED || i==0 )
     {
         printf( "Input pressure is too high!\n" );
            // exit(1);
         return (-1.0);
     }
     }

     ro0 = roarr[i-1];
     ro1 = roarr[i];
     PP0 = Ptmp[i-1];
     PP1 = Ptmp[i];
     i = 0;

   while ( i++<20 )
   {
     // Start interp
     ff = ro0;
     dens[0] = ro0;
     dens[1] = ro1;
     pres[0] = PP0;
     pres[1] = PP1;

     // first order
     x[0] = P-pres[0];
     f[0] = (dens[1]-dens[0])/(pres[1]-pres[0]);
     ff += f[0]*x[0];

     // second order
     dens[2] = ff;
     pres[2] = ZTOTALMIX(TT,ff*fact0, fluid)*ff*fact;

     if ( fabs(pres[2]-P)<1E-5 )
     {
       return ff*fact0;
     }

     x[1] = x[0]*(P-pres[1]);
     f[1] = (dens[2]-dens[1])/(pres[2]-pres[1]);

     f[0] = (f[1]-f[0])/(pres[2]-pres[0]);
     ff += f[0]*x[1];

     // third order
     dens[3] = ff;
     pres[3] = ZTOTALMIX(TT,ff*fact0, fluid)*ff*fact;
     if ( fabs(pres[3]-P)<1E-6 )
     {
      return ff*fact0;
     }
     x[2] = x[1]*(P-pres[2]);
     f[2] = (dens[3]-dens[2])/(pres[3]-pres[2]);
     f[1] = (f[2]-f[1])/(pres[3]-pres[1]);
     f[0] = (f[1]-f[0])/(pres[3]-pres[0]);
     ff += f[0]*x[2];
     dens[4] = ff;
     pres[4] = ZTOTALMIX(TT,ff*fact0, fluid)*ff*fact;
     if ( fabs(pres[4]-P)<1e-6 )
     {
      return ff*fact0;
     }

     choose(pres,P,x1,x2);

     ro0 = dens[x1];
     ro1 = dens[x2];
     PP0 = pres[x1];
     PP1 = pres[x2];

      if ( fabs((ro1-ro0))<0.001 )
      {
          a = (PP1-PP0)/(ro1-ro0);
          b = PP1-a*ro1;
          rotest = (P-b)/a;
          return rotest*(fact0);
      }
   }
        //return 10.;
         /// bad result

          a = (PP1-PP0)/(ro1-ro0);
          b = PP1-a*ro1;
          rotest = (P-b)/a;
          return rotest*(fact0);

 }

void choose(real *pres, real P,unsigned long int &x1,unsigned long int &x2)
{
  unsigned long int i;
  real deltam = -10000000.,tmp;
  real deltap = 10000000.;

  for ( i=0; i<NPOINT; i++ )
  {
    tmp = P-pres[i];

    if ( tmp>0. )
    {
       if ( tmp<deltap )
       {
        deltap=tmp;
        x1 = i;
       }
    }
    else
    {
       if ( tmp>deltam )
       {
        deltam=tmp;
        x2 = i;
       }
    }
  }
     return ;
 }

real ZTOTALMIX(real T_Real,real ro_Real,const PureFluid& fluid)
{
    // analytical derivative Z = 1 + ro dF/dro (was a finite difference, relative step DELTA)
    ThermoFun::cgf::PureFluidDerivatives o;
    ThermoFun::cgf::pureFluid(T_Real, ro_Real, fluid.sig, fluid.eps, fluid.m, fluid.a, *fluid.k, o);
    return 1. + ro_Real*o.Frho;
}

real FTOTALMIX(real T_Real,real ro_Real,const PureFluid& fluid)
{
    ThermoFun::cgf::PureFluidDerivatives o;
    ThermoFun::cgf::pureFluid(T_Real, ro_Real, fluid.sig, fluid.eps, fluid.m, fluid.a, *fluid.k, o);
    return o.F;
}

real Melt2(real T)
{
    return T*0.+3.;
 }

real Melt(real T)
{

    return T*0.+.9;

 }


/// The fugacity of the pure fluid, MPa. A negative density: the pressure is too low, no corrections are made.
long int CGFugacityOfPure(const PureFluid& fluid, real &fugacity, real Pbar, real T, real &roro)
{
    real P = Pbar/10.;

    real ro = ROTOTALMIX(P, T, fluid);

    if( ro < 0.0 )  // Too low pressure, no corrections will be done
        return ( -1 );

    real Z = P/(R*T*ro);
    real F0 = FTOTALMIX(T, ro, fluid);

    real fideal = log(R*T*ro/0.1);
    real ax = Z - 1.+fideal;

    // a pure fluid: d(nF)/dn = F
    if ( F0 < 100. )
        fugacity = exp(ax+F0);
    else
        fugacity = 0.;

    roro = ro;  // added, 21.06.2008 (TW)

    return 0;  // changed, 21.06.2008 (TW)
}

} // namespace

auto thermoPropertiesFluidCGF(real TK, real Pbar, const FluidInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    double TClow = subst.lower_T;
    real CPg[12] = {}; // CGcalcFugPure reads 12 coefficients
    for (unsigned int i = 0; i < 12 && i < subst.parameters.size(); i++)
    {
        CPg[i] = subst.parameters[i];
    }

    const ThermoFun::cgf::Constants constants(subst.constants);
    Work work;
    work.k = &constants;
    work.Tk = TK;
    work.Pbar = Pbar;
    work.gems = subst.gems;
    CGcalcFugPure(work, TClow, CPg, FugProps);

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // back correction
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}

}
