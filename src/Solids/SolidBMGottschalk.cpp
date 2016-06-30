#include "SolidBMGottschalk.h"

namespace TCorrPT {

//---------------------------------------------------------------------------
// Calculations related to Burch-Murnaghan (1947) equation (12 coeffs)
// Written in fortran by M.Gottschalk, GFZ Potsdam
// Translated to C/C++ by D.Kulik, 08 April 2003
// calculate the volume integral
auto BM_IntVol(Reaktoro::Pressure P, Reaktoro::Pressure Pref, Reaktoro::ThermoScalar vt, Reaktoro::ThermoScalar  vpt,
                 Reaktoro::ThermoScalar  kt0, Reaktoro::ThermoScalar  kp, Reaktoro::ThermoScalar  kpp) -> Reaktoro::ThermoScalar
{
    auto vt23 = pow( vt/vpt, 2./3. );

    auto pint = (-3.*kt0*vt*(6.*(199. - 75.*kp + 9.*kp*kp + 9.*kpp*kt0)*vpt*vt*pow( vt/vpt, 1./3. )
                + vt*vt*(-668. + kp*(276. - 63.*vt23) + 9*kp*kp*(-4. + vt23)
                + 9.*kpp*kt0*(-4. + vt23) + 143.*vt23)
                + vpt*vpt*(287. + kp*kp*(9. - 36.*vt23)
                + 9.*kpp*kt0*(1. - 4.*vt23) - 956.*vt23 + kp*(-87. + 324.*vt23))))/(128.*vpt*vpt);
// return vdP
   return (-Pref*vt+P*vpt-pint);
}


//-----------------------------------------------------------------------
// calculate the volume at P and T
auto BM_Volume( Reaktoro::ThermoScalar P, Reaktoro::ThermoScalar vt, Reaktoro::ThermoScalar kt0, Reaktoro::ThermoScalar kp,
                Reaktoro::ThermoScalar kpp, Reaktoro::ThermoScalar vstart) -> Reaktoro::ThermoScalar
{
//      double  veq, vv, vvnew, vvold, vt23, dveq;
      long int i=0;

      auto vv = vstart;
      auto vvold = vv;
      auto vvnew = vvold + 1.;

      // Newton iteration (max. 50 iterations)
      while ( fabs(vvnew.val-vvold.val) > 1e-10 && i++ < 50 )
      {
          auto vt23 = pow( vt/vv, 2./3.);
          auto veq  = 3./2.*kt0*(1.+3./4.*(kp-4.)*(vt23-1.)
                      + 3./8.*(143./9.-7.*kp+kp*kp+kpp*kt0)*(vt23-1.)*(vt23-1.))
                      * (vt23-1.)*pow( vt/vv, 5./3. ) - P;
          auto dveq = (kt0*vt*(vt*vt*(4509. + kp*kp*(243. - 99.*vt23)
                      + 9.*kpp*kt0*(27. - 11.*vt23) + 9.*kp*(-207. + 77.*vt23)
                      - 1573.*vt23) - 21.*(199. - 75.*kp + 9.*kp*kp + 9.*kpp*kt0)*vt*pow( vt/vv, 1./3. )*vv
                      + 5.*(239. - 81.*kp + 9.*kp*kp + 9.*kpp*kt0)*vt23*vv*vv))/(48.*vv*vv*vv*vv);
          vvold = vv;
          vvnew = vv - veq/dveq/2.;
          vv = vvnew;
      }

      // returns volume
      return vv;
}


//------------------------------------------------------------------------
// calculate the integral vdP using the Birch-Murnaghan EOS
// this function will be incorporated into GEM-Selektor v.2.1.0 code
auto BirchMurnaghan( Reaktoro::Pressure Pref, Reaktoro::Pressure P, Reaktoro::Temperature Tref, Reaktoro::Temperature T, Reaktoro::ThermoScalar v0,
          vector<double> BMConst, Reaktoro::ThermoScalar &vv, Reaktoro::ThermoScalar &alpha, Reaktoro::ThermoScalar &beta,
          Reaktoro::ThermoScalar &dG, Reaktoro::ThermoScalar &dH, Reaktoro::ThermoScalar &dS ) -> void
{
   Reaktoro::ThermoScalar vt, /*vpt,*/ a1, a2, a3, /*a4, a5,*/ kt00, kt0, dkdt, kp, kpp, vstart,
          /*Volume, IntVol, */ Pincr, Tincr, Pplus, Pminus, Tplus, Tminus,
          vPplus, vPminus, vTplus, vTminus, kt0Tplus, kt0Tminus, kppTplus, kppTminus,
          vtTplus, vtTminus, dGTplus, dGTminus;

   Pincr.val = 0.01;
   Tincr.val = 0.1;
       // v0 = BMConst(1) - in GEMS passed as a separate function parameter
    a1 = BMConst[0];
    a2 = BMConst[1];
    a3 = BMConst[2];
        // a4   = BMConst[3];  for future extensions
        // a5   = BMConst[4];
    kt00 = BMConst[5];
    dkdt = BMConst[6];
    kp = BMConst[7];
    kpp = BMConst[8];

    Pplus = P + Pincr;
    Pminus = P - Pincr;
    Tplus = T + Tincr;
    Tminus = T - Tincr;

    // calculate bulk modulus at T and its T increments
    kt0 = kt00 + dkdt*(T - Tref);
    kt0Tplus = kt00 + dkdt*(Tplus - Tref);
    kt0Tminus = kt00 + dkdt*(Tminus - Tref);

    // set kpp if not already defined and its T increments
    if ( fabs(kpp.val) < 1e-20 )
    {
      kpp = -((35./9.+(3.-kp)*(4.-kp))/kt0);
      kppTplus = -((35./9.+(3.-kp)*(4.-kp))/kt0Tplus);
      kppTminus = -((35./9.+(3.-kp)*(4.-kp))/kt0Tminus);
    }

    // calculate volume at T and Pref and its T increments
    vt = v0* exp( a1*(T-Tref)
         + a2/2.*(T*T-Tref*Tref)
         + a3*(-1./T+1./Tref) );
    vtTplus  =  v0* exp( a1*(Tplus-Tref)
         + a2/2.*(Tplus*Tplus-Tref*Tref)
         + a3*(-1./Tplus+1./Tref) );
    vtTminus =  v0* exp( a1*(Tminus-Tref)
         + a2/2.*(Tminus*Tminus-Tref*Tref)
         + a3*(-1./Tminus+1./Tref) );

    // calculate volume to start iterations
    vstart = vt* exp( -1./kt0*(P-Pref) );

    // calculate volumes at P and T and its increments
    vv = BM_Volume(P, vt, kt0, kp, kpp, vstart);
    vPplus = BM_Volume(Pplus, vt, kt0, kp, kpp, vv);
    vPminus = BM_Volume(Pminus, vt, kt0, kp, kpp, vv);
    vTplus = BM_Volume(P, vtTplus, kt0Tplus, kp, kppTplus, vv);
    vTminus = BM_Volume(P, vtTminus,kt0Tminus,kp,kppTminus, vv);

    // calculate aplha and beta at P and T
    alpha = 1./vv*((vTplus-vTminus)/(2.*Tincr));
    beta = - 1./vv*((vPplus-vPminus)/(2.*Pincr));

    // calculate vdP (P-T correction of G ->  dG)
    dG = BM_IntVol(P, Pref, vt, vv, kt0, kp, kpp);

    // calculate d(vdP)/dT (dS)
    dGTplus = BM_IntVol(P,Pref,vtTplus,vTplus,kt0Tplus,kp,kppTplus);
    dGTminus = BM_IntVol(P,Pref,vtTminus,vTminus,kt0Tminus,kp,kppTminus);
    dS = (dGTplus-dGTminus)/(2.*Tincr);

    // calculate dH
    dH = dG + T*dS;

}  // End of section for Birch-Murnaghan calculations



auto thermoPropertiesMinBMGottschalk (Reaktoro::Temperature t, Reaktoro::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{

    Reaktoro::Pressure Pst (0.1*subst.referenceP() / bar_to_Pa); // in bar
    Reaktoro::Temperature Tst (subst.referenceT()); // in K
    auto Vst = subst.thermoReferenceProperties().volume; // j/bar
    Reaktoro::Temperature T ( t.val );
    Reaktoro::Pressure P (0.1*p.val / bar_to_Pa); // in bar
    auto P_Pst = P - Pst;
    auto T_Tst = T -Tst;

    auto BMc = subst.thermoParameters().volume_BirchM_coeff;

    if( (P_Pst != 0.0 || T_Tst != 0.) && BMc.size() >= 8 )
    {
       Reaktoro::ThermoScalar VV0, GG0, HH0, SS0, aC, aE;

       BirchMurnaghan( Pst, P, Tst, T, Vst*10., BMc,
                       VV0, aC, aE, GG0, HH0, SS0 );
       // increments to V, G, H, S
       tps.volume += Vst+VV0/10.;    // fixed by KD on 22.11.04
           // aW.twp->V += VV0/10.;
       tps.entropy      += SS0;
       tps.gibbs_energy += GG0;
       tps.enthalpy     += HH0;
//       aW.twp->Alp = aC;
//       aW.twp->Bet = aE;
    }
    return tps;
}

}
