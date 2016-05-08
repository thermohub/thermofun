#ifndef WATERHGK_H
#define WATERHGK_H

// TCorrPT includes
//#include "ThermoModelsSolvent.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

// Forward declarations
struct ThermoPropertiesSolvent;

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
{ /*  tpoint   */
    double Utri, Stri, Htri, Atri, Gtri,
    Ttr, Ptripl, Dltrip, Dvtrip;
}
TPOINT;


struct  HGK_GEMS_PARAM // description of thermodinamic properties of water
{
    // working parametera
    // WLIQUID wl;
//    WPROPS wl;
//    WPROPS wr;
//    ABC2 a2;
//    ABC1 a1;
//    TERM_PR trp;
    UNITS un;
//    SATUR sa;
//    QQQQ qq;
//    FCTS fct;
//    BASEF ba;
//    RESF res;
//    IDF id;
//    THERM th;
//    PARAM par;
//    DERI2 d2;
//    DERIV dv;
//    IO_Y io;
    //
//    ACONST   *ac;
//    NCONST   *nc;
//    ELLCON   *el;
//    BCONST   *bcn;
//    ADDCON   *ad;
//    HGK_CRT  *hc;
//    TOLERS   *to;
//    HGK_BND  *hb;
//    LIQICE   *li;
    TPOINT   tt;
//    CRITS    *cr;
//    COEFS    *co;
};

auto WaterHGKProperties (double T, double &P) -> ThermoPropertiesSolvent;

auto unit(HGK_SPECS aSpc, HGK_GEMS_PARAM &hgk_gems_param) -> void;

auto tpset(HGK_GEMS_PARAM hgk_param) -> void;

} // namespace TCorrPT

#endif // WATERHGK_H
