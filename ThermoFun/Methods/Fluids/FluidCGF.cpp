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
const real PI_1 = 3.141592653589793120;
const real TWOPI = 6.283185307179586230;
const real PISIX = 0.523598775598298927;
const real TWOPOW1SIX = 1.12246204830937302;
const real DELTA = 0.00001;
const real DELTAMOLLIM = 0.0000001;
const real R = 8.31439;
const real NA = 0.6023;
const real P1 = 1.186892378996;
const real PP2 = -0.4721963005527;
const real P3 = 3.259515855283;
const real P4 = 3.055229342609;
const real P5 = 1.095409321023;
const real P6 = 1.282306659774E-2;
const real P7 = 9.55712461425E-2;
const real P8 = 13.67807693107;
const real P9 = 35.75464856619;
const real P10 = 16.04724381643;
const real AA1 = -0.120078459237;
const real AA2 = -.808712488307;
const real AA3 = .321543801337;
const real A4 = 1.16965477132;
const real A5 = -.410564939543;
const real A6 = -.516834310691;
const real BB1 = -2.18839961483;
const real BB2 = 1.59897428009;
const real BB3 = -.392578806128;
const real B4 = -.189396607904;
const real B5 = -.576898496254;
const real B6 = -0.0185167641359;
const real A00 = .9985937977069455;
const real A01 = .5079834224407451;
const real A10 = 1.021887697885469;
const real A11 = -5.136619463333883;
const real A12 = -5.196188074016755;
const real A21 = -6.049240839050804;
const real A22 = 18.67848155616692;
const real A23 = 20.10652684217768;
const real A31 = 9.896491419756988;
const real A32 = 14.6738380473899;
const real A33 = -77.44825116542995;
const real A34 = -4.82871082941229;
constexpr int FIRSTSEED = 15;
constexpr int NPOINT = 5;
const real ROMIN = 1.E-2;

real fI1_6( real nuw )
{
                    return (1.+(A4+(A5+A6*nuw)*nuw)*nuw)/
                        ((1.+(AA1+(AA2+AA3*nuw)*nuw)*nuw)*3.);
                }

real fI1_12( real nuw )
{
                    return (1.+(B4+(B5+B6*nuw)*nuw)*nuw)/
                        ((1.+(BB1+(BB2+BB3*nuw)*nuw)*nuw)*9.);
                }

real fa0( real nuw ,real nu1w2 )
{
                    return (A00 + A01*nuw)/nu1w2;
                }

real fa1( real nuw ,real nu1w3 )
{
                    return (A10+(A11+A12*nuw)*nuw)/nu1w3;
                }

real fa2( real nuw ,real nu1w4 )
{
                    return ((A21+(A22+A23*nuw)*nuw)*nuw)/nu1w4;
                }

real fa3( real nuw ,real nu1w5 )
{
                    return ((A31+(A32+(A33+A34*nuw)*nuw)*nuw)*nuw)/nu1w5;
                }


/// The parameters of the fluid, by species. Here there is always one.
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
                real SIG( long int i) { return sigpar[i]; }   ///< the diameter
                real MPAR( long int i){ return mpar[i]; }     ///< the dipole moment
                real M2R( long int i) { return m2par[i]; }
                real A( long int i)   { return apar[i]; }

        long int ParamMix( real *Xin);
};

void EOSPARAM::free()
{
    long int i;

    if ( NComp > 0)
    {
        for ( i=0;i<NComp;i++ )
            delete[]mixpar[i];
        delete[]mixpar;

        delete[]epspar;
        delete[]sig3par;
        delete[]XX;
        delete[]eps;
        delete[]eps05;
        delete[]sigpar;
        delete[]mpar;
        delete[]apar;
        delete[]aredpar;
        delete[]m2par;
        delete[]XX0;
        NComp = 0;
    }
}

void EOSPARAM::allocate()
{
    long int i;

    mixpar = new real*[NComp];
    for ( i=0; i<NComp; i++ )
        mixpar[i] = new real[NComp];

    epspar = new real[NComp];
    sig3par = new real[NComp];
    XX = new real[NComp];
    eps = new real[NComp];
    eps05 = new real[NComp];
    sigpar = new real[NComp];
    mpar = new real[NComp];
      apar = new real[NComp];
    aredpar = new real[NComp];
    m2par = new real[NComp];
    XX0 = new real[NComp];
}

void EOSPARAM::init( real *Xinp, real * data, long int nn )
{
    long int i,j;
    real tmp;

    if( nn != NComp )
    { // or error message
        free();
        NComp = nn;
        allocate();
    }

    for ( i=0;i<NComp;i++ )
    {
        XX0[i] = Xinp[i];

        sigpar[i] = data[i*4 ];
        eps[i] = data[i*4 + 1];
        mpar[i] = data[i*4 + 2];
        apar[i] = data[i*4 + 3];
    }
    for ( i=0; i<NComp; i++ )
    {
        tmp = sigpar[i];
        tmp = tmp*tmp*tmp;
        sig3par[i] = tmp;
        eps05[i] = sqrt(eps[i]);
        epspar[i] = tmp*eps[i];
        m2par[i] = mpar[i]*mpar[i]/(1.38048E-4);
        aredpar[i] = apar[i]/tmp;
    }

    // calculation of mixing properties
    for ( i=0; i<NComp-1; i++ )
    {
        for ( j=i+1; j<NComp; j++ )
        {
            tmp = (sigpar[i]+sigpar[j])*0.5;
            tmp = tmp*tmp*tmp;
            mixpar[i][j] = tmp;
            mixpar[j][i] = tmp*eps05[i]*eps05[j];
        }
    }
}

long int EOSPARAM::ParamMix( real *Xin )
  {
    long int j,i;
    real tmp,tmp1,tmp2;

    for ( i=0; i<NComp; i++ )
        XX[i] = Xin[i];

    emix = 0.;
    s3mix = 0.;
    for ( i=0; i<NComp-1; i++ )
    {
      for ( j=i+1; j<NComp; j++ )
      {
          tmp = XX[i]*XX[j];
          tmp2 = mixpar[j][i];  //eps
          tmp1 = mixpar[i][j];  //signa
          s3mix += tmp1*tmp;
          emix += tmp2*tmp;
      }
    }
    s3mix *= 2.;
    emix *= 2.;
    for ( i=0; i<NComp; i++ )
    {
          tmp = XX[i]*XX[i];

          s3mix += sig3par[i]*tmp;
          emix += epspar[i]*tmp;
    }
    emix = emix/s3mix;
    return NComp;
  }

/// The state of one calculation.
struct Work
{
    real Tk, Pbar, R_CONST = 8.31451;
    real Grs, Hrs, Srs, CPrs, Vrs;
    EOSPARAM* paar = nullptr;
    EOSPARAM* paar1 = nullptr;
    ~Work() { delete paar; delete paar1; }
};


long int CGcalcFugPure(Work& w, real Tmin, real *Cemp, real *FugProps);
long int CGResidualFunctPure(Work& w, const real *coeff, real ro, real T);
long int CGFugacityPT(Work& w, real *EoSparam, real *EoSparPT, real &Fugacity, real &Volume, real P, real T, real &roro);
real DENSITY(Work& w, real *X, real *param, unsigned long NN, real Pbar, real T);
real ROTOTALMIX(Work& w, real P,real TT,EOSPARAM* param);
void choose(Work& w, real *pres, real P,unsigned long int &x1,unsigned long int &x2);
real ZTOTALMIX(Work& w, real T_Real,real ro_Real,EOSPARAM* param);
real FTOTALMIX(Work& w, real T_Real,real ro_Real,EOSPARAM* param);
real J6LJ(Work& w, real T,real ro);
real UWCANum(Work& w, real T,real ro);
real ZWCANum(Work& w, real T,real ro);
real K23_13(Work& w, real T, real ro);
real FWCA(Work& w, real T,real ro);
real RPA(Work& w, real beta,real nuw);
real dHS(Work& w, real beta,real ro);
real Melt2(Work& w, real T);
real Melt(Work& w, real T);
void copy(Work& w, real* sours,real *dest,unsigned long int num);
void norm(Work& w, real *X,unsigned long int mNum);
long int CGActivCoefPT(Work& w, real *X,real *param, real *act, unsigned long int NN, real Pbar, real T, real &roro);

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
        retCode = CGFugacityPT(w, Coeff, Eos4parPT, Fugacity, Volume, P, T, roro );
        FugProps[0] = Fugacity/w.Pbar;
        FugProps[1] = 8.31451 * w.Tk * log( Fugacity / P );
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
        FugProps[4] = 8.31451*w.Tk/w.Pbar;
        return -1;
    }
}

long int CGResidualFunctPure(Work& w, const real *coeff, real ro, real T)
{
    ThermoFun::cgf::PureFluidDerivatives o;
    ThermoFun::cgf::pureFluid(T, ro, coeff, o);

    w.Srs = - ( T*o.FT + o.F ) * w.R_CONST;
    w.Hrs = (o.F*w.Tk*w.R_CONST + w.Tk*w.Srs) + (1. + ro*o.Frho)*w.R_CONST*w.Tk;
    w.Grs = w.Hrs - w.Tk*w.Srs;
    w.CPrs = 0.;
    w.Vrs = (1. + ro*o.Frho)*w.R_CONST*w.Tk/w.Pbar;

    return 0;
}

long int CGFugacityPT(Work& w, real *EoSparam, real *EoSparPT, real &Fugacity, real &Volume, real P, real T, real &roro)
{
    long int iRet = 0;
    // real ro;
    real X[1] = {1.};
    real FugPure[1];

    // modification to simplify CG database structure, 20.03.2007 (TW)
    EoSparPT[0] = EoSparam[0] + EoSparam[4]*exp(T*EoSparam[5]);
    EoSparPT[1] = EoSparam[1] + EoSparam[6]*exp(T*EoSparam[7]);
    EoSparPT[2] = EoSparam[2] + EoSparam[8]/(T+EoSparam[9]);
    EoSparPT[3] = EoSparam[3] + EoSparam[10]/(T+EoSparam[11]);

    // returns density
    CGActivCoefPT(w, X, EoSparPT, FugPure, 1, P, T, roro );  // changed, 21.06.2008 (TW)
    if( roro < 0.  )
    {
        return -1;
    };

    Fugacity = FugPure[0];
    roro = DENSITY(w, X, EoSparPT, 1, P, T );

    if( roro < 0 )
    {  // error - density could not be calculated
        iRet = -2;
        roro = 1.0;
    }

    Volume = 0.1/roro;  // in J/bar
    // roro = ro;  // added, 21.06.2008 (TW)

    return iRet;
}

real DENSITY(Work& w, real *X, real *param, unsigned long NN, real Pbar, real T)
{
    real P = Pbar * 0.1;
    real *xtmp;
    real ro;

    xtmp = new real [NN];
    if( !w.paar1 )
        w.paar1 = new EOSPARAM(X,param,NN);
    else
        w.paar1->init( X, param, NN );

    norm(w, w.paar1->XX0,w.paar1->NCmp());
    copy(w, w.paar1->XX0,xtmp,w.paar1->NCmp());
    w.paar1->ParamMix(xtmp);
    ro = ROTOTALMIX(w, P,T,w.paar1);

    delete [] xtmp;
    if( ro < 0. )
        funError("Churakov and Gottschalk fluid model", "The density cannot be found at this T, P.", __LINE__, __FILE__);
    return ro;
}

real ROTOTALMIX(Work& w, real P,real TT,EOSPARAM* param)
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

     T = TT/param->EMIX();
     fact0 = 1./(param->S3MIX()*NA);
     fact = R*TT*fact0;

     romax = Melt(w, T);
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
      Ptmp[i] = ZTOTALMIX(w, TT,roarr[i]*fact0,param);
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

     romax = Melt2(w, T);
     inttofloat = FIRSTSEED-1;
     dro = (romax-ROMIN)/inttofloat;
     for ( i=1; i<FIRSTSEED; i++)
     {
       inttofloat = i;
       roarr[i] = ROMIN+inttofloat*dro;
     }

     for ( i=1; i<FIRSTSEED; i++)
     {
      Ptmp[i] = ZTOTALMIX(w, TT,roarr[i]*fact0,param)*roarr[i]*fact;
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
     pres[2] = ZTOTALMIX(w, TT,ff*fact0,param)*ff*fact;

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
     pres[3] = ZTOTALMIX(w, TT,ff*fact0,param)*ff*fact;
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
     pres[4] = ZTOTALMIX(w, TT,ff*fact0,param)*ff*fact;
     if ( fabs(pres[4]-P)<1e-6 )
     {
      return ff*fact0;
     }

     choose(w, pres,P,x1,x2);

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

void choose(Work& w, real *pres, real P,unsigned long int &x1,unsigned long int &x2)
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

real ZTOTALMIX(Work& w, real T_Real,real ro_Real,EOSPARAM* param)
{
  if ( param->NCmp()==1 )
  {    // a pure fluid: analytical derivative Z = 1 + ro dF/dro (was a finite difference, relative step DELTA)
    ThermoFun::cgf::PureFluidDerivatives o;
    ThermoFun::cgf::pureFluid(T_Real, ro_Real, param->SIG(0), param->EPS(0), param->MPAR(0), param->A(0), o);
    return 1. + ro_Real*o.Frho;
  }
  real delta = DELTA;
  real a0,a1;
  a1 = FTOTALMIX(w, T_Real,ro_Real*(1.+delta),param);
  a0 = FTOTALMIX(w, T_Real,ro_Real,param);

  return 1.+(a1-a0)/delta;
 }

real FTOTALMIX(Work& w, real T_Real,real ro_Real,EOSPARAM* param)
{
    if ( param->NCmp()==1 )
    {    // a pure fluid: analytical free energy
        ThermoFun::cgf::PureFluidDerivatives o;
        ThermoFun::cgf::pureFluid(T_Real, ro_Real, param->SIG(0), param->EPS(0), param->MPAR(0), param->A(0), o);
        return o.F;
    }
    real FF,A0,A2,A3,AP,A1;
    // unsigned iall,inopol;
    real emix,s3mix,rotmp,T2R;
    real Jind,Jdp;
    long int /*itmp,jtmp,ktmp,*/ i,j,k;
    real s3tmp,mtmp,IK /*,atmp*/;
    real imtmp,jmtmp,iatmp,jatmp;
    real m2i,m2j,m2k;
    real s3tmpij,s3tmpik,s3tmpjk;
    real IKtmpij,IKtmpik,IKtmpjk;

    // iall=param.inonzero();
    // inopol=param.inonpolar();
    emix = param->EMIX();
    s3mix = param->S3MIX();

      rotmp = NA*ro_Real;
      T2R = T_Real*T_Real;

      A0 = FWCA(w, T_Real/emix,s3mix*rotmp);
     // if ( inopol< iall )
      {
        // dipole part
        A2 = 0.;
        for ( i=0; i<param->NCmp()-1; i++ )
        {
          for ( j=i+1; j<param->NCmp(); j++ )
          {
            s3tmp = param->MIXS3(i,j);
            Jdp = J6LJ(w, T_Real*s3tmp/param->MIXES3(i,j) , s3tmp*rotmp);
            A2 += param->M2R(i)*param->M2R(j)*Jdp*
                           param->X(i)*param->X(j)/s3tmp;
          }
        }
          A2 *= 2.;
          for ( i=0; i<param->NCmp(); i++ )
          {
            // itmp=param->ind(i);
            mtmp = param->M2R(i);
            s3tmp = param->SIG3(i);
            Jdp = J6LJ(w, T_Real/param->EPS(i),s3tmp*rotmp);
            A2 += mtmp*mtmp*Jdp*param->X(i)*param->X(i)/s3tmp;
          }

         A2 = -A2*TWOPI*rotmp/(3.*T2R);
         // A2 done

         if ( A2!=0. )
         {

          A3 = 0.;
          for ( i=0; i<param->NCmp(); i++ )
          {
            // itmp=param->ind(i);
            m2i = param->M2R(i);

            for ( j=0; j<param->NCmp(); j++  )
            {
             // jtmp=param->ind(j);
              m2j = param->M2R(j);

              s3tmpij = param->MIXS3(i,j);
              IKtmpij = K23_13(w, T_Real*s3tmpij/param->MIXES3(i,j),
                                                    s3tmpij*rotmp);
              for ( k=0; k<param->NCmp(); k++  )
              {
               // ktmp=param->ind(k);
               m2k = param->M2R(k);

               s3tmpik = param->MIXS3(i,k);
               s3tmpjk = param->MIXS3(j,k);

               IKtmpik = K23_13(w, T_Real*s3tmpik/param->MIXES3(i,k),
                                                    s3tmpik*rotmp);
               IKtmpjk = K23_13(w, T_Real*s3tmpjk/param->MIXES3(j,k),
                                                    s3tmpjk*rotmp);

               IK = IKtmpij*IKtmpik*IKtmpjk;
               A3 += m2i*m2j*m2k*IK*pow(s3tmpij*s3tmpik*s3tmpjk,-1./3.)*
               param->X(i)*param->X(j)*param->X(k);
              }
            }
          }
            A3 = A3*32.*sqrt(14.*PI_1/5.)*
                  rotmp*rotmp*PI_1*PI_1*PI_1/(135.*T_Real*T2R);
            AP = A2/(1. - A3/A2);
         }
         else AP = 0.;

        // induced interaction
        A1 = 0.;
        for ( i=0; i<param->NCmp(); i++ )
        {
         // itmp=param->ind(i);
          iatmp = param->A(i);
          imtmp = param->M2R(i);
          for ( j=0; j<param->NCmp(); j++ )
          {
            // jtmp=param->ind(j);
            jatmp = param->A(j);
            jmtmp = param->M2R(j);

            s3tmp = param->MIXS3(i,j);
            Jind = J6LJ(w, T_Real*s3tmp/param->MIXES3(i,j),s3tmp*rotmp);

           A1 += (iatmp*jmtmp + jatmp*imtmp)
                  *Jind*param->X(i)*param->X(j)/s3tmp;
          }
        }
        A1 = -A1*TWOPI*rotmp/T_Real;
// A1=-A1*FOURPI*rotmp/T_Real;
// A1=0.;

      }  // end of polar contribution

     FF = A0 + A1 + AP;
     //printf("%g %g %g %g %g",A0,A1,A2,A3,AP);
     //exit(1) ;

    return FF;
  }

real J6LJ(Work& w, real T,real ro)
{
    real kappa,Z,U,beta,F;
    beta = 1./T;
    Z = ZWCANum(w, T,ro);
    kappa = -16.*PI_1*ro*beta;
    U = UWCANum(w, T,ro);
    F = (4.*beta*U-Z+1.)/kappa;
    return F;
}

real UWCANum(Work& w, real T,real ro)
{
    // analytical derivative of the free energy: U = dF/dbeta (was a finite difference, relative step DELTA)
    const real beta = 1./T;
    return ThermoFun::cgf::wcaAt(beta, ro).Fb;
}

real ZWCANum(Work& w, real T,real ro)
{
    // analytical derivative of the free energy: Z = 1 + ro dF/dro (was a finite difference, relative step DELTA)
    const real beta = 1./T;
    const auto wca = ThermoFun::cgf::wcaAt(beta, ro);
    return 1. + ro*wca.Fr;
}

real K23_13(Work& w, real T, real ro)
{
    static real TOld,roOld,KOLD;
    static real a,b,c,d,e;
    static real dtmp[]=
    { -1.050534, 1.747476,  1.749366,  -1.999227, -0.661046, -3.028720};

    if ( (T!=TOld) || (ro!=roOld) )
    {
        TOld = T;
        roOld = ro;
        a = ro*ro*log(T);
        b = ro*ro;
        c = ro*log(T);
        d = ro;
        e = log(T);
    }
    else return KOLD;

    KOLD = dtmp[0]*a + dtmp[1]*b + dtmp[2]*c + dtmp[3]*d + dtmp[4]*e + dtmp[5];
    KOLD = exp(KOLD/3.);
    return KOLD;
}

real FWCA(Work& w, real T,real ro)
{
    static real TOld,roOld,F;
    real d,beta,nu,nuw;
    real nu1w1,nu1w2,nu1w3,nu1w4,nu1w5;
    real a0,a1,a2,a3;
    real I2;
    real I1_6,I1_12;
    real dW,dW12,dW6;
    real tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
    real F0,F1,FA;
    real rm,rmdw1,rmdw2,rmdw3,rmdw4,rmdw5;

    if ((T==TOld) && (ro==roOld))
    {
        return F;
    }
    else
    {
        TOld = T;
        roOld = ro;
    }

    rm = TWOPOW1SIX;
    beta = 1./T;
    d = dHS(w, beta, ro );
    tmp2 = PISIX*d*d*d;
    nu = tmp2*ro;
    tmp1 = (1. - nu/16.);
    nuw = nu*tmp1;
    dW = d*exp(1./3.*log(tmp1));

    nu1w1 = (1.-nuw);
    nu1w2 = nu1w1*nu1w1;
    nu1w3 = nu1w2*nu1w1;
    nu1w4 = nu1w2*nu1w2;
    nu1w5 = nu1w2*nu1w3;

    tmp1 = (1-nu);
    tmp1 = tmp1*tmp1;
    F0 = ((4.-3.*nu)*nu)/tmp1;

    a0 = fa0( nuw , nu1w2);
    a1 = fa1( nuw , nu1w3);
    a2 = fa2( nuw , nu1w4);
    a3 = fa3( nuw , nu1w5);

    I1_6 = fI1_6( nuw );
    I1_12 = fI1_12( nuw );

    rmdw1 = rm/dW;
    rmdw2 = rmdw1*rmdw1;
    rmdw3 = rmdw1*rmdw2;
    rmdw4 = rmdw2*rmdw2;
    rmdw5 = rmdw3*rmdw2;

    dW6 = dW*dW*dW;
    dW6 = 1./(dW6*dW6);
    dW12 = dW6*dW6;

    tmp1 = (a0/4.+ a1/12. + a2/24. + a3/24.)*dW6;
    tmp2 = (a0/10.+ a1/90. + a2/720. + a3/5040.)*(-dW12);
    tmp3 = (a0 - a1/3. + a2/12 - a3/60)/8.;
    tmp4 = (a0 - a1 + a2/2. - a3/6.)*rmdw2*(-9.)/40.;
    tmp5 = (a1 - a2 + a3/2)*rmdw3*(-2.)/9.;
    tmp6 = (a2 - a3)*rmdw4*(-9.)/64.;
    tmp7 = a3*(-3.)/35.*rmdw5;

    I2 = tmp1+tmp2+tmp3+tmp4+tmp5+tmp6+tmp7;

    F1 = 48.*nuw*(I1_12*dW12-I1_6*dW6 + I2)*beta;
    FA = RPA(w, beta,nuw);

    F = F0+F1+FA;

    return F;
}

real RPA(Work& w, real beta,real nuw)
{
    real fi1,fi2;
    fi1 = (1.20110+(0.064890+(-76.860+(562.686+(-2280.090+(6266.840+(-11753.40+(14053.8
            +(-9491.490 +2731.030*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw;
    fi2 = (0.588890+(-7.455360+(40.57590+(-104.8970+(60.25470+(390.6310+(-1193.080
            +(1576.350+(-1045.910+283.7580*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw*nuw;
    return  (-12.*fi1 + 192.*fi2*beta)*beta*beta/PI_1;
}

real dHS(Work& w, real beta,real ro)
{
    // service constants
    real DV112 = 1./12.;
    real DV712 = 7./12.;
    // local variables
    real T12, T112, T712, B13, dB, delta, d;
    real a0, a1, a6, a3, a4, a7, a9, a12;
    real p0, p2, p6, p3, p5, p8, p11;
    real dbdl, ri6ro, ri6ro2, d3, d2, dnew, F0, F1;
    unsigned long int i;

    T12 = sqrt(beta);
    T112 = exp(DV112*log(beta));
    T712 = exp(DV712*log(beta));
    B13 = (1+beta);
    B13 = B13*B13*B13;

    dB = (P1*T112+PP2*T712+(P3+(P4+P5*beta)*beta)*beta)/B13;
    delta = (P6+P7*T12)/(1.+(P8+(P9+P10*T12)*T12)*T12);

    dbdl = dB*delta;
    ri6ro = PISIX*ro;
    ri6ro2 = ri6ro*ri6ro;

    a0 = dB+dbdl;
    a1 = -1.;
    a3 = (-1.5*dB -3.75*dbdl)*ri6ro;
    a4 = (1.5*ri6ro);
    a6 = (2.*dB + dbdl)*0.25*ri6ro2;
    a7 = -0.5*ri6ro2;
    a9 = -2.89325*ri6ro2*ri6ro*dbdl;
    a12 = -0.755*ri6ro2*ri6ro2*dbdl;

    p0 = -1.;
    p2 = a3*3.;
    p3 = a4*4.;
    p5 = a6*6.;
    p6 = a7*7.;
    p8 = a9*9.;
    p11 = a12*12.;

    d = dB;
    i = 0;

    while ( i++<21 )
    {
        d2 = d*d;
        d3 = d*d*d;
        F0 = a0+(a1+(a3+(a4+(a6+(a7+(a9+a12*d3)*d2)*d)*d2)*d)*d2)*d;
        F1 = p0+(p2+(p3+(p5+(p6+(p8+p11*d3)*d2)*d)*d2)*d)*d2;
        dnew = d-F0/F1;
        if ( fabs(dnew-d)<1.E-7 )
        {
            return dnew;
        }
        d = dnew;
    }

    if ( i>=20 )
    {
        return dB;
    }
    return dnew;
}

real Melt2(Work& w, real T)
{
    return T*0.+3.;
 }

real Melt(Work& w, real T)
{

    return T*0.+.9;

 }

void copy(Work& w, real* sours,real *dest,unsigned long int num)
{
    unsigned long int i;
    for ( i=0; i<num; i++)
    {
        dest[i]=sours[i];
    };
}

void norm(Work& w, real *X,unsigned long int mNum)
{
    real tmp=0.;
    unsigned long int i;
    for ( i=0; i<mNum; i++ )
    {
        tmp += X[i];
    }
    tmp = 1./tmp;
    for ( i=0; i<mNum; i++ )
    {
        X[i] *= tmp;
    }
}

long int CGActivCoefPT(Work& w, real *X,real *param, real *act, unsigned long int NN, real Pbar, real T, real &roro)
{
    real *xtmp,*Fx;
    real P = Pbar/10.;
    xtmp = new real [NN];
    Fx = new real [NN];

    if(!w.paar)
        w.paar = new  EOSPARAM(X, param, NN);
    else
        w.paar->init( X, param, NN );

    real F0,Z,F1,fideal;
    real ro,delta = DELTA,ax,dx /*,tmp*/;
    long int i;

    norm(w, w.paar->XX0,w.paar->NCmp());
    copy(w, w.paar->XX0,xtmp,w.paar->NCmp());

    w.paar->ParamMix(xtmp);

    ro = ROTOTALMIX(w, P,T,w.paar);

    if( ro < 0.0 )  // Too low pressure, no corrections will be done
        return ( -1 );

    Z = P/(R*T*ro);
    F0 = FTOTALMIX(w, T,ro,w.paar);

    // fideal=log(R*T*ro/BARMPA);
    fideal = log(R*T*ro/0.1);
    ax = Z - 1.+fideal;

    for ( i=0;i<w.paar->NCmp();i++)
    {
        if ( xtmp[i]>0. && w.paar->NCmp()==1 )
        {    // a pure fluid: d(nF)/dn = F (exact; the finite difference of the mixture formula gives F with a rounding error)
            Fx[i] = F0;
        }
        else if ( xtmp[i]>0. )
        {
            copy(w, w.paar->XX0,xtmp,w.paar->NCmp());
            dx = xtmp[i]*delta;
            xtmp[i] += dx;
            norm(w, xtmp,w.paar->NCmp());
            w.paar->ParamMix(xtmp);
            F1 = FTOTALMIX(w, T,ro,w.paar)*(1.+dx);
            Fx[i] = (F1-F0)/(dx);
        }
        else Fx[i] = 0.;
    };

    // GMix=0.;
    for ( i=0;i<w.paar->NCmp();i++)
    {
        if ( xtmp[i]>0. && Fx[i]< 100. )
        {
            // tmp=log(w.paar.XX0[i]);
            // GMix+=tmp*w.paar.XX0[i];
            act[i] = exp(ax+Fx[i]);
        }
        else
        {
            act[i] = 0.;
        }
    };

    // GMix+=F0 + ax;
    // MLPutRealList(stdlink,act,w.paar.NCmp());
    delete[]xtmp;
    delete[]Fx;
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

    Work work;
    work.Tk = TK;
    work.Pbar = Pbar;
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
