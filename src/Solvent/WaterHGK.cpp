// TCorrPT includes
#include "Solvent/WaterHGK.h"
#include "ThermoProperties.h"
#include "Common/Exception.h"

namespace TCorrPT {

auto errorHKFH2OValidity(std::string type, double P, double T, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Error HKF H2O: at " << type << ".";
    exception.reason << "entered values of T: "<< T << " & "<< P << name;
    exception.line = line;
    RaiseError(exception);
}


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

    // check insert data if Errir that trow
    valid(aSpc, hgk_param, T, P, Dens, eR);
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

auto valspc(HGK_SPECS aSpc) -> int
{
    if ( !( (0 <= aSpc.it)     && (aSpc.it     <= 3) &&
            (0 <= aSpc.id)     && (aSpc.id     <= 3) &&
            (0 <= aSpc.ip)     && (aSpc.ip     <= 4) &&
            (0 <= aSpc.ih)     && (aSpc.ih     <= 5) &&
            (0 <= aSpc.itripl) && (aSpc.itripl <= 1) &&
            (0 <= aSpc.isat)   && (aSpc.isat   <= 1) &&
            (1 <= aSpc.iopt)   && (aSpc.iopt   <= 2) &&
            (0 <= aSpc.useLVS) && (aSpc.useLVS <= 1) &&
            (1 <= aSpc.epseqn) && (aSpc.epseqn <= 5)))
        return(0);
    return(1);
}

auto  TdegK(int it, double t) -> double
{
    double TdegK=t;
    switch(it)
    {
    case 0:
        TdegK = t;
        break;
    case 1:
        TdegK = t + 273.15e0;
        break;
    case 2:
        TdegK = t / 1.8e0;
        break;
    case 3:
        TdegK = (t + 459.67e0) / 1.8e0;
        break;
    }
    return(TdegK);
}

auto bb(double t, HGK_GEMS_PARAM &hgk_param) -> void
{
    int i=0;
    double v[10];
    v[0] = 1.0e0;
    while(++i <= 9)
    {
        v[i] = v[i-1] * hgk_param.ac.tz / t;
    }
    hgk_param.el.b1   = hgk_param.bcn.bp[0] + hgk_param.bcn.bp[1] * log(1.0 / v[1]);
    hgk_param.el.b2   = hgk_param.bcn.bq[0];
    hgk_param.el.b1t  = hgk_param.bcn.bp[1] * v[1] / hgk_param.ac.tz;
    hgk_param.el.b2t  = 0.0e0;
    hgk_param.el.b1tt = 0.0e0;
    hgk_param.el.b2tt = 0.0e0;
    i=1;
    while ( ++i <= 9 )
    {
        hgk_param.el.b1   += hgk_param.bcn.bp[i] * v[i-1];
        hgk_param.el.b2   += hgk_param.bcn.bq[i] * v[i-1];
        hgk_param.el.b1t  -= (i-1) * hgk_param.bcn.bp[i] * v[i-1] / t;
        hgk_param.el.b2t  -= (i-1) * hgk_param.bcn.bq[i] * v[i-1] / t;
        hgk_param.el.b1tt += hgk_param.bcn.bp[i] *(i-1)*(i-1) * v[i-1] / t / t;
        hgk_param.el.b2tt += hgk_param.bcn.bq[i] * (i-1)*(i-1) * v[i-1] / t / t;
    }
    hgk_param.el.b1tt -= hgk_param.el.b1t / t;
    hgk_param.el.b2tt -= hgk_param.el.b2t / t;
}

//--------------------------------------------------------------------//
/*   /resf/  /qqqq/   /nconst/ /aconst/  /addcon/ /RTcurr/ /tolers/ */
/*  atz[3], adz[3], aat[3], aad[3], g[39], ii[39], jj[39]  */
auto resid(double t, double &d, HGK_GEMS_PARAM &hgk_param) -> void
{
    int i=0, l, j, k, km;
    double  qr[11], qt[10], qzr[9], qzt[9];
    double  e, /* q10,*/ q20, v, dfdt,/* q2a, q5t,*/ d2f, dpt, tex, /* fct,*/
    /*dadt,*/ ddz, zz, del, dex,/* qp,*/ qm, att, tx, tau, ex1, ex2;
    long double q10, fct, q5t, qp, dadt, q2a;
    qr[0]  = 0.0e0;
    hgk_param.qq.q5 = 0.0e0;
    hgk_param.qq.q0 = 0.0e0;
    hgk_param.res.ar = 0.0e0;
    dadt   = 0.0e0;
    hgk_param.res.cvr= 0.0e0;
    hgk_param.res.dpdtr = 0.0e0;
    e     = exp(-hgk_param.ac.aa * d);
    q10   = (d) * (d) * e;
    q20   = 1.0e0 - e;
    qr[1] = q10;
    v     = hgk_param.ac.tz / t;
    qt[0] = t / hgk_param.ac.tz;

    while(++i < 10)
    {
        qr[i+1] = qr[i] * q20;
        qt[i]   = qt[i-1] * v;
    }
    for (i=0; i<=8; i++) /* EQUIVALENCE (qr(3), qzr(1)), (qt(2), qzt(1)) */
    {
        qzr[i] = qr[i+2];
        qzt[i] = qt[i+1];
    }                  /*  as resalt: qzr[0]=qr[2], ... qzr[8]=qr[10];
                                         qzt[0]=qt[1], ... qzt[8]=qt[9]; */
    i=-1;
    while(++i <  hgk_param.nc.nc)
    {
        k  = hgk_param.nc.ii[i] + 1;
        l  = hgk_param.nc.jj[i];
        zz = k;
        if (k == 1)
            qp = hgk_param.nc.g[i] * hgk_param.ac.aa * qr[1] * qzt[l-1];
        else
            qp = hgk_param.nc.g[i] * hgk_param.ac.aa * qzr[k-2] * qzt[l-1];

        hgk_param.qq.q0 += qp;
        hgk_param.qq.q5 += hgk_param.ac.aa * (2.0 / d - hgk_param.ac.aa * (1.0 - e * (k-1) / q20)) * qp;
        hgk_param.res.ar += hgk_param.nc.g[i] * qzr[k-1] * qzt[l-1] / q10 / zz / hgk_param.a1.rt;
        dfdt    = pow(q20,((double)k)) * (1-l) * qzt[l] / hgk_param.ac.tz / k;
        d2f     = l * dfdt;
        dpt     = dfdt*q10 * hgk_param.ac.aa * k / q20;
        dadt   += hgk_param.nc.g[i] * dfdt;
        hgk_param.res.dpdtr += hgk_param.nc.g[i] * dpt;
        hgk_param.res.cvr   += hgk_param.nc.g[i] * d2f / hgk_param.ac.gascon;
    }
    qp  = 0.0e0;
    q2a = 0.0e0;
    j=35;
    while (++j <= 39 )
    {
        if ( hgk_param.nc.g[j] )
        {
            k   = hgk_param.nc.ii[j];
            km  = hgk_param.nc.jj[j];
            ddz = hgk_param.ad.adz[j-36];
            del = d / ddz - 1.0e0;
            if (fabs(del) < 1.0e-10)
                del = 1.0e-10;
            ex1   = -hgk_param.ad.aad[j-36] * pow(del,(double)k);

            if (ex1 < hgk_param.to.EXPTOL)
                dex = 0.0e0;
            else
                dex = exp(ex1)  * pow(del,(double)km);

            att   = hgk_param.ad.aat[j-36];
            tx    = hgk_param.ad.atz[j-36];
            tau   = t / tx - 1.0e0;
            ex2   = -att * tau * tau;

            if (ex2 <=  hgk_param.to.EXPTOL)
                tex = 0.0e0;
            else
                tex = exp(ex2);
            q10   = dex * tex;
            qm    = km / del - k * hgk_param.ad.aad[j-36] * pow(del,((double)k-1));
            fct   = qm * d * d * q10 / ddz;
            q5t   = fct * (2.0e0 / d + qm / ddz) - (d / ddz) * (d / ddz) *
                    q10 * (km / del / del + k * (k-1) * hgk_param.ad.aad[j-36] *
                           pow(del,((double)k-2)));
            hgk_param.qq.q5    += q5t * hgk_param.nc.g[j];
            qp        += hgk_param.nc.g[j] * fct;
            dadt      -= 2.0e0 * hgk_param.nc.g[j] * att * tau * q10 / tx;
            hgk_param.res.dpdtr -= 2.0e0 * hgk_param.nc.g[j] * att * tau * fct / tx;
            q2a       += t * hgk_param.nc.g[j] * (4.0e0 * att * ex2 + 2.0e0 *
                                         att) * q10 / tx / tx;
            hgk_param.res.ar    += q10 * hgk_param.nc.g[j] / hgk_param.a1.rt;
        }
    }
    hgk_param.res.sr   = -dadt / hgk_param.ac.gascon;
    hgk_param.res.ur   = hgk_param.res.ar + hgk_param.res.sr;
    hgk_param.res.cvr += q2a / hgk_param.ac.gascon;
    hgk_param.qq.q0  += qp;
}

//--------------------------------------------------------------------//
// calc  Abase, Gbase, Sbase, Ubase, Hbase, Cvbase,
// see  Haar , Gallagher & Kell (1979), eq(1)
/*  /ellcon/  /basef/   /aconst/  */
auto base(double &d, double t, HGK_GEMS_PARAM &hgk_param) -> void
{
    double x, z0, dz0, bb2tt;

    hgk_param.ac.yb = .25e0 * hgk_param.el.b1 * d;
    x      = 1.0e0 - hgk_param.ac.yb;
    z0     = (1.0e0 + hgk_param.el.g1 * hgk_param.ac.yb + hgk_param.el.g2 * hgk_param.ac.yb * hgk_param.ac.yb) / (x*x*x);
    hgk_param.ac.zb = z0 + 4.0e0 * hgk_param.ac.yb * (hgk_param.el.b2 / hgk_param.el.b1 - hgk_param.el.gf);
    dz0    = (hgk_param.el.g1 + 2.0e0 * hgk_param.el.g2 * hgk_param.ac.yb) / (x * x * x) + 3.0e0 *
             (1.0e0 + hgk_param.el.g1 * hgk_param.ac.yb + hgk_param.el.g2 * hgk_param.ac.yb * hgk_param.ac.yb) / (x*x*x*x);
    hgk_param.ac.dzb= dz0 + 4.0e0 * (hgk_param.el.b2 / hgk_param.el.b1 - hgk_param.el.gf);
    hgk_param.ba.pb = hgk_param.ac.zb;
    hgk_param.ba.ab = -log(x) - (hgk_param.el.g2 - 1.0e0)/x + 28.16666667e0 / x / x +
            4.0e0* hgk_param.ac.yb *(hgk_param.el.b2 / hgk_param.el.b1 - hgk_param.el.gf) + 15.166666667e0 +
            log( d * t * hgk_param.ac.gascon / .101325e0);
    hgk_param.ba.gb = hgk_param.ba.ab + hgk_param.ac.zb;
    hgk_param.ba.ub = -t * hgk_param.el.b1t * (hgk_param.ac.zb - 1.0e0 - d * hgk_param.el.b2) / hgk_param.el.b1
            - d * t * hgk_param.el.b2t;
    hgk_param.ba.sb = hgk_param.ba.ub - hgk_param.ba.ab;
    hgk_param.ba.hb = hgk_param.ac.zb + hgk_param.ba.ub;
    bb2tt  = t * t * hgk_param.el.b2tt;
    hgk_param.ba.cvb= 2.0e0 * hgk_param.ba.ub + (z0 - 1.0e0) * (((t * hgk_param.el.b1t / hgk_param.el.b1)
                                            * (t * hgk_param.el.b1t / hgk_param.el.b1)) - t * t * hgk_param.el.b1tt / hgk_param.el.b1)
            - d * (bb2tt - hgk_param.el.gf * hgk_param.el.b1tt * t * t) -
            (t * hgk_param.el.b1t /hgk_param.el.b1) * (t * hgk_param.el.b1t / hgk_param.el.b1) * hgk_param.ac.yb * dz0;
    hgk_param.ba.dpdtb = hgk_param.ba.pb / t + d * (hgk_param.ac.dzb * hgk_param.el.b1t / 4.0e0 + hgk_param.el.b2t -
                                 hgk_param.el.b2 / hgk_param.el.b1 * hgk_param.el.b1t);
}

//--------------------------------------------------------------------//
//calc density (d in g/cm3) and dP/dD (dPdd) as f(p(MPa), t(degK))
// from an initial density guess (dguess).
auto denHGK(double &d, double &p, double dguess, double t, double &dpdd, HGK_GEMS_PARAM &hgk_param) -> void
{
    double pp, dp, dpdx, x;
    int  i= 0;
    d  = dguess;
FIRST:
    do
    {
        i++;
        if (d <= 0.0e0) d = 1.0e-8;
        if (d > 1.9e0)  d = 1.9e0;
        resid(t,d, hgk_param);
        base(d,t, hgk_param);
        pp    = hgk_param.a1.rt * d * hgk_param.ba.pb + hgk_param.qq.q0;
        dpdd = hgk_param.a1.rt * (hgk_param.ac.zb + hgk_param.ac.yb * hgk_param.ac.dzb) + hgk_param.qq.q5;
        /*  if  dpdd < 0  assume d in 2-phase region and adjust accordingly */
        if (dpdd > 0.0) goto OTHER;
        if (dguess >= 0.2967e0)   d *= 1.02e0;
        if (dguess < 0.2967e0)    d *= 0.98e0;
    }
    while (i <= 10);
    return;
OTHER:
    {
        dpdx = dpdd * 1.1e0;
        if (dpdx < 0.1e0)
            dpdx = 0.1e0;
        dp   = fabs(1.0e0 - pp / p);
        if ((dp < 1.0e-8) || ((dguess > 0.3e0) && (dp < 1.0e-7)) ||
                ((dguess > 0.7e0) && (dp < 1.0e-6)))
            return;
        x = (p - pp) / dpdx;
        if (fabs(x)  >  0.1e0)
            x *= 0.1e0 / fabs(x);
        d += x;
        if (d <= 0.0e0)
            d = 1.0e-8;
        if ( i <= 30 ) goto FIRST;
    }
}

//--------------------------------------------------------------------//
// calc t/d properties H2O as ideal gas: equat see Woolley (1979).
auto ideal(double t, HGK_GEMS_PARAM &hgk_param) -> void
{
    int i=1;
    double  tt, tl, emult;
    double c[18]={ .19730271018e2,    .209662681977e2,  -.483429455355e0,
                   .605743189245e1,   .2256023885e2,    -.987532442e1,
                   -.43135538513e1,    .458155781e0,     -.47754901883e-1,
                   .41238460633e-2,  -.27929052852e-3,   .14481695261e-4,
                   -.56473658748e-6,   .16200446e-7,     -.3303822796e-9,
                   .451916067368e-11,-.370734122708e-13, .137546068238e-15 };

    tt  = t / 1.0e2;
    tl  = log(tt);
    hgk_param.id.gi  = - (c[0] / tt + c[1]) * tl;
    hgk_param.id.hi  = (c[1] + c[0] * (1.0e0 - tl) / tt);
    hgk_param.id.cpi = c[1] - c[0] / tt;
    while ( ++i <= 17 )
    {
        emult = pow(tt,((double)i - 5.));
        hgk_param.id.gi  -= c[i] * emult;
        hgk_param.id.hi  += c[i] * (i-5) * emult;
        hgk_param.id.cpi += c[i] * (i-5) * (i-4) * emult;
    }
    hgk_param.id.ai  = hgk_param.id.gi - 1.0e0;
    hgk_param.id.ui  = hgk_param.id.hi - 1.0e0;
    hgk_param.id.cvi = hgk_param.id.cpi - 1.0e0;
    hgk_param.id.si  = hgk_param.id.ui - hgk_param.id.ai;
}

//--------------------------------------------------------------------//
// calc t/d functions for HGK ( Helmholtz, Gibbs, et all)
auto thmHGK(double &d, double t, HGK_GEMS_PARAM &hgk_param) -> void
{
    double  z;

    z = hgk_param.ac.zb + hgk_param.qq.q0 / hgk_param.a1.rt/ d;
    hgk_param.fct.dpdd = hgk_param.a1.rt * (hgk_param.ac.zb + hgk_param.ac.yb * hgk_param.ac.dzb) + hgk_param.qq.q5;
    hgk_param.fct.ad   = hgk_param.ba.ab + hgk_param.res.ar + hgk_param.id.ai - hgk_param.ac.uref / t + hgk_param.ac.sref;
    hgk_param.fct.gd   = hgk_param.fct.ad + z;
    hgk_param.fct.ud   = hgk_param.ba.ub + hgk_param.res.ur + hgk_param.id.ui - hgk_param.ac.uref / t;
    hgk_param.fct.dpdt = hgk_param.a1.rt * d * hgk_param.ba.dpdtb + hgk_param.res.dpdtr;
    hgk_param.fct.cvd  = hgk_param.ba.cvb + hgk_param.res.cvr + hgk_param.id.cvi;
    hgk_param.fct.cpd  = hgk_param.fct.cvd + t * hgk_param.fct.dpdt * hgk_param.fct.dpdt /
               (d * d * hgk_param.fct.dpdd * hgk_param.ac.gascon);
    hgk_param.fct.hd   = hgk_param.fct.ud + z;
    hgk_param.fct.sd   = hgk_param.ba.sb + hgk_param.res.sr + hgk_param.id.si - hgk_param.ac.sref;
    hgk_param.fct.dvdt = hgk_param.fct.dpdt / hgk_param.fct.dpdd / d / d;
    hgk_param.fct.cjtt = 1.0e0 / d - t * hgk_param.fct.dvdt;
    hgk_param.fct.cjth = -hgk_param.fct.cjtt / hgk_param.fct.cpd / hgk_param.ac.gascon;
}

//--------------------------------------------------------------------//
// calc   (d(alpha)/dt)p(d,t,alpha) on equat Haar et al. (1983).
auto dalHGK(double &d, double t, double alpha, HGK_GEMS_PARAM &hgk_param) -> double
{
    int i=-1;
    double  k, l, km, lm, /* kp,*/ lp, x, xtzt, dydtp, dbdd,
    db2dd, db2ddt, db2dtp, db3ddt, db3dtt, drdd, dr2dd, dr2ddt,
    dr2dtp, /* dr3ddt, dr3dtt,*/ e1, e2, tzt, ai, bi, di, ti, tau, del,
    /* ex1, ex2, ex12, */ qm, xdell, xdelk, dalHGK/*, dex, tex*/,
    term1, term2, term3, term4;
    long double /* db2dd, db2ddt, db2dtp, db3ddt, db3dtt,
                 drdd, dr2dd, dr2ddt,dr2dtp,*/ dr3ddt, dr3dtt,
    ex1, ex2, ex12, dex, tex, term5;
    /*   evaluate derivatives for the base function */
    hgk_param.ac.yb= .25e0 * hgk_param.el.b1 * d;
    x     = 1.0e0 - hgk_param.ac.yb;
    dydtp = (d / 4.0e0) * (hgk_param.el.b1t - hgk_param.el.b1 * alpha);
    dbdd  = hgk_param.ac.gascon * t * ((hgk_param.el.b1 / 4.0e0 / x) * (1.0e0 -
                              (hgk_param.el.g2 - 1.0e0) / x + (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) /
                              x / x) + hgk_param.el.b2 - hgk_param.el.b1 * hgk_param.el.gf + 1.0e0 / d);
    db2dd = hgk_param.ac.gascon * t * ((hgk_param.el.b1 * hgk_param.el.b1 / 16.0e0 / x / x) *
                              (1.0e0 - 2.0e0 * (hgk_param.el.g2 - 1.0e0) / x + 3.0e0 *
                               (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) / x / x) - 1.0e0 / d / d);
    db2ddt = hgk_param.ac.gascon * t * ((hgk_param.el.b1t / 4.0e0 / x / x) *
                               (1.0e0 - (hgk_param.el.g2 - 1.0e0) * (1.0e0 + hgk_param.ac.yb) / x +
                                (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) * (1.0e0 + 2.0e0 * hgk_param.ac.yb) /
                                x / x) + hgk_param.el.b2t - hgk_param.el.gf * hgk_param.el.b1t) + dbdd / t;
    db2dtp = dbdd / t + hgk_param.ac.gascon * t * ((hgk_param.el.b1 * dydtp / 4.0e0 /
                                           x / x / x) * (1.0e0 - hgk_param.el.g2 + 2.0e0 * (hgk_param.el.g1 +
                                                                                   hgk_param.el.g2 + 1.0e0) / x) + ((x * hgk_param.el.b1t + hgk_param.el.b1 *
                                                                                                            dydtp) / 4.0e0 / x / x) * (1.0e0 - (hgk_param.el.g2 - 1.0e0) /
                                                                                                                                       x + (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) / x / x) + hgk_param.el.b2t -
                                          hgk_param.el.gf * hgk_param.el.b1t + alpha / d );
    db3ddt = db2dd / t + hgk_param.ac.gascon * t * ((hgk_param.el.b1 * hgk_param.el.b1 *
                                            dydtp / 8.0e0 / x / x / x / x) * (1.0e0 - hgk_param.el.g2 +
                                                                              3.0e0 * (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) / x) + (hgk_param.el.b1 *
                                                                                      (x * hgk_param.el.b1t + hgk_param.el.b1 * dydtp) / 8.0e0 / x / x / x) *
                                           (1.0e0 - 2.0e0 * (hgk_param.el.g2 - 1.0e0) / x + 3.0e0 *
                                            (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) / x / x) - 2.0e0 * alpha /
                                           d / d );
    db3dtt = (db2ddt - dbdd / t) / t + hgk_param.ac.gascon * t * ((hgk_param.el.b1t *
             dydtp / 2.0e0 / x / x / x / x) * (1.0e0 - hgk_param.el.g2 +
                                               (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) * (2.0e0 + hgk_param.ac.yb) / x) +
             ((x * hgk_param.el.b1tt + 2.0e0 * hgk_param.el.b1t * dydtp) / 4.0e0 /
              x / x / x) * (1.0e0 - (hgk_param.el.g2 - 1.0e0) * (1+ hgk_param.ac.yb) /
                            x + (hgk_param.el.g1 + hgk_param.el.g2 + 1.0e0) * (1.0e0 + 2.0e0 * hgk_param.ac.yb) /
                            x / x) + hgk_param.el.b2tt - hgk_param.el.gf * hgk_param.el.b1tt ) + (t * db2dtp -
                                    dbdd) / t / t;
    /***********************************************************/

    /*   evaluate derivatives for the residual function  */

    /*      drdd   = q/d/d                  */
    /*      dr2dd  = (q5 - 2.0e0/d*q0)/d/d   */
    /*      dr2ddt = dpdtr/d/d              */

    e1  = exp(-hgk_param.ac.aa * d);
    e2  = 1.0e0 - e1;
    tzt = hgk_param.ac.tz / t;
    drdd   = 0.0e0;
    dr2dd  = 0.0e0;
    dr2ddt = 0.0e0;
    dr2dtp = 0.0e0;
    dr3ddt = 0.0e0;
    dr3dtt = 0.0e0;
    /*    evaluate terms 1-36      */
    while (++i < hgk_param.nc.nc)
    {
        k = hgk_param.nc.ii[i] + 1.0e0;
        l = hgk_param.nc.jj[i] - 1.0e0;
        km = k - 1.0e0;
        //   lm = l - 1.0e0;
        /*  kp = k + 1.0e0; */
        lp = l + 1.0e0;
        xtzt = pow(tzt,l);
        drdd   = drdd + hgk_param.nc.g[i] * xtzt * pow(e2,km) * e1;
        dr2dd  = dr2dd + hgk_param.nc.g[i] * e1 * xtzt * pow(e2,km) *
                 (km * e1 / e2 - 1.0e0);
        dr2ddt = dr2ddt - hgk_param.nc.g[i] * e1 * l * pow(e2,km) * pow(tzt,lp) /
                 hgk_param.ac.tz;
        dr2dtp = dr2dtp + hgk_param.nc.g[i] * e1 * pow(e2,km) * xtzt *
                 ( d * alpha - l / t - km * e1 * d * alpha / e2 );
        dr3ddt = dr3ddt + hgk_param.nc.g[i] * (km * d * alpha * e1 * e1 * xtzt *
                                      pow(e2,(k-3.0e0)) + e1 * xtzt * pow(e2,km) * (km*e1/e2 -
                                              1.0e0) * (d * alpha - l / t - km * d * alpha * e1 / e2) );
        dr3dtt = dr3dtt + hgk_param.nc.g[i] * l * e1 * pow(e2,km) * pow(tzt,lp) /
                 hgk_param.ac.tz * (lp / t + d * alpha * km * e1 / e2 - d * alpha );
    }

    /*   evaluate terms 37-40   */

    i=35;
    while (++i <= 39)
    {
        k  = hgk_param.nc.ii[i];
        l  = hgk_param.nc.jj[i];
        km = k - 1.0e0;
        lm = l - 1.0e0;
        /*   kp = k + 1.0e0; */
        //  lp = l + 1.0e0;
        ai = hgk_param.ad.aad[i-36];
        bi = hgk_param.ad.aat[i-36];
        di = hgk_param.ad.adz[i-36];
        ti = hgk_param.ad.atz[i-36];
        tau = t / ti - 1.0e0;
        del = d / di - 1.0e0;

        if (fabs(del) < 1.0e-10)
            del = 1.0e-10;
        ex1 = -ai * pow(del,k);
        if (ex1 < hgk_param.to.EXPTOL)
            dex = 0.0e0;
        else
            dex = exp(ex1);
        ex2  = -bi * tau * tau;
        if (ex2 <= hgk_param.to.EXPTOL)
            tex = 0.0e0;
        else
            tex = exp(ex2);
        ex12    = dex * tex;
        qm      = l / del - k * ai * pow(del,km);
        xdell   = pow(del,l);
        xdelk   = pow(del,k);
        drdd   += hgk_param.nc.g[i] * xdell * ex12 / di * qm;

        dr2dd  += hgk_param.nc.g[i] * xdell * ex12 / di / di * (qm * qm -
                  l / di / di - ai * k * km * pow(del,k-2.0e0));

        dr2ddt -= hgk_param.nc.g[i] * 2.0e0 * bi * tau * ex12 * xdell / ti / di * qm;

        dr2dtp += hgk_param.nc.g[i] / di * (d * alpha * xdell * ex12 / di / del/del *
                                   (l + ai * k * km * xdelk) + qm * ( ex12 * (xdell* (k * ai *
                                                                      d * alpha * pow(del,km) / di - 2.0e0 * bi * tau / ti) - l *
                                                                      d * alpha * pow(del,lm)/di)));

        term1 = l * d * alpha / di / del / del + ai * k * km * d * alpha *
                pow(del,k-2.) / di;
        term2 = 2.* qm * term1 - 2.0e0 * l * d * alpha / di / del / del /
                del + ai * k * km * (k-2.) * pow(del,k-3.) * d * alpha / di;
        term3 = qm * qm - l / del / del - ai * k * km * pow(del,k-2.);
        term4 = ai * k * pow(del,k-1.) * d * alpha / di - 2.0e0 * bi * tau / ti;
        term5 = ex12 * xdell * term4 - ex12 * l * pow(del,l-1.) * d * alpha / di;

        dr3ddt += hgk_param.nc.g[i] / di / di * (xdell * ex12 * term2 + term3 * term5);

        dr3dtt -= 2.0e0 * hgk_param.nc.g[i] * bi / ti / di * (tau * xdell * ex12 * d *
                  alpha / del / del / di * (l + ai * k * km * pow(del,k)) + qm *
                  (xdell * ex12 * (ai * k * d * alpha * tau * pow(del,km) / di +
                                   (1.0e0 - 2.0e0 * bi * tau * tau) / ti - tau * l * d * alpha /
                                   di / del )));
    }

    /*   compute (d(alpha)/dT)P   */

    dalHGK = ((db3dtt + dr3dtt) * (2.0e0 * (dbdd + drdd) + d * (db2dd +
                                   dr2dd)) - (db2ddt + dr2ddt) * (2.0e0 * (db2dtp + dr2dtp) +
                                                                  d * (db3ddt + dr3ddt) - d * alpha * (db2dd + dr2dd))) /
             (2.0e0 * (dbdd + drdd) + d * (db2dd + dr2dd)) / (2.0e0 *
                     (dbdd + drdd) + d * (db2dd + dr2dd));

    return(dalHGK);
}

//--------------------------------------------------------------------//
// din. viscosity H2O in kg/m*s, if Pbars in validity region
// specified  by the  initial if  statement Watson et al. (1980);
//See  statement 3.1-2 and 4.1-5 in Tabl. 1,6,8 in Sengers and Kamgar-Parsi (1984).
auto viscos(double Tk, double Pbars, double Dkgm3, double betaPa) -> double
{
    double  a[4] = { 0.0181583e0,  0.0177624e0,
                     0.0105287e0, -0.0036744e0  };
    double b[7][6] =
        {
            { 0.5132047e0, 0.3205656e0, 0.0e0, 0.0e0,-0.7782567e0, 0.1885447e0 },
            { 0.2151778e0, 0.7317883e0, 1.2410440e0, 1.4767830e0, 0.0e0, 0.0e0 },
            {-0.2818107e0,-1.0707860e0,-1.2631840e0, 0.0e0, 0.0e0, 0.0e0 },
            { 0.1778064e0, 0.4605040e0, 0.2340379e0,-0.4924179e0, 0.0e0, 0.0e0 },
            {-0.0417661e0, 0.0e0, 0.0e0, 0.1600435e0, 0.0e0, 0.0e0 },
            { 0.0e0,-0.01578386e0, 0.0e0, 0.0e0, 0.0e0, 0.0e0  },
            { 0.0e0, 0.0e0, 0.0e0,-0.003629481e0, 0.0e0, 0.0e0 }
        };
    double Tstar, Dstar, Pstar, ustar, TOL, vIscos, TdegC, sum,
    T, D, u0, u1, u2, xt;
    int i=-1, j=-1;

    Tstar  = 647.270e0;
    Dstar  = 317.763e0;
    Pstar  = 22.1150e6;
    ustar  = 1.0e-6;
    TOL    = 1.0e-2;
    vIscos = 0.0e0;
    TdegC  = Tk - 273.15e0;

    if ((Pbars > 5000.0e0 + TOL) ||
            ((Pbars > 3500.0e0 + TOL) && (TdegC > 150.0e0 + TOL)) ||
            ((Pbars > 3000.0e0 + TOL) && (TdegC > 600.0e0 + TOL)) ||
            (TdegC  >  900.0e0 + TOL))
        return(vIscos);
    T = Tk / Tstar;
    D = Dkgm3 / Dstar;
    sum = 0.0e0;
    while(++i <= 3 )
    {
        sum += a[i] / pow(T,i);
    }
    u0 = ustar * sqrt(T) / sum;
    sum = 0.0e0;
    i=-1;
    while (++i <= 5)
    {
        while (++j <= 6)
        {
            sum += b[j][i] * pow((1.0e0/T-1),i) * pow((D-1),j);
        }
    }
    u1 = exp(D * sum);
    if( (0.997e0 <= T) && (T <= 1.0082e0) &&
            (0.755e0 <= D) && (D <= 1.2900e0) )
    {
        xt = Pstar / pow(Dstar,2.) * betaPa * pow(Dkgm3,2.);
        if (xt < 22.0e0)
            u2 = 1.0e0;
        else
            u2 = 0.922 * pow(xt,0.0263e0);
    }
    else
        u2 = 1.0e0;
    vIscos = u0 * u1 * u2;
    return(vIscos);
}

//--------------------------------------------------------------------//
// thermal conductivity H2O in J/m*deg*s (=W/m*deg),
// specified by the initial IF statement equat Sengers et al. (1984);
//See  statement 3.2.14 in Tabl.2-5 and I.5-6 from the above reference.
auto thcond(double Tk, double Pbars, double Dkgm3, double alph,
                       double betaPa) -> double
{
    int i=-1, j=-1;
    double TOL, Tstar, Dstar, Pstar, ustar, C, TdegC;
    double thCOND, T, D, sum, L0, L1, L2, u0, u1, xt, dPdT;
    double aL[4] = { 0.2022230e1, 0.1411166e2, 0.5255970e1,-0.2018700e1 };
    double au[4] = { 0.0181583e0, 0.0177624e0, 0.0105287e0,-0.0036744e0 };
    double bL[5][6] =
        {
            { 1.329304600e0,-0.404524370e0, 0.244094900e0, 0.018660751e0,
              -0.129610680e0, 0.044809953e0                                 },
            { 1.701836300e0,-2.215684500e0, 1.651105700e0,-0.767360020e0,
              0.372833440e0,-0.112031600e0                                 },
            { 5.224615800e0,-1.012411100e1, 4.987468700e0,-0.272976940e0,
              -0.430833930e0, 0.133338490e0                                 },
            { 8.712767500e0,-9.500061100e0, 4.378660600e0,-0.917837820e0,
              0.0e0, 0.0e0                                                 },
            {-1.852599900e0, 0.934046900e0, 0.0e0, 0.0e0, 0.0e0, 0.0e0     }
        };
    double bu[6][5] =
        {
            { 0.5019380e0, 0.2356220e0,-0.2746370e0, 0.1458310e0,-0.0270448e0 },
            { 0.1628880e0, 0.7893930e0,-0.7435390e0, 0.2631290e0,-0.0253093e0 },
            {-0.1303560e0, 0.6736650e0,-0.9594560e0, 0.3472470e0,-0.0267758e0 },
            { 0.9079190e0, 1.2075520e0,-0.6873430e0, 0.2134860e0,-0.0822904e0 },
            {-0.5511190e0, 0.0670665e0,-0.4970890e0, 0.1007540e0, 0.0602253e0 },
            { 0.1465430e0,-0.0843370e0, 0.1952860e0,-0.0329320e0,-0.0202595e0 }
        };
    TOL    = 1.0e-2;
    Tstar  = 647.270e0;
    Dstar  = 317.763e0;
    Pstar  = 22.1150e6;
    ustar  = 1.0e-6;
    C      = 3.7711e-8;
    thCOND = 0.0e0;
    TdegC  = Tk - 273.15e0;

    if ((Pbars > 4000.0e0+TOL) ||
            ((Pbars > 2000.0e0+TOL) && (TdegC > 125.0e0+TOL)) ||
            ((Pbars > 1500.0e0+TOL) && (TdegC > 400.0e0+TOL)) ||
            (TdegC  > 800.0e0+TOL))
        return(thCOND);
    T = Tk / Tstar;
    D = Dkgm3 / Dstar;
    sum = 0.0e0;
    while (++i <= 3)
    {
        sum += aL[i] / pow(T,(double)i);
    }
    L0 = sqrt(T) / sum;
    sum = 0.0e0;
    i=-1;
    while(++i <= 4)
    {
        while (++j <= 5)
        {
            sum += bL[i][j] * pow((1.0e0 / T-1),(double)i) * pow((D-1),(double)j);
        }
    }
    L1 = exp(D*sum);
    sum = 0.0e0;
    i=-1;
    while (++i <= 3)
    {
        sum += au[i] / pow(T,(double)i);
    }
    u0 = ustar * pow(T,0.5) / sum;
    sum = 0.0e0;
    i=-1;
    j=-1;
    while (++i <= 5)
    {
        while (++j <= 4)
        {
            sum += bu[i][j] * pow((1.0e0 / T-1),(double)i) *
                   pow( (D - 1), (double)j );
        }
    }
    u1   = exp(D * sum);
    xt   = Pstar / pow(Dstar,2.) * betaPa * pow(Dkgm3,2.);
    dPdT = Tstar / Pstar * alph/betaPa;
    L2   = C / (u0 * u1) * pow((T / D),2.) * pow(dPdT,2.) *
           pow(xt,0.4678e0) * pow(D,0.5) * exp(-18.66e0 *
                                               pow((T - 1),2.) - pow((D - 1),4.));
    thCOND = L0 * L1 + L2;
    return(thCOND);
}

//--------------------------------------------------------------------//
// Vargaftik et al. (1983). See  statement 10.1-2, in Kestin et al. (1984);
// and C.5 in Tabl. 11, Haar et al. (1984).
auto surten(double Tsatur) -> double
{
    double Ttripl, Tcrit, Tstar, Tcstar, v, B, stref, FPTOL;
    double sUrten, Tnorm;

    Ttripl = 273.16e0;
    Tcrit  = 647.067e0;
    Tstar  = 647.27e0;
    Tcstar = 0.999686e0;
    v      =  1.256e0;
    B      = -0.625e0;
    stref  = 0.2358e0;
    FPTOL  = 1.0e-10;

    if ((Tsatur < Ttripl) || (Tsatur > Tcrit))
    {
        sUrten = 0.0e0;
        return(sUrten);
    }

    if (Tsatur >= Tcrit - FPTOL)
        Tnorm = 0.0e0;
    else
        Tnorm = (Tcstar - Tsatur / Tstar) / Tcstar;

    sUrten = stref * pow(Tnorm,v) * (1.0e0 + B * Tnorm);
    return(sUrten);
}

//--------------------------------------------------------------------//
/* JN91 -Calculated (eps, dedP, dedT, d2edT2)(T,D) on statements
*         Johnson and Norton (1991);   fit regressed  from least
*         squares (HK74 and Pitz83 )
*           DATA type: T ............... K
*                    D ............... g/cm**3
*                    beta, dedP ...... bar**(-1)
*                    alpha, dedT ..... K**(-1)
*                    daldT, d2edT2 ... K**(-2)                         */
void JN91(double T, double D, double beta, double &alpha, double &daldT,
                   double &eps, double &dedP, double &dedT, double &d2edT2)
{
    int j=-1, k=0;
    double c[5], dcdT[5], dc2dTT[5];
    double Tref = 298.15e0, Tn;

    double a[10] = { 0.1470333593e+02, 0.2128462733e+03,
                     -0.1154445173e+03, 0.1955210915e+02,
                     -0.8330347980e+02, 0.3213240048e+02,
                     -0.6694098645e+01,-0.3786202045e+02,
                     0.6887359646e+02,-0.2729401652e+02 };

    Tn        = T / Tref;
    c[0]      = 1.0e0;
    dcdT[0]   = 0.0e0;
    dc2dTT[0] = 0.0e0;

    c[1]      = a[0] / Tn;
    dcdT[1]   = -a[0] * Tref / pow(T,2.);
    dc2dTT[1] = 2.0e0 * a[0] * Tref / pow(T,3.);

    c[2]      = a[1] / Tn + a[2] + a[3] * Tn;
    dcdT[2]   = -a[1] * Tref / pow(T,2.) + a[3] / Tref;
    dc2dTT[2] = 2.0e0 * a[1] * Tref / pow(T,3.);

    c[3]      = a[4] / Tn + a[5] * Tn + a[6] * pow(Tn,2.);
    dcdT[3]   = -a[4] * Tref / pow(T,2.) + a[5] / Tref + 2.0e0 *
                a[6] * T / pow(Tref,2.);
    dc2dTT[3] = 2.0e0 * a[4] * Tref / pow(T,3.) + 2.0e0 * a[6] / pow(Tref,2.);

    c[4]      = a[7] / pow(Tn,2.) + a[8] / Tn + a[9];
    dcdT[4]   =-2.0e0 * a[7] * pow(Tref,2.) / pow(T,3.) - a[8] * Tref /
               pow(T,2.);
    dc2dTT[4] = 6.0e0 * a[7] * pow(Tref,2.) / pow(T,4.) + 2.0e0 * a[8] *
                Tref / pow(T,3.);

    eps = 0.0e0;

    while (++k <= 5)
    {
        eps += c[k-1] * pow(D,(double)(k - 1));
    }
    dedP = 0.0e0;
    while (++j <=  4)
    {
        dedP += j * c[j] * pow(D,(double)j);
    }
    dedP *= beta;
    dedT = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        dedT += pow(D,(double)j) * (dcdT[j] - j * alpha * c[j]);
    }
    d2edT2 = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        d2edT2 += pow(D,(double)j) * (dc2dTT[j] - j * (alpha *
                                       dcdT[j] + c[j] * daldT) - j * alpha *
                                       (dcdT[j] - j * alpha * c[j]));
    }
}

//--------------------------------------------------------------------//
/* epsBrn - calc Brown functions Z, Q, Y, and X from their eps, dedP, dedT
   and d2edT2 counterparts.           */
auto epsBrn(double &eps, double dedP, double dedT,double d2edT2,
                     double &Z, double &Q, double &Y, double &X) -> void
{
    Z = -1.0e0 / eps;
    Q =  1.0e0 / pow(eps,2.) * dedP;
    Y =  1.0e0 / pow(eps,2.) * dedT;
    X =  1.0e0 / pow(eps,2.) * d2edT2 - 2.0e0 * eps * pow(Y,2.);
}

//--------------------------------------------------------------------//
/* Born92 - calculated Z, Q, Y, X .
*           epseqn = 1 statement Helgeson-Kirkham (1974)
*           epseqn = 2 statement Pitzer (1983)
*           epseqn = 3 statement Uematsu-Franck (1980)
*           epseqn = 4 statement Johnson-Norton (1991) !!!
*           epseqn = 5 statement Archer-Wang (1990)      */
auto Born92(double TK, double Pbars, double Dgcm3, double betab,
                     double &alphaK, double &daldT, double &eps, double &Z,
                     double &Q, double &Y, double &X, int epseqn) -> void
{
    //int iRET = 0;
    double TMAX, PMAX, TOL, TdegC, dedP, dedT, d2edT2;

    TMAX = 1000.0e0;
    PMAX = 5000.0;
    TOL  = 1.0e-3;

    eps = 0.0e0;
    Z   = 0.0e0;
    Y   = 0.0e0;
    Q   = 0.0e0;
    X   = 0.0e0;
    TdegC = TK - 273.15e0;
    /*  The following line can be commented out to facilitate probably
    *   unreliable, yet potentially useful, predictive calculations
    *   at state conditions beyond the validity limits of the aqueous
    *   species equation of state.                                   */

    if ( (TdegC > TMAX + TOL) || (Pbars > PMAX + TOL) )
        return;
    if (epseqn == 4)
    {
        /* d2edT2=0.; */
        JN91(TK, Dgcm3, betab, alphaK, daldT, eps, dedP, dedT,
             d2edT2);
        epsBrn(eps, dedP, dedT, d2edT2, Z, Q, Y, X);
    }
}

//--------------------------------------------------------------------//
/* triple - translate values U, S, H, A, G zero triple point
*  properties (Haar et al., 1984; Levelt Sengers et al., 1983) referenced
*  to triple  point properties data in Helgeson and   Kirkham, 1974a. */
auto triple(double T, WPROPS  &wr, HGK_GEMS_PARAM &hgk_param) -> void
{
    double TS;

    wr.Sw    = wr.Sw + hgk_param.tt.Stri;
    TS        = T * wr.Sw - hgk_param.tt.Ttr * hgk_param.tt.Stri;
    wr.Gw = wr.Hw - TS + hgk_param.tt.Gtri;
    wr.Aw = wr.Uw - TS + hgk_param.tt.Atri;
    wr.Hw = wr.Hw + hgk_param.tt.Htri;
    wr.Uw = wr.Uw + hgk_param.tt.Utri;
}

//--------------------------------------------------------------------//
auto dimHGK(int isat, int itripl, double t, double &p, double &d, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void
{
    double pbars, dkgm3, betaPa, betab, CpJKkg;

    hgk_param.wr.Aw   = hgk_param.fct.ad * hgk_param.a1.rt * hgk_param.un.fh;
    hgk_param.wr.Gw   = hgk_param.fct.gd * hgk_param.a1.rt * hgk_param.un.fh;
    hgk_param.wr.Sw   = hgk_param.fct.sd * hgk_param.ac.gascon * hgk_param.un.fh * hgk_param.un.ft;
    hgk_param.wr.Uw   = hgk_param.fct.ud * hgk_param.a1.rt * hgk_param.un.fh;
    hgk_param.wr.Hw   = hgk_param.fct.hd * hgk_param.a1.rt * hgk_param.un.fh;
    hgk_param.wr.Cvw  = hgk_param.fct.cvd * hgk_param.ac.gascon * hgk_param.un.fh * hgk_param.un.ft;
    hgk_param.wr.Cpw  = hgk_param.fct.cpd * hgk_param.ac.gascon * hgk_param.un.fh * hgk_param.un.ft;
    hgk_param.wr.Speedw = sqrt(fabs(hgk_param.fct.cpd * hgk_param.fct.dpdd * 1.0e3 /
                          hgk_param.fct.cvd)) * hgk_param.un.fs;
    hgk_param.wr.Betaw  = 1.0e0 / (d * hgk_param.fct.dpdd * hgk_param.un.fp);
    hgk_param.wr.Alphaw = d * hgk_param.fct.dvdt;
    hgk_param.wr.dAldT  = dalHGK(d,t,hgk_param.wr.Alphaw, hgk_param);                 /*!!!*/
    pbars = p * 1.0e1;
    dkgm3 = d * 1.0e3;
    betaPa = hgk_param.wr.Betaw * hgk_param.un.fp / 1.0e6;
    betab  = hgk_param.wr.Betaw * hgk_param.un.fp / 1.0e1;
    CpJKkg = hgk_param.wr.Cpw / hgk_param.un.fh / hgk_param.un.ft * 1.0e3;
    hgk_param.wr.Viscw  = viscos(t,pbars,dkgm3,betaPa) * hgk_param.un.fvd;
    hgk_param.wr.Tcondw = thcond(t,pbars,dkgm3,hgk_param.wr.Alphaw,betaPa) * hgk_param.un.fc * hgk_param.un.ft;

    if ((isat == 0) || (isat == 2))
        hgk_param.wr.Surtenw = 0.0e0;
    else
        hgk_param.wr.Surtenw = surten(t) * hgk_param.un.fst;

    Born92(t,pbars,dkgm3/1.0e3,betab,hgk_param.wr.Alphaw,hgk_param.wr.dAldT,
           hgk_param.wr.Dielw,hgk_param.wr.ZBorn,hgk_param.wr.QBorn,hgk_param.wr.YBorn,hgk_param.wr.XBorn,epseqn);
    hgk_param.wr.Tdiffw = hgk_param.wr.Tcondw / hgk_param.un.fc / hgk_param.un.ft / (dkgm3 * CpJKkg) * hgk_param.un.fvk;
    if ( hgk_param.wr.Tcondw )
        hgk_param.wr.Prndtlw = hgk_param.wr.Viscw / hgk_param.un.fvd * CpJKkg /
                     (hgk_param.wr.Tcondw / hgk_param.un.fc / hgk_param.un.ft);
    else
        hgk_param.wr.Prndtlw = 0.0e0;
    hgk_param.wr.Visckw = hgk_param.wr.Viscw / hgk_param.un.fvd / dkgm3 * hgk_param.un.fvk;
    hgk_param.wr.Albew  = hgk_param.wr.Alphaw / hgk_param.wr.Betaw;

    if (itripl == 1)
        triple(t, hgk_param.wr, hgk_param);
}

auto corr(int itripl, double t, double &p, double &dL, double &dV, double &delg, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void
{
    double dguess, gl, gv;
    bb(t, hgk_param);
    dguess = dL;
    if (dL <= 0.0e0)
        dguess = 1.11e0 - 0.0004e0 * t;
    denHGK(dL,p,dguess,t, hgk_param.fct.dpdd, hgk_param);
    ideal(t, hgk_param);
    thmHGK(dL,t, hgk_param);
    dimHGK(1,itripl,t,p,dL,epseqn, hgk_param);
    gl     = hgk_param.fct.gd;
    dguess = dV;
    if (dV <= 0.0e0)
        dguess = p / hgk_param.a1.rt;
    denHGK(dV,p,dguess,t,hgk_param.fct.dpdd, hgk_param);
    if (dV < 5.0e-7)
        dV = 5.0e-7;
    ideal(t, hgk_param);
    thmHGK(dV,t, hgk_param);
    /*  FCTS
     *      ୮  (dimensionless); after
     *    pcorr's final call of corr (*delg < 10d-4)   */
    gv    = hgk_param.fct.gd;
    delg = gl - gv;
}

auto pcorr(int itripl, double t, double &p, double &dL, double &dV, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void
{
    double dp;
    p  = PsHGK(t);
    dL= 0.0e0;
    dV = 0.0e0;
    do
    {
        corr(itripl, t, p, dL, dV, hgk_param.trp.delg, epseqn, hgk_param);
        dp   = hgk_param.trp.delg * hgk_param.ac.gascon * t / (1.0e0 / dV - 1.0e0 / dL);
        p  += dp;
    }
    while ( fabs(hgk_param.trp.delg) > 1.0e-4 );
}

auto PsHGK(double t) -> double
{
    double  pl, psHGK, v, w, b, q, z;
    int i=-1;
    double a[8] ={ -.78889166e1,  .25514255e1, -.6716169e1,  .33239495e2,
                   -.10538479e3,  .17435319e3, -.14839348e3, .48631602e2};
    if (t <= 314.0e0)
    {
        pl    = 6.3573118e0 - 8858.843e0 / t + 607.56335e0 * pow(t,-0.6e0);
        psHGK = 0.1e0 * exp(pl);
    }
    else
    {
        v = t / 647.25e0;
        w = fabs(1.0e0 - v);
        b = 0.0e0;
        while (++i <= 7)
        {
            z  = i + 1;
            b += a[i] * pow(w,((z + 1.0e0) / 2.0e0));
        }
        q = b / v;
        psHGK = 22.093e0 * exp(q);
    }
    return(psHGK);
}

auto  valTD(double T, double D, HGK_GEMS_PARAM &hgk_param, HGK_SPECS &aSpc, int epseqn) -> int
{
    int istemp;
    double TmnLVS, Tcrit, Ttripl, Dlimit, Tk, /* Ps, Dl, Dv, */
    /*P,*/ PMPa, Dguess, Dsublm, dPdD;

    TmnLVS= hgk_param.co.x[0];
    if ((T - hgk_param.to.FPTOL > hgk_param.hb.Ttop) || (T + hgk_param.to.FPTOL < hgk_param.hb.Tbtm) ||
            (D - hgk_param.to.FPTOL > hgk_param.hb.Dtop) || (D + hgk_param.to.FPTOL < hgk_param.hb.Dbtm))
        return(0);
    Tcrit = hgk_param.cr.Tc - 273.15e0;
    Ttripl = hgk_param.tt.Ttr - 273.15e0;
    if ((T + hgk_param.to.FPTOL >= Tcrit) || ((T >= hgk_param.li.TnIB30) && (D >= hgk_param.tt.Dltrip)))
    {
        Dlimit = hgk_param.li.sDIB30 * (T - hgk_param.li.TnIB30) + hgk_param.hb.Dtop;
        if ( !(D - hgk_param.to.FPTOL <= Dlimit ))
            return(0);
    }
    else
    {
        if (D - hgk_param.to.FPTOL <= hgk_param.tt.Dltrip)
        {
            if (T >= Ttripl)
            {
                Tk = T + 273.15e0;
                if (Tk < TmnLVS)
                {
                    hgk_param.a1.rt = hgk_param.ac.gascon * Tk;
                    pcorr(0,Tk,hgk_param.trp.Ps,hgk_param.trp.Dl,hgk_param.trp.Dv,epseqn, hgk_param);
                }
                else
                {
                    istemp = 1;
                    hgk_param.sa.DH2O = 0.0e0;
//                    trp.P    = Pfind(istemp, Tk, sa.DH2O);
//                    denLVS(istemp, Tk, trp.P);
                    hgk_param.trp.Dv = hgk_param.sa.Dvap / 1.0e3;
                    hgk_param.trp.Dl = hgk_param.sa.Dliq / 1.0e3;
                }
                if ((D >= hgk_param.trp.Dv) && (D <= hgk_param.trp.Dl))
                    aSpc.isat = 1; //isat = 1;
            }

            else
            {
//                hgk_param.trp.P = Psublm(T);
                PMPa = hgk_param.trp.P / 1.0e1;
                Tk = T + 273.15e0;
                Dguess = PMPa / Tk / 0.4e0;
                hgk_param.a1.rt = hgk_param.ac.gascon * Tk;
//                bb(Tk);
//                denHGK(&Dsublm,&PMPa,Dguess,Tk,&dPdD);
                if ( !( D - hgk_param.to.FPTOL <= Dsublm ))
                    return(0);
            }
        }
        else
        {
            if (D <= hgk_param.li.Dli13)
            {
                Dlimit = hgk_param.li.sDli1 * (T - hgk_param.li.Tli13) + hgk_param.li.Dli13;
                if ( !( D + hgk_param.to.FPTOL >= Dlimit ))
                    return(0);
            }
            else
            {
                Dlimit = hgk_param.li.sDli37 * (T - hgk_param.li.Tli13) + hgk_param.li.Dli13;
                if ( !( D - hgk_param.to.FPTOL <= Dlimit ))
                    return(0);
            }
        }
    }
    return(1);
}

auto valid(HGK_SPECS aSpc, HGK_GEMS_PARAM &hgk_param, double T, double &P, double &Dens0, int &eR) -> void
{
    double Ttripl, Tcrit, Pcrit; /*T, D, P, */
    // valid specification
    eR = valspc(aSpc);

    if (!eR)
    {
        Exception exception;
        exception.error << "HKF H2O: Specification error.";
        exception.reason << "HGK controls and specifications were not set correctly.";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    // translate to  degC, bars, g/cm3
    hgk_param.trp.T   = TdegK(aSpc.it, T) - 273.15e0;
    hgk_param.trp.D   = Dens0 * hgk_param.un.fd;
    hgk_param.trp.P   = P / hgk_param.un.fp * 1.0e1;
    Ttripl  = hgk_param.tt.Ttr - 273.15e0;
    Tcrit   = hgk_param.cr.Tc - 273.15e0;
    Pcrit   = hgk_param.cr.Pc * 1.0e1;

    if (aSpc.isat == 0)
    {
        if (aSpc.iopt == 1)
        {
            eR = valTD(hgk_param.trp.T, hgk_param.trp.D, hgk_param, aSpc, aSpc.epseqn);
            errorHKFH2OValidity("isat=0 & iopt=1", P, T, " fall outside validity region of HGK EOS!", __LINE__);
        }
        else
        {
//            eR = valTP(trp.T, trp.P);
              errorHKFH2OValidity("isat=0 & iopt=2", P, T, " fall outside validity region of HGK EOS!", __LINE__);
        }
    }
    else
    {
        if (aSpc.iopt == 1)
        {
//            if ( !((hgk_param.trp.T + hgk_param.to->FPTOL  >= Ttripl) && (hgk_param.trp.T- to->FPTOL <= Tcrit)))
//            {
//                eR = 0;
                  errorHKFH2OValidity("isat=1 & iopt=1", P, T, " input T falls outside the region between triple and critical points of H2O!\n"
                                                               " Please try with pressure P > 0.006 bar...", __LINE__);
//            }
        }
        else
        {
//            if ( !((hgk_param.trp.P+ hgk_param.to->FPTOL >= hgk_param.tt->Ptripl) && (trp.P - hgk_param.to->FPTOL <= Pcrit)))
//            {
//                //                *eR = 0;
                  errorHKFH2OValidity("isat=1 & iopt=2", P, T, " input P falls outside the region between triple and critical points of H2O!", __LINE__);
//            }
        }
    }
}

} // namespace TCorrPT
