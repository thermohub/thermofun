#ifndef WATERHGKGEMS_H
#define WATERHGKGEMS_H

#include <memory>
#include <cstring>

namespace TCorrPT {

// Forward declarations
struct ThermoPropertiesSolvent;
struct ThermoPropertiesSubstance;

typedef struct
{ //work structure  t/d properties of water-solution
    // ( see at WATERPARAM)
    double Aw, Gw, Sw, Uw, Hw, Cvw, Cpw, Speedw, Alphaw,
    Betaw, Dielw, Viscw, Tcondw, Surtenw, Tdiffw,
    Prndtlw, Visckw, Albew;
    double ZBorn, YBorn, QBorn, dAldT, XBorn;
}
WPROPS;

struct HGK_SPECS
{// HGK CONTROLS AND SPECIFICAIONS
    int it;
    int id;
    int ip;
    int ih;
    int itripl;
    int isat;
    int iopt;
    int useLVS;
    int epseqn;
    int icrit;
};

struct HGK_STATES
{
    double Temp;    // C
    double Pres;    // bar
    double Dens[2]; // g/cm3
//    STATES()
//    {
//        Temp = -274.;
//        Pres = -1;
//        Dens[0] =Dens[1] = 0.;
//    }
//    void setdef()
//    {
//        Temp = -274.;
//        Pres = -1;
//        Dens[0] =Dens[1] = 0.;
//    }
};

typedef struct
{ // set local parametres to fact scales
    double ft;
    double fd;
    double fvd;
    double fvk;
    double fs;
    double fp;
    double fh;
    double fst;
    double fc;
}
UNITS;

typedef struct
{ /*  abc2    */
    double r, th;
}
ABC2;

typedef struct
{ /*  satur   */
    int iphase;
    double Dliq, Dvap, DH2O;
}
SATUR;

typedef struct
{ /* qqqq */
    double q0, q5;
}
QQQQ;

typedef struct
{ /*  fcts  */
    double ad, gd, sd, ud, hd, cvd, cpd,
    dpdt, dvdt, dpdd, cjtt, cjth;
}
FCTS;

typedef struct
{ /*  basef */
    double ab, gb, sb, ub, hb, cvb, pb, dpdtb;
}
BASEF;

typedef struct
{ /*  resf */
    double ar, gr, sr, ur, hr, cvr, dpdtr;
}
RESF;

typedef struct
{ /*  idf  */
    double ai, gi, si, ui, hi, cvi, cpi;
}
IDF;

typedef struct
{ /*  abc1,  abc3,   RTcurr  */
    double dPdM, dPdTcd, rt;
}
ABC1;

typedef struct
{ /* param  */
    double r1, th1;
}
PARAM;

typedef struct
{ /*  deri2  */
    double dPdD, dPdT;
}
DERI2;

typedef struct
{ /*  deriv  */
    double amu, s[2], sd[2], Pw, Tw, dTw,
    dM0dT, dP0dT, d2PdM2, d2PdMT,
    d2PdT2, p0th, p1th, xk[2];
}
DERIV;

typedef struct
{ /*  tpoint   */
    double Utri, Stri, Htri, Atri, Gtri,
    Ttr, Ptripl, Dltrip, Dvtrip;
}
TPOINT;

typedef struct
{ /*  aconst   */
    double wm, gascon, tz, aa, uref, sref, zb, dzb, yb;
}
ACONST;

typedef struct
{ /*  nconst  */
    double g[40];
    int    ii[40], jj[40], nc;
}
NCONST;

typedef struct
{ /*  addcon   */
    double atz[4], adz[4], aat[4], aad[4];
}
ADDCON;

typedef struct
{ /*  ellcon   */
    double g1, g2, gf, b1, b2, b1t, b2t, b1tt, b2tt;
}
ELLCON;

typedef struct
{ /*  bconst   */
    double bp[10], bq[10];
}
BCONST;

typedef struct
{ /*  crits    */
    double Tc, rhoC, Pc, Pcon, Ucon, Scon, dPcon;
}
CRITS;

typedef struct
{ /*  coefs   */
    double a[20], q[20], x[11];
}
COEFS;

typedef struct
{ /*  tolers   */
    double TTOL, PTOL, DTOL, XTOL, EXPTOL, FPTOL;
}
TOLERS;

typedef struct
{ /*  HGKbnd   */
    double Ttop, Tbtm, Ptop, Pbtm, Dtop, Dbtm;
}
HGK_BND;

typedef struct
{ /*  liqice   */
    double sDli1, sPli1, sDli37, sPli37,
    sDIB30, Tli13, Pli13, Dli13, TnIB30, DnIB30;
}
LIQICE;

typedef struct
{ /* io  */
    int iconf;
    double rterm, wterm, reacf, pronf, tabf, plotf;
}
IO_Y;

typedef struct
{ /*  HGKcrt   */
    double tcHGK, dcHGK, pcHGK;
}
HGK_CRT;

typedef struct
{ /*  therm  */
    double AE, GE, U, H, Entrop, Cp, Cv,
    betaw, alphw, heat, Speed;
}
THERM;

typedef struct
{// local values
    double T;
    double P;
    double Ps;
    double D;
    double Dv;
    double Dl;
    double delg;  /* (Gl-Gv)/RT                     */
}
TERM_PR;

struct STORE
{ /*  store   */
    int isav1;
    double sav2, sav3, sav4, sav5, sav6, sav7,
    sav8, sav9, sav10, sav11, sav12, sav13,
    sav14, sav15, sav16, sav17, sav18, sav19;
    STORE():isav1(0)
    {
        memset( &sav2, 0, sizeof(double)*18);
    }
};

class WaterHGKgems
{
public:
    WaterHGKgems();

private:

    HGK_SPECS aSpc; HGK_STATES aSta;

    WPROPS wl;
    WPROPS wr;
    ABC2 a2;
    ABC1 a1;
    TERM_PR trp;
    UNITS un;
    SATUR sa;
    QQQQ qq;
    FCTS fct;
    BASEF ba;
    RESF res;
    IDF id;
    THERM th;
    PARAM par;
    DERI2 d2;
    DERIV dv;
    IO_Y io;
    //
    ACONST   *ac;
    NCONST   *nc;
    ELLCON   *el;
    BCONST   *bcn;
    ADDCON   *ad;
    HGK_CRT  *hc;
    TOLERS   *to;
    HGK_BND  *hb;
    LIQICE   *li;
    TPOINT   *tt;
    CRITS    *cr;
    COEFS    *co;

    auto unit(int it, int id, int ip, int ih, int itripl) -> void;

    auto tpset( ) -> void;

    auto valid(int it, int id, int ip, int ih, int itripl, int isat,
               int iopt, int useLVS, int epseqn, double Temp, double *Pres,
               double *Dens0, int *eR) -> void;

    auto valspc(int it, int id, int ip, int ih, int itripl,
                int isat, int iopt, int useLVS, int epseqn) -> int;

//    auto TdegK(int it, double t) -> double;

    auto valTD(double T, double D, int isat, int epseqn) -> int;

    auto pcorr(int itripl, double t, double *p, double *dL,double *dV, int epseqn) -> void;

    auto PsHGK(double t) -> double;

    auto corr(int itripl, double t, double *p, double *dL,
              double *dV, double *delg, int epseqn) -> void;

    auto bb(double t) -> void;

    auto resid(double t, double *d) -> void;

    auto base(double *d, double t) -> void;

    auto denHGK(double *d, double *p, double dguess, double t, double *dpdd) -> void;

    auto ideal(double t) -> void;

    auto thmHGK(double *d, double t) -> void;

    auto dalHGK(double *d, double t, double alpha) -> double;

    auto viscos(double Tk, double Pbars, double Dkgm3, double betaPa) -> double;

    auto thcond(double Tk, double Pbars, double Dkgm3, double alph, double betaPa) -> double;

    auto surten(double Tsatur) -> double;

    auto JN91(double T, double D, double beta, double *alpha, double *daldT, double *eps, double *dedP, double *dedT, double *d2edT2) -> void;

    auto epsBrn(double *eps, double dedP, double dedT,double d2edT2,
                         double *Z, double *Q, double *Y, double *X) -> void;

    auto Born92(double TK, double Pbars, double Dgcm3, double betab,
                         double *alphaK, double *daldT, double *eps, double *Z,
                         double *Q, double *Y, double *X, int epseqn) -> void;

    auto triple(double T, WPROPS  *wr) -> void;

    auto dimHGK(int isat,int itripl, double t, double *p, double *d, int epseqn) -> void;

    auto crtreg(int isat, int iopt, int it, double *T, double *P,
                double *D, int *eR) -> void;

    auto LVSeqn(int isat, int iopt, int itripl, double TC,
                double *P, double *Dens0, int epseqn) -> void;

    auto HGKeqn(int isat, int iopt, int itripl, double Temp,
                double *Pres, double *Dens0, int epseqn) ->void;

    auto valTP(double T, double P) -> int;

    auto Pfind(int isat, double T, double DD) -> double;

    auto conver(double *rho,
                          double Tee, double rho1s, double *rhodi, double error1) -> void;

    auto ss(double r, double th, double *s, double *sd) -> void;

    auto rtheta(double *r, double *theta, double rho, double Tee) -> void;

    auto aux(double r1, double th1, double *d2PdT2, double *d2PdMT,
                       double *d2PdM2, double aa, double *xk, double *sd, double Cvcoex) -> void;

    auto denLVS(int isat, double T, double P) -> void;

    auto Psublm(double Temp) -> double;

    auto TsLVS(int isat, double Pres) -> double;

    auto LVSsat(int iopt, int isat, double *T, double *P, double *D)  -> void;

    auto thmLVS(int isat, double T, double r1, double th1)  -> void;

    auto dalLVS(double D, double T, double P, double alpha) -> double;

    auto dimLVS(int isat, int itripl, double theta, double T, double *Pbars,
                         double *dL, double *dV, WPROPS *www, int epseqn) -> void;

    auto cpswap() -> void;

    auto HGKsat(int& isat, int iopt, int itripl, double Temp,
                         double *Pres, double *Dens, int epseqn) -> void;

    auto calcv3(int iopt, int itripl, double Temp, double *Pres,
                         double *Dens, int epseqn) -> void;

    auto TdegUS(int it, double t) -> double;

    auto tcorr(int itripl, double *t, double *p, double *dL, double *dV,
                        int epseqn) -> void;

    auto TdPsdT(double t_) -> double;

    auto TsHGK(double Ps_) -> double;

    auto TdegK(int it, double t) -> double;

    auto errorHKFH2OValidity(std::string type, double P, double T, std::string name, int line) -> void;

    auto backup( struct STORE &sto ) -> void;

    auto restor( struct STORE sto ) -> void;

public:

    auto waterHGK (double T, double P) -> void;

    auto propertiesSolvent() -> ThermoPropertiesSolvent;

    auto propertiesSubstance() -> ThermoPropertiesSubstance;

};

}

#endif // WATERHGKGEMS_H
