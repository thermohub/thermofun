// TCorrPT includes
#include "Solvent/WaterHGK.h"
#include "ThermoProperties.h"

namespace TCorrPT {

auto WaterHGKProperties (double T, double &P) -> ThermoPropertiesSolvent
{
    ThermoPropertiesSolvent prop_solvent;
    HGK_GEMS_PARAM hgk_param;
    HGK_SPECS aSpc;

    int eR;
    double tempy, Dens;

    if( T < 0.01 && T >= 0.0 ) // Deg. C!
        T = 0.01;
    if( P < 6.11732e-3 ) // 6.11732e-3 is P_sat at triple point of H2O
        // At lower pressures HKF/HGK runs unstable or crashes
        P = 0.0;  // 06.12.2006  DK

    // in SUPCRT92 set type calculation of parametres
    aSpc.it=1;
    aSpc.id=1;
    aSpc.ip=1;
    aSpc.ih=4;
    aSpc.itripl=1;
    if( fabs( P ) == 0 )
    { // set only T
        aSpc.isat=1;
        aSpc.iopt=1;
    }
    else
    { //set T and P
        aSpc.isat = 0;
        aSpc.iopt = 2;
    }
    aSpc.useLVS=1;
    aSpc.epseqn=4;
    Dens = 0.0;

    //set local parameters with to fact scales
    unit(aSpc, hgk_param);
//    // check insert data if Errir that trow
//    valid(aSpc.it, aSpc.id, aSpc.ip, aSpc.ih, aSpc.itripl,
//          aSpc.isat, aSpc.iopt, aSpc.useLVS, aSpc.epseqn,
//          aSta.Temp, &aSta.Pres, &aSta.Dens[0], &eR);
//    // chack parametres in critical region eR=1
//    crtreg(aSpc.isat, aSpc.iopt, aSpc.it, &aSta.Temp, &aSta.Pres,
//           &aSta.Dens[0], &eR);
//    if( eR )
//    {
//        aSpc.icrit  = 1;
//        aSpc.useLVS = 1;
//    }
//    else
//    {
//        aSpc.icrit  = 0;
//        aSpc.useLVS = 0;
//    }

//    if ( aSpc.useLVS )
//    { // calculation of thermodynamic and transport water properties
//        // in the near critical region
//        ideal(aSta.Temp);   // bugfix for Akinfiev nonelectrolyte model (TW, 16.05.2008)
//    	LVSeqn(aSpc.isat, aSpc.iopt, aSpc.itripl,
//               aSta.Temp, &aSta.Pres, &aSta.Dens[0], aSpc.epseqn);
//        aSta.Dens[0] /= 1.0e3;
//        if ( aSpc.isat == 1 )
//            aSta.git rebase masterDens[1] /= 1.0e3;
//    }
//    else
//    {
//        aSta.Dens[0] /= 1.0e3;
//        HGKeqn(aSpc.isat, aSpc.iopt, aSpc.itripl,
//               aSta.Temp, &aSta.Pres, &aSta.Dens[0], aSpc.epseqn);
//    }
//    load(0, aWp);
//    if (aSpc.isat == 1)
//    {
//        tempy = aSta.Dens[0];
//        aSta.Dens[0] = aSta.Dens[1];
//        aSta.Dens[1] = tempy;
//        load(aSpc.isat, aWp);
//    }
//    // translate T - to users units
//    aSta.Temp   = TdegUS(aSpc.it, aSta.Temp);
//    aSta.Pres  *= un.fp;
//    aSta.Dens[0] /= un.fd;

//    if (aSpc.isat == 1)
//        aSta.Dens[1] /= un.fd;
//    *P = aSta.Pres;
//    aWp.init = true;




    return prop_solvent;
}

auto unit (HGK_SPECS aSpc, HGK_GEMS_PARAM &hgk_param) -> void
{
    double fft[4] =  {1.e0, 1.e0, 0.555555556e0, 0.555555556e0 };
    double ffd[4] = {1.e-3, 1.e0, 1.80152e-2, 1.6018e-2};
    double ffvd[4]= {1.e0, 1.e1, 0.555086816e0, 0.671968969e0 };
    double ffvk[4]= {1.e0, 1.e4, 1.0e4, 1.076391042e1 };
    double ffs[4] = {1.e0, 1.e2, 1.0e2, 3.280833e0 };
    double ffp[5] = {1.e0, 1.e1, 9.869232667e0, 1.45038e2, 1.01971e1};
    double ffh[6] = {1.e0, 1.e0, 1.80152e1, 2.3901e-1, 4.305816e0, 4.299226e-1};
    double ffst[4]= {1.e0, 1.e3, 0.555086816e2, 0.2205061e1 };
    double ffcd[4]= {1.e0, 1.e-2, 1.0e-2, 0.3048e0 };
    double ffch[6]= {1.e-3, 1.e0, 1.0e0, 0.23901e0, 0.23901e0, 0.947244e-3 };


    hgk_param.un.ft  = fft[aSpc.it];
    hgk_param.un.fd  = ffd[aSpc.id];
    hgk_param.un.fvd = ffvd[aSpc.id];
    hgk_param.un.fvk = ffvk[aSpc.id];
    hgk_param.un.fs  = ffs[aSpc.id];
    hgk_param.un.fp  = ffp[aSpc.ip];
    hgk_param.un.fh  = ffh[aSpc.ih];
    hgk_param.un.fst = ffst[aSpc.id];
    hgk_param.un.fc  = ffcd[aSpc.id] * ffch[aSpc.ih];
    if ( aSpc.itripl == 1 )
        tpset(hgk_param);
}

auto tpset(HGK_GEMS_PARAM &hgk_param) -> void
{
    double Utr, Str, Htr, Atr, Gtr;

    Utr = -15766.0e0;
    Str =  3.5144e0;
    Htr = -15971.0e0;
    Atr = -12870.0e0;
    Gtr = -13073.0e0;

    hgk_param.tt.Utri = Utr * hgk_param.un.fh;
    hgk_param.tt.Stri = Str * hgk_param.un.fh;
    hgk_param.tt.Htri = Htr * hgk_param.un.fh;
    hgk_param.tt.Atri = Atr * hgk_param.un.fh;
    hgk_param.tt.Gtri = Gtr * hgk_param.un.fh;
}

} // namespace TCorrPT
