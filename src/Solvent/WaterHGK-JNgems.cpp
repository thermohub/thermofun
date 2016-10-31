// TCorrPT includes
#include "Common/Exception.h"
#include "WaterHGK-JNgems.h"
#include "ThermoProperties.h"

namespace TCorrPT {


auto WaterHGKgems::thermoPropertiesWaterHGKgems(int state) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance wp;

    if ( (aSpc.isat && (state == 1)) || (!aSpc.isat && (state == 0)) ) // vapor properties at Psat or liquid properties not at Psat, also supercritical fluid
    {
        wp.helmholtz_energy         = wl.Aw * cal_to_J;
        wp.gibbs_energy             = wl.Gw * cal_to_J;
        wp.entropy                  = wl.Sw * cal_to_J;
        wp.internal_energy          = wl.Uw * cal_to_J;
        wp.enthalpy                 = wl.Hw * cal_to_J;
        wp.heat_capacity_cv         = wl.Cvw * cal_to_J;
        wp.heat_capacity_cp         = wl.Cpw * cal_to_J;

        // Rho in g/cm3, waterMolarMass in g/mol
        wp.volume                   = 1/aSta.Dens[1] * H2OMolarMass * cm3_mol_to_J_bar;
    } else
    if ((aSpc.isat && (state == 0))|| (!aSpc.isat && (state == 1)) ) // liquid properties at Psat or vapor properties not at Psat, also supercritical fluid
    {
        wp.helmholtz_energy         = wr.Aw * cal_to_J;
        wp.gibbs_energy             = wr.Gw * cal_to_J;
        wp.entropy                  = wr.Sw * cal_to_J;
        wp.internal_energy          = wr.Uw * cal_to_J;
        wp.enthalpy                 = wr.Hw * cal_to_J;
        wp.heat_capacity_cv         = wr.Cvw * cal_to_J;
        wp.heat_capacity_cp         = wr.Cpw * cal_to_J;

        // Rho in g/cm3, waterMolarMass in g/mol
        wp.volume                   = 1/aSta.Dens[0] * H2OMolarMass * cm3_mol_to_J_bar;
    }

return wp;
}

auto WaterHGKgems::propertiesWaterHGKgems(int state) -> PropertiesSolvent
{
    PropertiesSolvent wp;
    double rho, alp, dal, bet;

    if ( (aSpc.isat && (state == 1)) || (!aSpc.isat && (state == 0)) ) // vapor properties at Psat or liquid properties not at Psat, also supercritical fluid
    {
        wp.surface_tension   = wl.Surtenw;
        wp.Alpha    = wl.Alphaw;
        wp.Beta     = wl.Betaw / 1e05; // from bar-1 to Pa-1
        wp.thermal_conductivity    = wl.Tcondw;
        wp.Tdiff    = wl.Tdiffw;
        wp.Prndtl   = wl.Prndtlw;
        wp.dAldT    = wl.dAldT;
        wp.Albe     = wl.Albew;
        wp.speed_of_sound    = wl.Speedw;
        wp.dynamic_viscosity     = wl.Viscw;
        wp.Visck    = wl.Visckw;

        alp = wl.Alphaw;
        dal = wl.dAldT;
        bet = wl.Betaw / 1e05; // from bar-1 to Pa-1

        wp.density          = aSta.Dens[1] * 1000; // in kg/m3
        rho = aSta.Dens[1] * 1000;
    } else
    if ((aSpc.isat && (state == 0))|| (!aSpc.isat && (state == 1)) )
    {
        wp.surface_tension   = wr.Surtenw;
        wp.Alpha    = wr.Alphaw;
        wp.Beta     = wr.Betaw / 1e05; // from bar-1 to Pa-1
        wp.thermal_conductivity    = wr.Tcondw;
        wp.Tdiff    = wr.Tdiffw;
        wp.Prndtl   = wr.Prndtlw;
        wp.dAldT    = wr.dAldT;
        wp.Albe     = wr.Albew;
        wp.speed_of_sound    = wr.Speedw;
        wp.dynamic_viscosity     = wr.Viscw;
        wp.Visck    = wr.Visckw;

        alp = wr.Alphaw;
        dal = wr.dAldT;
        bet = wr.Betaw / 1e05; // from bar-1 to Pa-1

        wp.density          = aSta.Dens[0] * 1000; // in kg/m3
        rho = aSta.Dens[0] * 1000;
    }

//    if ((aSpc.isat && (state == 0)) || (aSpc.metastable && (state == 0)))
//    {
//        wp.Surten   = wl.Surtenw;
//        wp.Alpha    = wl.Alphaw;
//        wp.Beta     = wl.Betaw / 1e05; // from bar-1 to Pa-1
//        wp.Tcond    = wl.Tcondw;
//        wp.Tdiff    = wl.Tdiffw;
//        wp.Prndtl   = wl.Prndtlw;
//        wp.dAldT    = wl.dAldT;
//        wp.Albe     = wl.Albew;
//        wp.Speed    = wl.Speedw;
//        wp.Visc     = wl.Viscw;
//        wp.Visck    = wl.Visckw;

//        alp = wl.Alphaw;
//        dal = wl.dAldT;
//        bet = wl.Betaw / 1e05; // from bar-1 to Pa-1

//    } else
//    {
//        wp.Surten   = wr.Surtenw;
//        wp.Alpha    = wr.Alphaw;
//        wp.Beta     = wr.Betaw / 1e05; // from bar-1 to Pa-1
//        wp.Tcond    = wr.Tcondw;
//        wp.Tdiff    = wr.Tdiffw;
//        wp.Prndtl   = wr.Prndtlw;
//        wp.dAldT    = wr.dAldT;
//        wp.Albe     = wr.Albew;
//        wp.Speed    = wr.Speedw;
//        wp.Visc     = wr.Viscw;
//        wp.Visck    = wr.Visckw;

//        alp = wr.Alphaw;
//        dal = wr.dAldT;
//        bet = wr.Betaw / 1e05; // from bar-1 to Pa-1
//    }

    wp.gibbsIdealGas    = id.gi;
    wp.entropyIdealGas  = id.si;
    wp.cpIdealGas       = id.cpi;

//    if (aSpc.metastable && !aSpc.isat)
//    {
//        wp.density          = aSta.Dens[!aSpc.isat] * 1000; // in kg/m3
//        rho = aSta.Dens[!aSpc.isat] * 1000;
//    } else
//    {
//        wp.density          = aSta.Dens[aSpc.isat] * 1000; // in kg/m3
//        rho = aSta.Dens[aSpc.isat] * 1000;
//    }

    wp.densityT = - alp * rho;
    wp.densityTT = rho * ( pow(alp,2.) - dal );
    wp.densityP = bet * rho;
//    wp.densityPP =
//    wp.pressure = P;

return wp;
}

auto WaterHGKgems::electroPropertiesWaterJNgems(int state) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent wp;
    double eps, xborn, yborn, qborn;

    if ( (aSpc.isat && (state == 1)) || (!aSpc.isat && (state == 0)) ) // vapor properties at Psat or liquid properties not at Psat, also supercritical fluid
    {
        wp.epsilon  = wl.Dielw;
        wp.bornZ    = wl.ZBorn;
        wp.bornY    = wl.YBorn;
        wp.bornQ    = wl.QBorn / 1e05; // from bar-1 to Pa-1
        wp.bornX    = wl.XBorn;

        eps   = wl.Dielw;
        xborn = wl.XBorn;
        yborn = wl.YBorn;
        qborn = wl.QBorn;
    } else
    if ((aSpc.isat && (state == 0))|| (!aSpc.isat && (state == 1)) )
    {
        wp.epsilon  = wr.Dielw;
        wp.bornZ    = wr.ZBorn;
        wp.bornY    = wr.YBorn;
        wp.bornQ    = wr.QBorn / 1e05; // from bar-1 to Pa-1
        wp.bornX    = wr.XBorn;

        eps   = wr.Dielw;
        xborn = wr.XBorn;
        yborn = wr.YBorn;
        qborn = wr.QBorn;
    }

//    if ((aSpc.isat && (state == 0)) || (aSpc.metastable && (state == 0)))
//    {
//        wp.epsilon  = wl.Dielw;
//        wp.bornZ    = wl.ZBorn;
//        wp.bornY    = wl.YBorn;
//        wp.bornQ    = wl.QBorn / 1e05; // from bar-1 to Pa-1
//        wp.bornX    = wl.XBorn;

//        eps   = wl.Dielw;
//        xborn = wl.XBorn;
//        yborn = wl.YBorn;
//        qborn = wl.QBorn;
//    }
//    else
//    {
//        wp.epsilon  = wr.Dielw;
//        wp.bornZ    = wr.ZBorn;
//        wp.bornY    = wr.YBorn;
//        wp.bornQ    = wr.QBorn / 1e05; // from bar-1 to Pa-1
//        wp.bornX    = wr.XBorn;

//        eps   = wr.Dielw;
//        xborn = wr.XBorn;
//        yborn = wr.YBorn;
//        qborn = wr.QBorn;
//    }

    wp.epsilonT  = yborn * pow(eps,2.);
    wp.epsilonTT = (xborn + 2.*eps*pow(yborn,2.)) * pow(eps,2.);
    wp.epsilonP  = qborn * pow(eps,2.);

return wp;
}


/* HGKcon - Constant parameters for the H2O equation of state given by
*           Haar,  Gallagher, & Kell (1984):  bp, bq = b(j), B(j) from
*           Table A.1, p.272; g1, g2, gf = alpha, beta, gamma  from eq
*           (A.2), p.272; g, ii, jj  = g(i), k(i), l(i) from eq (A.5),
*           p.272. Note that  tz < tcHGK.
*                 Tolerence limits required  in various real & inexact
*            comparisons are set and stored in COMMON /tolers/.     */


ACONST  ac_ = { /* wm, gascon, tz, aa, uref, sref*/
    0.1801520000e2, 0.46152200e0, 0.647073e3, 0.1e1,
    -0.4328455039e4, 0.76180802e1,  0., 0., 0.};

NCONST  nc_ ={ /* g(40), ii(40), jj(40), nc */
    {-0.53062968529023e3, 0.22744901424408e4, 0.78779333020687e3,
     -0.69830527374994e2, 0.17863832875422e5,-0.39514731563338e5,
     0.33803884280753e5,-0.13855050202703e5,-0.25637436613260e6,
     0.48212575981415e6,-0.34183016969660e6, 0.12223156417448e6,
     0.11797433655832e7,-0.21734810110373e7, 0.10829952168620e7,
     -0.25441998064049e6,-0.31377774947767e7, 0.52911910757704e7,
     -0.13802577177877e7,-0.25109914369001e6, 0.46561826115608e7,
     -0.72752773275387e7, 0.41774246148294e6, 0.14016358244614e7,
     -0.31555231392127e7, 0.47929666384584e7, 0.40912664781209e6,
     -0.13626369388386e7, 0.69625220862664e6,-0.10834900096447e7,
     -0.22722827401688e6, 0.38365486000660e6, 0.68833257944332e4,
     0.21757245522644e5,-0.26627944829770e4,-0.70730418082074e5,
     -0.22500000000000e0,-0.16800000000000e1, 0.5500000000000e-1,
     -.93000000000000e2},                             /* g[40]  */
    {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
     4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 8, 8, 8, 8,
     2, 2, 0, 4, 2, 2, 2, 4},                        /* ii[40] */
    {2, 3, 5, 7, 2, 3, 5, 7, 2, 3, 5, 7, 2, 3, 5, 7,
     2, 3, 5, 7, 2, 3, 5, 7, 2, 3, 5, 7, 2, 3, 5, 7,
     1, 4, 4, 4, 0, 2, 0, 0},                        /* jj[40] */
    36};                                           /*  nc    */

ELLCON  el_ = { /* g1, g2, gf */
    0.11e2, 0.44333333333333e2, 0.35e1, 0., 0., 0., 0., 0., 0.};

BCONST  bc_ = { /* bp(10), bq(10) */
    {0.7478629e0,-0.3540782e0,  0.0e0, 0.0, 0.7159876e-2,
     0.0e0,      -0.3528426e-2, 0.0e0, 0.0, 0.0},    /* bp[10] */
    {0.11278334e1, 0.0e0, -0.5944001e0, -0.5010996e1, 0.0e0,
     0.63684256e0, 0.0e0, 0.0, 0.0, 0.0}};           /* bq[10] */

ADDCON  ad_ = { /* atz(4), adz(4), aat(4), aad(4) */
    {0.64e3,  0.64e3,  0.6416e3, 0.27e3},   /* atz[4]  */
    {0.319e0, 0.319e0, 0.319e0,  0.155e1},  /* adz[4]  */
    {0.2e5,   0.2e5,   0.4e5,    0.25e2},   /* aat[4]  */
    {0.34e2,  0.4e2,   0.3e2,    0.105e4} }; /* aad[4]  */

HGK_CRT hc_ = { /* tcHGK, dcHGK, pcHGK */
    0.647126e3, 0.322e3, 0.22055e2};

TOLERS  to_ = { /* TTOL, PTOL, DTOL, XTOL, EXPTOL, FPTOL */
    1.0e-6, 1.0e-6, 1.0e-9, 1.0e-5, -673.5e0, 1.0e-7};

HGK_BND hb_ = { /* Ttop, Tbtm, Ptop, Pbtm, Dtop, Dbtm */
    2.25e3,-2.0e1, 3.0e4, 1.0e-3,
    0.138074666423686955066817336896e1,
    0.858745555396173972667420987465e-7};

LIQICE  li_ = { /* sDli1, sPli1, sDli37, sPli37, sDIB30,
                       Tli13, Pli13, Dli13, TnIB30, DnIB30 */
    -0.584797401732178547634910059828e-2,
    -0.138180804975562958027981345769e3,
    0.183244000000000000000000000007e-2,
    0.174536874999999999999999999995e3,
    -0.168375439429928741092636579574e-3,
    -0.15e2,
    0.20741e4,
    0.108755631570602617113573577945e1,
    0.145e3,
    0.102631640581853166397515716306e1};

TPOINT  tt_ = {/* Utri,Stri,Htri,Atri,Gtri
                       Ttr, Ptripl, Dltrip, Dvtrip */
    0e0, 0e0, 0e0, 0.0e0, 0e0,
    2.7316e2,
    0.611731677193563186622762580414e-2,
    0.999778211030936587977889295063e0,
    0.485467583448287303988319166423e-5};

CRITS   cr_ = { /* Tc, rhoC, Pc, Pcon, Ucon, Scon, dPcon */
    647.067e0,  322.778e0,  22.046e0,
    0.034070660379837018423130834983e0,
    22046.0e0,
    0.034070660379837018423130834983e3,
    0.000000327018783663660700780197e0};

COEFS   co_ = { /* a(20), q(20), x(11) */
    {-0.017762e0, 5.238000e0, 0.000000e0,-2.549150e1,
     6.844500e0, 0.325000e0, 1.440300e0, 0.000000e0,
     1.375700e0, 2.366660e1, 4.820000e0, 0.294200e0,
     -1.123260e1,-2.265470e1,-1.788760e1,-4.933200e0,
     1.109430391161019373812391218008e0,
     -1.981395981400671095301629432211e0,
     0.246912528778663959151808173743e0,
     -0.843411332867484343974055795059e0},      /* a[20] */

    {-0.006000e0,-0.003000e0, 0.000000e0, 6.470670e2,
     3.227780e2, 2.204600e1, 0.267000e0,-1.600000e0,
     0.491775937675717720291497417773e0, 0.108500e0,
     0.586534703230779473334597524774e0,
     -1.026243389120214352553706598564e0,
     0.612903225806451612903225804745e0, 0.500000e0,
     -0.391500e0, 0.825000e0, 0.741500e0,
     0.103245882826119154987166286332e0,
     0.160322434159191991394857495360e0,
     -0.169859514687100893997445721324e0},      /* q[20] */

    {6.430000e2, 6.453000e2, 6.950000e2,
     1.997750e2, 4.200400e2,
     2.09945691135940719075293945960e1,
     2.15814057875264119875397458907e1,
     3.0135e1, 4.0484e1,
     0.175777517046267847932127026995e0,
     0.380293646126229135059562456934e0}};     /* x[11] */

auto WaterHGKgems::errorHKFH2OValidity(std::string type, double P, double T, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Error HKF H2O: at " << type << ".";
    exception.reason << "entered values of T: "<< T << " & "<< P << name;
    exception.line = line;
    RaiseError(exception);
}

auto  WaterHGKgems::TdegK(int it, double t) -> double
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

WaterHGKgems::WaterHGKgems()
{
    // set global constant
    //  npH2O++;
    ac = &ac_;
    nc = &nc_;
    el = &el_;
    bcn = &bc_;
    ad = &ad_;
    hc = &hc_;
    to = &to_;
    hb = &hb_;
    li = &li_;
    tt = &tt_;
    cr = &cr_;
    co = &co_;
}

auto WaterHGKgems::calculateWaterHGKgems(double T, double &P) -> void
{
    int eR;
    double tempy;
    double Tk = T + 273.15;

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

    if (Tk <= 647.067e0)
        aSta.Psat = PsHGK(Tk)*10;
    else
        aSta.Psat = 0;

    if( (fabs( P ) == 0) || (P == aSta.Psat) )
    { // set only T
        aSpc.isat=1;
        aSpc.iopt=1;
        aSpc.metastable = 0;
        P = PsHGK(Tk)*10;
    }
    else
    { //set T and P
        aSpc.isat = 0;
        aSpc.iopt = 2;
        aSpc.metastable = 1;
    }
    aSpc.useLVS=1;
    aSpc.epseqn=4;
    aSta.Dens[1] = aSta.Dens[0] = 0.0;
    aSta.Pres = P;
    aSta.Temp = T;


    //
//    aSpc.isat = 1;
//    aSpc.iopt = 1;

    memset( &wl, 0, sizeof(WPROPS));
    memset( &wr, 0, sizeof(WPROPS));
    memset( &a2, 0, sizeof(ABC2));
    memset( &a1, 0, sizeof(ABC1));
    memset( &trp, 0, sizeof(TERM_PR));
    memset( &un, 0, sizeof(UNITS));
    memset( &sa, 0, sizeof(SATUR));
    memset( &qq, 0, sizeof(QQQQ));
    memset( &fct, 0, sizeof(FCTS));
    memset( &ba, 0, sizeof(BASEF));
    memset( &res, 0, sizeof(RESF));
    memset( &id, 0, sizeof(IDF));
    memset( &th, 0, sizeof(THERM));
    memset( &par, 0, sizeof(PARAM));
    memset( &d2, 0, sizeof(DERI2));
    memset( &dv, 0, sizeof(DERIV));
    memset( &io, 0, sizeof(IO_Y));

    //set local parameters with to fact scales
    unit(aSpc.it, aSpc.id, aSpc.ip, aSpc.ih, aSpc.itripl);

    // check insert data if Errir that trow
    valid(aSpc.it, aSpc.id, aSpc.ip, aSpc.ih, aSpc.itripl,
          aSpc.isat, aSpc.iopt, aSpc.useLVS, aSpc.epseqn,
          aSta.Temp, &aSta.Pres, &aSta.Dens[0], &eR);
    // chack parametres in critical region eR=1
    crtreg(aSpc.isat, aSpc.iopt, aSpc.it, &aSta.Temp, &aSta.Pres,
           &aSta.Dens[0], &eR);
    if( eR )
    {
        aSpc.icrit  = 1;
        aSpc.useLVS = 1;
    }
    else
    {
        aSpc.icrit  = 0;
        aSpc.useLVS = 0;
    }

    if ( aSpc.useLVS )
    { // calculation of thermodynamic and transport water properties
        // in the near critical region
        ideal(aSta.Temp);   // bugfix for Akinfiev nonelectrolyte model (TW, 16.05.2008)
        LVSeqn(aSpc.isat, aSpc.iopt, aSpc.itripl,
               aSta.Temp, &aSta.Pres, &aSta.Dens[0], aSpc.epseqn);
        aSta.Dens[0] /= 1.0e3;
        if ( aSpc.isat == 1 )
            aSta.Dens[1] /= 1.0e3;

        WPROPS tw = wl;
        memcpy(&wl, &wr, sizeof(WPROPS));
        memcpy(&wr, &tw, sizeof(WPROPS));

//        double temp = aSta.Dens[0];
//        aSta.Dens[0] = aSta.Dens[1];
//        aSta.Dens[1] = temp;

    }
    else
    {
        aSta.Dens[0] /= 1.0e3;
        HGKeqn(aSpc.isat, aSpc.iopt, aSpc.itripl,
               aSta.Temp, &aSta.Pres, &aSta.Dens[0], aSpc.epseqn);
    }

//    prop_solvent = load(0);
//    if (aSpc.isat == 1)
//    {
//        tempy = aSta.Dens[0];
//        aSta.Dens[0] = aSta.Dens[1];
//        aSta.Dens[1] = tempy;
//        //        prop_solvent = load(1);
//    }
    // translate T - to users units
    aSta.Temp   = TdegUS(aSpc.it, aSta.Temp);
    aSta.Pres  *= un.fp;
    aSta.Dens[0] /= un.fd;

    if (aSpc.isat == 1)
    {
        aSta.Dens[0] /= un.fd;
        aSta.Dens[1] /= un.fd;
    }
//    P = aSta.Pres;
//    aWp.init = true;

}

auto WaterHGKgems::unit(int it, int id, int ip, int ih, int itripl) ->void
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

    un.ft  = fft[it];
    un.fd  = ffd[id];
    un.fvd = ffvd[id];
    un.fvk = ffvk[id];
    un.fs  = ffs[id];
    un.fp  = ffp[ip];
    un.fh  = ffh[ih];
    un.fst = ffst[id];
    un.fc  = ffcd[id] * ffch[ih];
    if ( itripl == 1 )
        tpset();
}

//--------------------------------------------------------------------//
// valid insert data and scales for HKF
auto WaterHGKgems::valid(int it, int id, int ip, int ih, int itripl, int isat,
               int iopt, int useLVS, int epseqn, double Temp, double *Pres,
               double *Dens0, int *eR) -> void  /* double Temp, Pres, Dens; */
{
    double Ttripl, Tcrit, Pcrit; /*T, D, P, */
    // valid specification
    *eR = valspc(it, id, ip, ih, itripl, isat, iopt, useLVS, epseqn);

    if (!eR)
    {
        Exception exception;
        exception.error << "HKF H2O: Specification error.";
        exception.reason << "HGK controls and specifications were not set correctly.";
        exception.line = __LINE__;
        RaiseError(exception);
    }


    // translate to  degC, bars, g/cm3
    trp.T   = TdegK(it, Temp) - 273.15e0;
    trp.D   = *Dens0 * un.fd;
    trp.P   = *Pres / un.fp * 1.0e1;
    Ttripl  = tt->Ttr - 273.15e0;
    Tcrit   = cr->Tc - 273.15e0;
    Pcrit   = cr->Pc * 1.0e1;
    if (isat == 0)
    {
        if (iopt == 1)
        {
            *eR = valTD(trp.T, trp.D, isat, epseqn);
            if(!*eR)
            errorHKFH2OValidity("isat=0 & iopt=1", *Pres, Temp, " fall outside validity region of HGK EOS!", __LINE__);
        }
        else
        {
            *eR = valTP(trp.T, trp.P);
            if(!*eR)
            errorHKFH2OValidity("isat=0 & iopt=2", *Pres, Temp, " fall outside validity region of HGK EOS!", __LINE__);
        }
    }
    else
    {
        if (iopt == 1)
        {
            if ( !((trp.T + to->FPTOL  >= Ttripl) && (trp.T- to->FPTOL <= Tcrit)))
            {
                *eR = 0;
                errorHKFH2OValidity("isat=1 & iopt=1", *Pres, Temp, " input T falls outside the region between triple and critical points of H2O!\n"
                                                             " Please try with pressure P > 0.006 bar...", __LINE__);
            }
        }
        else
        {
            if ( !((trp.P+ to->FPTOL >= tt->Ptripl) && (trp.P - to->FPTOL <= Pcrit)))
            {
                *eR = 0;
                errorHKFH2OValidity("isat=1 & iopt=2", *Pres, Temp, " input P falls outside the region between triple and critical points of H2O!", __LINE__);
            }
        }
    }
}

auto WaterHGKgems::valspc(int it, int id, int ip, int ih, int itripl,
                    int isat, int iopt, int useLVS, int epseqn) -> int
{
    if ( !((0 <= it)     && (it     <= 3) &&
            (0 <= id)     && (id     <= 3) &&
            (0 <= ip)     && (ip     <= 4) &&
            (0 <= ih)     && (ih     <= 5) &&
            (0 <= itripl) && (itripl <= 1) &&
            (0 <= isat)   && (isat   <= 1) &&
            (1 <= iopt)   && (iopt   <= 2) &&
            (0 <= useLVS) && (useLVS <= 1) &&
            (1 <= epseqn) && (epseqn <= 5)))
        return(0);
    return(1);
}

//--------------------------------------------------------------------//
/* Pfind - Return P(T,D) if OK and -1 - if error
*          Calculated (dP/dD)T  when  invoked by Dfind
*          (isat=0)  and  (dP/dT)D  when  invoked  by  SUB  TsLVS
*          (isat=1).       */
auto  WaterHGKgems::Pfind(int isat, double T, double DD) -> double
{
    double err=0.;
    double  /* sd[2],*/ Pw0, Pw1, Pw2, Pw3,  am1, am2, am3,
    p00, p20, p40, p01, p21, p41, aa, xk0, xk1, pw11, alpha,
    alhi, besq, Tee, rho, rhodi=0., rho1=0., tt1, tt2, Pwmu,
                                         dPw0, dPw1, dPw, Pfind, Uw, dPdTcd, dPwdTw, Cvcoex=0.;
    /*  double amc */

    Pw1  = co->a[4];
    Pw2  = co->a[3];
    Pw3  = co->a[1];
    /* amc  = co->a[12]; */
    am1  = co->a[13];
    am2  = co->a[14];
    am3  = co->a[15];
    p00  = co->q[10];
    p20  = co->q[11];
    p40  = co->q[12];
    p01  = co->q[17];
    p21  = co->q[18];
    p41  = co->q[19];
    aa   = co->a[9];
    xk0  = co->a[6];
    xk1  = co->a[11];
    pw11 = co->q[8];
    alpha= co->q[9];
    alhi = co->q[14];
    besq = co->a[8];

    dv.xk[0] = xk0;
    dv.xk[1] = xk1;

    if (fabs(T - cr->Tc) < to->FPTOL)
        T = cr->Tc;
    Tee     = (T - cr->Tc) / cr->Tc;
    dv.Tw  = - cr->Tc / T;
    dv.dTw = 1.0 + dv.Tw;
    if ( !isat )
    {                           /* when isat == 0 */
        rho = DD / cr->rhoC;
        conver(&rho, Tee, rho1, &rhodi, err);
    }
    else
    {                           /* when isat == 1 */
        par.th1 = -1.0;
        a2.th  = par.th1;
        par.r1  = dv.dTw / (1.0 - besq);
        a2.r   = par.r1;

        ss(par.r1, par.th1, dv.s, dv.sd);
        rho = par.th1 * (xk0 * pow(par.r1, co->a[5]) +
                         xk1 * pow(par.r1, co->q[15])) + co->a[0] * (dv.s[0] + dv.s[1]);
        rho += 1.0 + pw11 * dv.dTw;
        dv.amu = 0.0;
        DD = rho * cr->rhoC;
    }
    tt1 = par.th1 * par.th1;
    tt2 = tt1*tt1;
    Pw0  = 1.0 + dv.dTw * (Pw1 + dv.dTw * (Pw2 + dv.dTw * Pw3));
    if ( !isat )
        Pwmu = dv.amu * rhodi;              /* when isat == 0 */
    else
        Pwmu = 0.0;                          /* when isat == 1 */
    dv.p0th = p00 + p20 * tt1 + p40 * tt2;
    dv.p1th = p01 + p21 * tt1 + p41 * tt2;
    dPw0 = xk0 * dv.p0th * pow(par.r1,(2.0 - alpha));
    dPw1 = xk1 * dv.p1th * pow(par.r1,(2.0 - alhi));
    dPw  = aa * (dPw0 + dPw1);
    dv.Pw   = Pw0 + Pwmu + dPw;
    Pfind = dv.Pw * cr->Pcon * T;
    if (fabs(par.th1) < 1.0)
        sa.iphase = 1;
    else
    {
        sa.iphase = 2;
        dv.dP0dT = Pw1 + dv.dTw * (2.0 * Pw2 + 3.0 * Pw3 * dv.dTw);
        dv.dM0dT = am1 + dv.dTw * (2.0 * am2 + 3.0 * am3 * dv.dTw);
        Uw = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu + dv.s[0] + dv.s[1];
        dPdTcd = Uw + rho * dv.dM0dT;
        dPwdTw = dv.Pw - dv.Tw * dPdTcd;
        d2.dPdT   = cr->Pcon * dPwdTw;
    }

    aux(par.r1, par.th1, &dv.d2PdT2, &dv.d2PdMT, &dv.d2PdM2,
        aa, dv.xk, dv.sd, Cvcoex);
    if (sa.iphase == 1)
        d2.dPdD = cr->dPcon * DD * T / dv.d2PdM2;
    return(Pfind);
}

//--------------------------------------------------------------------//
/*   /coefs/ a[19], q[19], x[10]   */
auto  WaterHGKgems::aux(double r1, double th1, double *d2PdT2, double *d2PdMT,
                   double *d2PdM2, double aa, double *xk, double *sd, double Cvcoex) -> void
/* xk[2], sd[2] */
{
    int i= -1;
    double  sS[2], w[2], y[2], z[2], coex[2], cc, beta, besq, delta, alpha,
    s00, s20, s01, s21, deli, ww, yy, zz, gamma, tt1, ter, g, alhi,
    beti, gami, a1, a2, a4, f1;

    cc   = co->a[0];
    beta = co->a[5];
    besq = co->a[8];
    delta= co->a[10];
    alpha= co->q[9];
    s00  = co->a[16];
    s20  = co->a[17];
    s01  = co->a[18];
    s21  = co->a[19];

    deli  = 0.0;
    sS[0]   = s00 + s20 * th1 * th1;
    sS[1]   = s01 + s21 * th1 * th1;
    sd[0]  = 2.0 * th1 * s20;
    sd[1]  = 2.0 * th1 * s21;
    ww     = 0.0;
    yy     = 0.0;
    zz     = 0.0;
    gamma  = beta * (delta - 1.0);
    tt1    = th1 * th1;
    ter    = 2.0 * beta * delta - 1.0;
    g      = (1.0 + (besq * ter - 3.0) * tt1 - besq * (ter - 2.0) * tt1 * tt1);
    Cvcoex = 0.0;

    while ( ++i < 2 )
    {
        alhi    = alpha - deli;
        beti    = beta + deli;
        gami    = gamma - deli;
        if ( r1 )
        {
            w[i]  = (1.0 - alhi) * (1.0 - 3.0 * tt1) * sS[i] -
                    beta * delta * (1.0-tt1) * th1 * sd[i];
            w[i]  = (w[i] * pow(r1, -alhi)) / g;
            w[i]  = w[i] * xk[i];
            ww    = ww + w[i];
            y[i]  = beti * (1.0 - 3.0 * tt1) * th1 -
                    beta * delta * (1.0 - tt1) * th1;
            y[i]  = (y[i] * pow(r1, beti - 1.0)) * xk[i] / g;
            yy    = yy + y[i];
            z[i]  = 1.0-besq*(1.0-(2.0*beti))*tt1;
            z[i]  = (z[i]*pow(r1,-gami)) * xk[i] / g;
            zz    = zz + z[i];
            a1 = (beta * (delta - 3.0) - 3.0 * deli - besq * alhi * gami) /
                 (2.0 * besq * besq * (2.0 - alhi) * (1.0 - alhi) * alhi);
            a2 = 1 + ((beta * (delta - 3.0) - 3.0 * deli - besq * alhi
                       * ter) / (2.0 * besq * (1.0 - alhi) * alhi));
            a2 = -a2;
            a4 = 1.0 + ((ter - 2.0) / (2.0 * alhi));
            f1 = a1 + a2 + a4;
            coex[i] = ((2.0 - alhi) * (1.0 - alhi) * pow(r1, -alhi) * f1 * xk[i]);
            Cvcoex  += coex[i];
        }
        deli    = 0.5;
    }
    *d2PdT2 = aa * ww;
    *d2PdMT = yy + aa * cc * ww;
    *d2PdM2 = zz / aa + 2.0 * cc * yy + cc * cc * aa * ww;
}

//--------------------------------------------------------------------//
/* denLVS - Calculatin DH2O(T,P) or Dvap,Dliq(T,P) for equation
*           of state Levelt Sengers, et al (1983)             */
auto WaterHGKgems::denLVS(int isat, double T, double P) -> void
{
    int i=-1;
    double  /*s[2], sd[2],*/ Pnext, Pdif, delD, Dmin, Dmax, pw11,
    xk0, xk1, Tw, dTw, rho1, rho2;

    /*  EQUIVALENCE (Dmin, x(4)), (Dmax, x(5)), (pw11, q(9)),
        1            (xk0,  a(7)), (xk1,  a(12))  */

    Dmin =  co->x[3];
    Dmax =  co->x[4];
    pw11 =  co->q[8];
    xk0  =  co->a[6];
    xk1  =  co->a[11];

    if (isat == 0)
    {
        sa.DH2O = cr->rhoC;
        while (++i < 20 )
        {
            Pnext = Pfind(isat, T, sa.DH2O);
            Pdif  = Pnext - P;
            if (sa.iphase == 2)
            {
                if (fabs(Pdif) <= 0.0e0)
                    return;

                if (Pdif < 0.0e0)
                    sa.DH2O = Dmax;
                else
                    sa.DH2O = Dmin;
            }
            else
            {
                delD = -Pdif / d2.dPdD;
                sa.DH2O +=  delD;
                if (sa.DH2O < Dmin)
                    sa.DH2O = Dmin;
                if (sa.DH2O > Dmax)
                    sa.DH2O = Dmax;
                if (fabs(delD / sa.DH2O) < 1.0e-6)
                    return;
            }
        }
    }
    else
    {
        Tw   = - cr->Tc / T;
        dTw  = 1.0e0 + Tw;

        ss(par.r1, par.th1, dv.s, dv.sd);
        rho1 = 1.0e0 + pw11 * dTw + co->a[0] * (dv.s[0] + dv.s[1]);
        rho2 = xk0 * pow(par.r1, co->a[5]) + xk1 * pow(par.r1, co->q[15]);
        sa.Dvap = cr->rhoC * (rho1 - rho2);
        sa.Dliq = cr->rhoC * (rho1 + rho2);
    }
}

//--------------------------------------------------------------------//
// Calculated equation P(T) see Washburn (1924): Monthly Weather Rev.,
// v.52,   pp.488-490.
auto WaterHGKgems::Psublm(double Temp) -> double
{
    double TT, PmmHg, Psublm;

    TT = Temp + 2.731e2;
    PmmHg = pow(1.0e1, (-2.4455646e3 / TT + 8.2312e0 * 0.4342945 * log(TT) -
                        1.677006e-2 * TT + 1.20514e-5 * TT * TT - 6.757169e0));
    /*** convert mmHg to bars ***/
    Psublm = PmmHg * 1.33322e-3;
    return(Psublm);
}

//--------------------------------------------------------------------//
int  WaterHGKgems::valTD(double T, double D, int /*isat*/, int epseqn)
{
    int istemp;
    double TmnLVS, Tcrit, Ttripl, Dlimit, Tk, /* Ps, Dl, Dv, */
    /*P,*/ PMPa, Dguess, Dsublm, dPdD;

    TmnLVS= co->x[0];
    if ((T - to->FPTOL > hb->Ttop) || (T + to->FPTOL < hb->Tbtm) ||
            (D - to->FPTOL > hb->Dtop) || (D + to->FPTOL < hb->Dbtm))
        return(0);
    Tcrit = cr->Tc - 273.15e0;
    Ttripl = tt->Ttr - 273.15e0;
    if ((T + to->FPTOL >= Tcrit) || ((T >= li->TnIB30) && (D >= tt->Dltrip)))
    {
        Dlimit = li->sDIB30 * (T - li->TnIB30) + hb->Dtop;
        if ( !(D - to->FPTOL <= Dlimit ))
            return(0);
    }
    else
    {
        if (D - to->FPTOL <= tt->Dltrip)
        {
            if (T >= Ttripl)
            {
                Tk = T + 273.15e0;
                if (Tk < TmnLVS)
                {
                    a1.rt = ac->gascon * Tk;
                    pcorr(0,Tk,&trp.Ps,&trp.Dl,&trp.Dv,epseqn);
                }
                else
                {
                    istemp = 1;
                    sa.DH2O = 0.0e0;
                    trp.P    = Pfind(istemp, Tk, sa.DH2O);
                    denLVS(istemp, Tk, trp.P);
                    trp.Dv = sa.Dvap / 1.0e3;
                    trp.Dl = sa.Dliq / 1.0e3;
                }
                if ((D >= trp.Dv) && (D <= trp.Dl))
                    aSpc.isat = 1; //isat = 1;
            }

            else
            {
                trp.P = Psublm(T);
                PMPa = trp.P / 1.0e1;
                Tk = T + 273.15e0;
                Dguess = PMPa / Tk / 0.4e0;
                a1.rt = ac->gascon * Tk;
                bb(Tk);
                denHGK(&Dsublm,&PMPa,Dguess,Tk,&dPdD);
                if ( !( D - to->FPTOL <= Dsublm ))
                    return(0);
            }
        }
        else
        {
            if (D <= li->Dli13)
            {
                Dlimit = li->sDli1 * (T - li->Tli13) + li->Dli13;
                if ( !( D + to->FPTOL >= Dlimit ))
                    return(0);
            }
            else
            {
                Dlimit = li->sDli37 * (T - li->Tli13) + li->Dli13;
                if ( !( D - to->FPTOL <= Dlimit ))
                    return(0);
            }
        }
    }
    return(1);
}

//--------------------------------------------------------------------//
// Return 1, if T-P define liquid or vaper  H2O in dimention field of HKF
// else 0.
auto  WaterHGKgems::valTP(double T, double P) -> int
{
    double  Plimit, Psubl;

    if ((T - to->FPTOL > hb->Ttop) || (T + to->FPTOL < hb->Tbtm) ||
            (P - to->FPTOL > hb->Ptop) || (P + to->FPTOL < hb->Pbtm))
        return(0);

    if (P >= li->Pli13)
    {
        Plimit = li->sPli37 * (T - li->Tli13) + li->Pli13;
        if ( !( P - to->FPTOL <= Plimit ))
            return(0);
    }
    else
    {
        if (P >= tt->Ptripl)
        {
            Plimit = li->sPli1 * (T - li->Tli13) + li->Pli13;
            if ( !( P +to->FPTOL >= Plimit ))
                return(0);
        }
        else
        {
            Psubl = Psublm(T);
            if ( !( P - to->FPTOL <= Psubl ))
                return(0);
        }
    }
    return(1);
}


//--------------------------------------------------------------------//
// Check parametres of state H2O in critical region *eR=1 if yes, else *eR=0
// T,P, D insert in users units, returned in degK, MPa, kg/m3.
auto WaterHGKgems::crtreg(int isat, int iopt, int it, double *T, double *P,
                     double *D, int *eR) -> void
{
    int isat1;
    double Tmin1, Tmin2, Tmax, Dmin, Dmax, Pbase1,
    Pbase2, PTmins, PTmaxs, Pstest, ddummy,/* crtreg,*/
    Pmin, Pmax;
    /*    EQUIVALENCE (Tmin1,  x(1)),  (Tmin2, x(2)),  (Tmax,  x(3)),
       1            (Dmin,   x(4)),  (Dmax,  x(5)),
       2            (Pbase1, x(6)),  (Pbase2,x(7)),
       3            (PTmins, x(10)), (PTmaxs,x(11)) */
    Tmin1=  co->x[0];
    Tmin2= co->x[1];
    Tmax=  co->x[2];
    Dmin=  co->x[3];
    Dmax=  co->x[4];
    Pbase1= co->x[5];
    Pbase2= co->x[6];
    PTmins= co->x[9];
    PTmaxs= co->x[10];

    *T = TdegK(it, *T);     /* � ��ନ�㥬 T � degK */

    if (isat == 0)
    {
        if (iopt == 1)
        {
            *D *= un.fd * 1.0e3;
            if ( !( (*T >= Tmin1) && (*T <= Tmax) && (*D >= Dmin) && (*D <= Dmax) ))
            {
                *eR = 0;
                return;
            }
        }
        else
        {
            *P /= un.fp;
            if ((*T < Tmin1) || (*T > Tmax))
            {
                *eR = 0;
                return;
            }
            else
            {
                Pmin = Pbase1 + PTmins * (*T - Tmin1);
                Pmax = Pbase2 + PTmaxs * (*T - Tmin2);
                if ((*P >= Pmin) && (*P <= Pmax))
                {
                    *eR = 1;
                    return;
                }
                else
                {
                    if ((*P >= Pmin) && (*T <= Tmin2))
                    {
                        isat1 = 1;
                        ddummy = 0.0e0;
                        Pstest = Pfind(isat1, *T, ddummy);
                        if ( !(*P <= Pstest))
                        {
                            *eR = 0;
                            return;
                        }
                    }
                    else
                    {
                        *eR = 0;
                        return;
                    }
                }
            }
        }
    }
    else
    {
        if (iopt == 1)
        {
            if ( !(*T >= Tmin1))
            {
                *eR = 0;
                return;
            }
        }
        else
        {
            *P /= un.fp;
            if ( !(*P >= Pbase1))
            {
                *eR = 0;
                return;
            }
        }
    }
    *eR = 1;
}

//--------------------------------------------------------------------//
// T - ���������
auto WaterHGKgems::TsLVS(int isat, double Pres) -> double
{
    double TsLVS2, TsLVS, Dn, Pnext, dT;
    int i = 0;
    TsLVS2 = cr->Tc - 1.0e0;
    Dn     = cr->rhoC;
    do
    {
        Pnext = Pfind(isat, TsLVS2, Dn);
        dT = (Pnext - Pres) / d2.dPdT;
        TsLVS2 -= dT;
        if (TsLVS2 > cr->Tc)
            TsLVS2 = cr->Tc;
    }
    while( (++i > 20) || ( fabs(dT / TsLVS2) > 1.0e-8 ));
    TsLVS = TsLVS2;
    return(TsLVS);
}

//--------------------------------------------------------------------//
// backup - save COMMON values Pfind during saturation check).
auto WaterHGKgems::backup( struct STORE &sto ) -> void
{
    sto.isav1 = sa.iphase;
    sto.sav2  = par.r1;
    sto.sav3  = par.th1;
    sto.sav4  = dv.amu;
    sto.sav5  = dv.s[0];
    sto.sav6  = dv.s[1];
    sto.sav7  = dv.Pw;
    sto.sav8  = dv.Tw;
    sto.sav9  = dv.dTw;
    sto.sav10 = dv.dM0dT;
    sto.sav11 = dv.dP0dT;
    sto.sav12 = dv.d2PdM2;
    sto.sav13 = dv.d2PdMT;
    sto.sav14 = dv.d2PdT2;
    sto.sav15 = dv.p0th;
    sto.sav16 = dv.p1th;
    sto.sav17 = dv.xk[0];
    sto.sav18 = dv.xk[1];
    sto.sav19 = d2.dPdD;
}

//--------------------------------------------------------------------//
// restor - Restore COMMON  values Pfind after saturation check.
auto WaterHGKgems::restor( struct STORE sto ) -> void
{
    sa.iphase = sto.isav1;
    par.r1     = sto.sav2;
    par.th1    = sto.sav3;
    dv.amu    = sto.sav4;
    dv.s[0]   = sto.sav5;
    dv.s[1]   = sto.sav6;
    dv.Pw     = sto.sav7;
    dv.Tw     = sto.sav8;
    dv.dTw    = sto.sav9;
    dv.dM0dT  = sto.sav10;
    dv.dP0dT  = sto.sav11;
    dv.d2PdM2 = sto.sav12;
    dv.d2PdMT = sto.sav13;
    dv.d2PdT2 = sto.sav14;
    dv.p0th   = sto.sav15;
    dv.p1th   = sto.sav16;
    dv.xk[0]  = sto.sav17;
    dv.xk[1]  = sto.sav18;
    d2.dPdD   = sto.sav19;
}

//--------------------------------------------------------------------//
/* LVSsat - if isat=1, calculated Psat(T) or Tsat(P) (iopt=1,2).
*           if isat=0, check T-D or T-P (iopt=1,2) into TOL
*           if yes    isat <- 1  and  T <- Tsat.                  */
auto WaterHGKgems::LVSsat(int iopt, int isat, double *T, double *P, double *D)  -> void
{
    double   ERRTOL= 1.0e-12, TCTOL = 1.0e-2, Tsat;
    struct STORE sto; /*  backup() or restor() */
    if (isat == 1)
    {
        if (iopt == 1)
            *P = Pfind(isat, *T, *D);
        *T = TsLVS(isat, *P);
    }
    else
    {
        if (iopt == 1)
            *P = Pfind(isat, *T, *D);
        if ( !(*P-ERRTOL > cr->Pc))
        {
            backup( sto );
            Tsat = TsLVS(isat, *P);
            if (fabs(Tsat - *T) < TCTOL)
            {
                *T = Tsat;
                aSpc.isat = isat = 1;
            }
            else
                restor( sto );
        }
    }
}

//--------------------------------------------------------------------//
//thmLVS - calculation thermodinamic and transport fitches in critical
//       region H2O on equations of state Levelt Sengers, et al (1983).
auto WaterHGKgems::thmLVS(int isat, double T, double r1, double th1)  -> void
{
    /* double  sd[2];*/
    double pw1, pw11, pw2, pw3, amc, am1, am2, am3, aa, Hw, dPwdTw,
    dPdTal,/* xk0, xk1,*/ d2P0dT, d2M0dT, dPdT2, rho, Uw, Cvcoex=0.,
            CviTw2, Cvw, Cpw, Sw, Scond;
    /* double alhi, besq, alpha; */

    pw2  = co->a[3];
    pw3  = co->a[1];
    /*  besq = co->a[8]; */
    amc  = co->a[12];
    am1  = co->a[13];
    am2  = co->a[14];
    aa   = co->a[9];
    dv.xk[0]  = co->a[6];
    am3  = co->a[15];
    dv.xk[1]  = co->a[11];
    pw11 = co->q[8];
    /*  alpha= co->q[9]; */
    /* alhi = co->q[14]; */
    pw1  = co->a[4];

    d2P0dT = 2.0 * pw2 + 6.0 * pw3 * dv.dTw;
    d2M0dT = 2.0 * am2 + 6.0 * am3 * dv.dTw;
    dv.dP0dT  = pw1 + dv.dTw * (2.0 * pw2 + 3.0 * pw3 * dv.dTw);
    dv.dM0dT  = am1 + dv.dTw * (2.0 * am2 + 3.0 * am3 * dv.dTw);
    if (isat == 0)
    {
        rho = sa.DH2O / cr->rhoC;
        Uw  = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu +
              dv.s[0] + dv.s[1];
    }
    else
    {
        rho  = th1 * ( dv.xk[0] * pow(r1,co->a[5]) + dv.xk[1] *
                       pow(r1,co->q[15])) + co->a[0] * (dv.s[0] + dv.s[1]);
        rho += 1.0 + pw11 * dv.dTw;
        Uw   = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu +
               dv.s[0] + dv.s[1];
        sa.DH2O = rho * cr->rhoC;
        dPdT2    = dv.Pw - dv.Tw * (Uw + rho * dv.dM0dT);
        th.heat = 1.0e3 * T * (cr->Pcon * dPdT2) * (1.0 / sa.Dvap -
                  1.0 / sa.Dliq);

        ss(r1, th1, dv.s, dv.sd);
        aux(r1,th1, &dv.d2PdT2, &dv.d2PdMT, &dv.d2PdM2, aa,
            dv.xk, dv.sd, Cvcoex);
        if (r1 != 0.0)
            d2.dPdD = cr->dPcon * sa.DH2O * T / dv.d2PdM2;
    }
    if (r1 != 0.0)
    {
        a1.dPdTcd = dv.dP0dT + pw11 * (dv.amu -rho / dv.d2PdM2) +
                    dv.s[0] + dv.s[1] - dv.d2PdMT * rho / dv.d2PdM2;
        dPwdTw = dv.Pw - dv.Tw * a1.dPdTcd;
        dPdTal = cr->Pcon * dPwdTw;
        CviTw2 = d2P0dT - rho * d2M0dT + dv.d2PdT2 -
                 (pw11 + dv.d2PdMT) * (pw11 + dv.d2PdMT) / dv.d2PdM2;
        Cvw    = CviTw2 * dv.Tw * dv.Tw;
        Cpw    = Cvw + dv.d2PdM2 * dPwdTw * dPwdTw / (rho * rho);
        th.betaw  = 1.0 / (sa.DH2O * d2.dPdD);
        th.alphw  = th.betaw * dPdTal;
        th.Speed   = 1.0e3 * sqrt(Cpw / Cvw * d2.dPdD);
    }
    else
    {
        Cvw   = 1.0;
        Cpw   = 1.0;
        th.betaw = 1.0;
        th.alphw = 1.0;
        th.Speed = 0.0;
    }
    Hw = dv.Pw - dv.Tw * Uw;
    Sw = Hw - rho * (dv.amu + amc + dv.dTw *
                     (am1 + dv.dTw * (am2 + dv.dTw * am3)));
    Scond  = cr->Scon / sa.DH2O;
    th.U      = Uw * cr->Ucon / sa.DH2O;
    th.H      = Hw * Scond * T;
    th.Entrop = Sw * Scond;
    th.AE     = th.U - T * th.Entrop;
    th.GE     = th.H - T * th.Entrop;
    th.Cv     = Cvw * Scond;
    th.Cp     = Cpw * Scond;
}

/* *************************************************************** */
/*  dalLVS - Calc and return  (d(alpha)/dt)p(D,T,alpha) for equation
*            of state Levelt Sengers et al. (1983).
*            Comments:  D (kg/m**3),  T (degK),  P (MPa),
*                         alpha (degK**-1).                        */
auto WaterHGKgems::dalLVS(double D, double T, double P, double alpha) -> double
{
    int  i=-1;
    double s[2], dsdT[2], sp[2], dspdT[2], k[2], calpha[2], cbeta[2],
    cgamma[2], u[2], v[2], w[2], dudT[2], dvdT[2], dwdT[2];
    double dalLVS, a, c, delta, bsq, P11, Delta1, P2, P3, s00, s01, s20,
    s21, b1, b2, ar1, a01, ar2, a02, delT, q, amult, d0dT, drdT,
    dqdT, dP0dTT, ddelMT, dPdTT, dPdMMT, dPdMTT, dPPTT, pterm;
    /* double P1; */

    a     = co->a[9];
    c     = co->a[0];
    delta = co->a[10];
    bsq   = co->a[8];
    P11   = co->q[8];
    Delta1= co->q[13];
    /* P1    = co->a[4]; */
    P2    = co->a[3];
    P3    = co->a[1];
    s00   = co->a[16];
    s01   = co->a[18];
    s20   = co->a[17];
    s21   = co->a[19];

    if (a2.r == 0.0)
    {
        dalLVS = 1.0e6;
        return(dalLVS);
    }

    k[0]      = co->a[6];
    k[1]      = co->a[11];
    calpha[0] = co->q[9];
    calpha[1] = co->q[14];
    cbeta[0]  = co->a[5];
    cbeta[1]  = co->q[15];
    cgamma[0] = cbeta[0] * (delta - 1.0);
    cgamma[1] = cgamma[0] - Delta1;
    delT      = (T - cr->Tc) / T;
    s[0]      = s00 + s20 * pow(a2.th,2.);
    s[1]      = s01 + s21 * pow(a2.th,2.);
    sp[0]     = 2.0 * s20 * a2.th;
    sp[1]     = 2.0 * s21 * a2.th;
    /* ********************************************************************* */
    /* Compute drdT and d0dT from solution of the linear system              */
    /*                      ax = b                                           */
    /* d(dPdM)/dT = -D/Dc*alpha - P11*Tc/pow(T,2) = ar1*drdT + a01*d0dT = b1 */
    /* d(delT)/dT =           Tc/pow(T,2)         = ar2*drdT + a02*d0dT = b2 */
    /* ********************************************************************* */

    b1 = -D / cr->rhoC * alpha - P11 * cr->Tc / T / T;
    b2 =  cr->Tc / pow(T,2.);

    ar1 = 0.0;
    a01 = 0.0;

    while(++i < 2 )
    {
        ar1 = ar1 + k[i] * (cbeta[i] * a2.th * pow(a2.r,(cbeta[i]-1.0)) +
                            a * c * (1.0 - calpha[i]) * pow(a2.r,-calpha[i]) * s[i]);
        a01 = a01 + k[i] * (pow(a2.r,cbeta[i]) + a * c * sp[i] *
                            pow(a2.r,(1.0-calpha[i])));
    }
    ar2 = 1.0 - bsq * pow(a2.th,2.) - a * c * cbeta[0] * delta *
          (1.0 - pow(a2.th,2.)) * a2.th * pow(a2.r,(cbeta[0] * delta - 1.0));
    a02 = 3.0 * a * c * pow(a2.th,2.) * pow(a2.r,cbeta[0] * delta) -
          2.0 * bsq * a2.r * a2.th - a * c * pow(a2.r,cbeta[0] * delta);
    /********************************************************************/
    /* solve the linear system with simplistic GE w/ partial pivoting  */
    /********************************************************************/
    if (fabs(ar1) > fabs(ar2))
    {
        amult = -ar2 / ar1;
        d0dT  = (b2 + amult * b1) / (a02 + amult * a01);
        drdT  = (b1 - a01 * d0dT) / ar1;
    }
    else
    {
        amult = -ar1 / ar2;
        d0dT  = (b1 + amult * b2) / (a01 + amult * a02);
        drdT  = (b2 - a02 * d0dT) / ar2;
    }
    /********************************************************************/
    /*   Compute theta polynomials and their tempertaure derivatives  */
    dsdT[0]   = 2.0 * s20 * a2.th * d0dT;
    dsdT[1]   = 2.0 * s21 * a2.th * d0dT;
    dspdT[0]  = 2.0 * s20 * d0dT;
    dspdT[1]  = 2.0 * s21 * d0dT;
    q    = 1.0 + (bsq * (2.0 * cbeta[0] * delta - 1.0) - 3.0) *
           pow(a2.th,2.) - bsq * (2.0 * cbeta[0] * delta - 3.0)
           * pow(a2.th,4.);
    dqdT = 2.0 * (bsq * (2.0 * cbeta[0] * delta - 1.0) - 3.0) *
           a2.th * d0dT - 4.0 * bsq * (2.0 * cbeta[0] * delta -
                                       3.0) * pow(a2.th,3.) * d0dT;
    i=-1;
    while(++i < 2)
    {
        u[i]    = (1.0 - bsq * (1.0 - 2.0 * cbeta[i]) * pow(a2.th,2.)) / q;
        dudT[i] = (-2.0 * bsq * (1.0 - 2.0 * cbeta[i]) * a2.th * d0dT -
                   u[i] * dqdT) / q;
        v[i]    = ((cbeta[i] - cbeta[0] * delta) * a2.th +
                   (cbeta[0] * delta - 3.0 * cbeta[i]) * pow(a2.th,3.)) / q;
        dvdT[i] = ((cbeta[i] - cbeta[0] * delta) * d0dT +
                   3.0 * (cbeta[0] * delta - 3.0 * cbeta[i]) *
                   pow(a2.th,2.) * d0dT - v[i] * dqdT) / q;
        w[i]    = ((1.0 - calpha[i]) * (1.0 - 3.0 * pow(a2.th,2.)) *
                   s[i] - cbeta[0] * delta * (a2.th - pow(a2.th,3.)) *
                   sp[i]) / q;
        dwdT[i] = ((1.0 - calpha[i]) * ((1.0 - 3.0 * pow(a2.th,2.)) *
                                        dsdT[i] - 6.0 * a2.th * s[i] * d0dT) - cbeta[0] *
                   delta * ((a2.th - pow(a2.th,3.)) * dspdT[i] + sp[i] *
                            (d0dT - 3.0 * pow(a2.th,2.) * d0dT)) - w[i] * dqdT) / q;
    }
    /***************************************************************/
    /*   Compute dP0dTT, ddelMT, dPdTT, dPdMMT, dPdMTT, dPPTT      */

    dP0dTT = cr->Tc / pow(T,2.) * (2.0* P2 + 6.0 * P3 * delT);
    ddelMT = a * pow(a2.r,cbeta[0]*delta) * (cbeta[0]* delta *
             a2.th / a2.r * (1.0 - pow(a2.th,2.)) * drdT +
             (1.0 - 3.0 * pow(a2.th,2.)) * d0dT);
    dPdTT  = 0.0;
    dPdMMT = 0.0;
    dPdMTT = 0.0;
    i=-1;
    while (++i < 2)
    {
        dPdTT  = dPdTT + a * k[i] * (pow(a2.r,(1.0 - calpha[i])) *
                                     dsdT[i] + s[i] * (1.0 - calpha[i]) *
                                     pow(a2.r,-calpha[i]) * drdT);
        dPdMMT = dPdMMT + k[i] * ((pow(a2.r,(-cgamma[i])) * dudT[i] -
                                   u[i] * cgamma[i] * pow(a2.r,(-1.0 - cgamma[i]) * drdT)) /
                                  a + 2.0 * c * (pow(a2.r,(cbeta[i] - 1.0)) * dvdT[i] +
                                                 v[i] * (cbeta[i] - 1.0) * pow(a2.r,(cbeta[i] - 2.0)) * drdT)
                                  + a * pow(c,2.) * (pow(a2.r,(-calpha[i])) * dwdT[i] -
                                                     calpha[i] * w[i] * pow(a2.r,(-1.0 - calpha[i]) * drdT)));
        dPdMTT = dPdMTT + k[i] * (pow(a2.r,(cbeta[i] - 1.0)) * dvdT[i] +
                                  v[i] * (cbeta[i] - 1.0) * pow(a2.r,(cbeta[i] - 2.0)) *
                                  drdT + a * c * (pow(a2.r,(-calpha[i])) * dwdT[i] -
                                                  calpha[i] * pow(a2.r,(-1.0 - calpha[i])) * drdT * w[i]));
    }
    dPPTT = dP0dTT + dPdTT + P11 * ddelMT - D / cr->rhoC * dPdMTT /
            dv.d2PdM2 + (P11 + dv.d2PdMT) * (D / cr->rhoC * alpha /
                                             dv.d2PdM2 + D / cr->rhoC * dPdMMT / pow(dv.d2PdM2,2.));
    pterm = P / cr->Pc + a1.dPdTcd;

    /*          compute (d(alpha)/dT)P              */
    dalLVS  = cr->Tc * pow(cr->rhoC,2.) / pow(D,2.) / pow(T,2.) *
              (-2.0 / T * dv.d2PdM2 * pterm + 2.0 * alpha *
               dv.d2PdM2 * pterm + pterm * dPdMMT + dv.d2PdM2 * dPPTT);
    return(dalLVS);
}

//--------------------------------------------------------------------//
// translate parametrs into users units and load they into tprops.
auto WaterHGKgems::dimLVS(int isat, int itripl, double theta, double T, double *Pbars,
                     double *dL, double *dV, WPROPS *www, int epseqn) -> void
{
    double CpJKkg, betaPa, betab, dkgm3, pbars;
    pbars = *Pbars * 1.0e1;
    if (isat == 1)
    {
        *dV   = sa.Dvap;
        *dL   = sa.Dliq;
    }
    www->Aw = th.AE * un.fh;
    www->Gw  = th.GE * un.fh;
    www->Sw  = th.Entrop * un.fh * un.ft;
    www->Uw  = th.U * un.fh;
    www->Hw  = th.H * un.fh;
    www->Cvw = th.Cv * un.fh * un.ft;
    www->Cpw = th.Cp * un.fh * un.ft;
    www->Speedw = th.Speed * un.fs;
    www->Betaw = th.betaw / un.fp;
    www->Alphaw = th.alphw;
    /**************************************************************/
    a2.th = theta;
    www->dAldT = dalLVS(sa.DH2O,T, pbars/1.0e1,www->Alphaw); /*!!!!!*/
    /**************************************************************/
    CpJKkg  = th.Cp * 1.0e3;
    betaPa  = th.betaw / 1.0e6;
    betab   = th.betaw / 1.0e1;

    if (fabs(theta) != 1.0e0)
    {
        dkgm3 = sa.DH2O;
        www->Surtenw = 0.0e0;
    }
    else
    {
        if (theta < 0.0e0)
        {
            dkgm3 = sa.Dvap;
            www->Surtenw = 0.0e0;
        }
        else
        {
            dkgm3 = sa.Dliq;
            www->Surtenw = surten(T) * un.fst;
        }
    }
    Born92(T, pbars, dkgm3/1.0e3, betab, &www->Alphaw,
           &www->dAldT, &www->Dielw, &www->ZBorn,
           &www->QBorn, &www->YBorn, &www->XBorn, epseqn);
    www->Viscw  = viscos(T, pbars, dkgm3, betaPa) * un.fvd;
    www->Tcondw = thcond(T, pbars, dkgm3, www->Alphaw, betaPa) *
                  un.fc * un.ft;
    www->Tdiffw = www->Tcondw / un.fc / un.ft / (dkgm3 * CpJKkg) *
                  un.fvk;
    www->Prndtlw= www->Viscw / un.fvd * CpJKkg /
                  (www->Tcondw / un.fc / un.ft);
    www->Visckw = www->Viscw / un.fvd / dkgm3 * un.fvk;
    www->Albew = www->Alphaw / www->Betaw;

    if (itripl == 1)
        triple(T, www);
}

//--------------------------------------------------------------------//
//cpswap - load critical parametres A, G, U, H, S, Vs, Di,ZB
//from wpliq to wprops and aprocsimaze values - Cv, Cp, alpha, beta, visc,
//tcond, Prndtl, tdiff, visck, YB, QB, XB, daldT, st �� wprops � wpliq.
auto WaterHGKgems::cpswap() -> void
{
    wr.Aw     = wl.Aw;
    wr.Gw     = wl.Gw;
    wr.Sw     = wl.Sw;
    wr.Uw     = wl.Uw;
    wr.Hw     = wl.Hw;
    wr.Dielw  = wl.Dielw;
    wr.ZBorn  = wl.ZBorn;
    wr.Surtenw= wl.Surtenw;
    wl.Cvw    = wr.Cvw;
    wl.Cpw    = wr.Cpw;
    wl.Alphaw = wr.Alphaw;
    wl.Betaw  = wr.Betaw;
    wl.YBorn  = wr.YBorn;
    wl.QBorn  = wr.QBorn;
    wl.XBorn  = wr.XBorn;
    wl.Tcondw = wr.Tcondw;
    wl.Tdiffw = wr.Tdiffw;
    wl.Prndtlw= wr.Prndtlw;
    wl.dAldT  = wr.dAldT;
    wl.Albew  = wr.Albew;
    wr.Speedw = 0.429352766443498e2 * un.fs;
    wr.Viscw  = 1.0e6;
    wr.Visckw = 1.0e6;
    wl.Speedw = wr.Speedw;
    wl.Viscw  = wr.Viscw;
    wl.Visckw = wr.Visckw;
}

//--------------------------------------------------------------------//
// Calculation thermodinamic and transport water fitches critical region H2O
// ( 369.85 - 419.85  degC, 0.20-0.42 gm/cm3) see equat Levelt Sengers, et al
// (1983): J.Phys. Chem. Ref. Data, V.12, No.1, pp.1-28.
auto WaterHGKgems::LVSeqn(int isat, int iopt, int itripl, double TC,
                     double *P, double *Dens0, int /*epseqn*/) -> void
{
    int  cpoint=0, ioptsv=0, sW=0;
    double dL=0., dV=0., cdens=0.;

    /* cpoint = 0; */
    sa.DH2O = *Dens0;
    do
    {
        LVSsat(iopt, isat, &TC, P, &sa.DH2O);
        if ((isat != 0) || (iopt != 1))
            denLVS(isat, TC, *P);
        if ( !isat )
            *Dens0 = sa.DH2O;
        else
        {
            *Dens0 = sa.Dliq;
            aSta.Dens[1] = sa.Dvap;
        }
        if ( !isat )
        {
            thmLVS(isat, TC, par.r1, par.th1);
            dimLVS(isat, itripl, par.th1, TC, P, &dL, &dV, &wr, aSpc.epseqn);
            /*   iRET = change(wpr, wr); */
            if ( cpoint )
            {
                cpswap();
                *Dens0 = cdens;
                aSta.Dens[1] = cdens;
                aSpc.isat = isat = 1;
                aSpc.iopt = iopt = ioptsv;
            }
        }
        else
        {
            par.th1 = -1.0e0;
            thmLVS(isat, TC, par.r1, par.th1);
            dimLVS(isat, itripl, par.th1, TC, P, &dL, &dV, &wr, aSpc.epseqn);
            /*    iRET = change(wpr, wr);   */
            par.th1 =  1.0e0;
            thmLVS(isat, TC, par.r1, par.th1);
            dimLVS(isat, itripl, par.th1, TC, P, &dL, &dV, &wl, aSpc.epseqn);
            /*   iRET = change(wpl, wl); */
            if (dL == dV)
            {
                cpoint = 1;
                cdens  = dL;
                TC =  647.067000003e0;
                *P =  22.0460000008e0;
                ioptsv   = iopt;
                aSpc.iopt = iopt = 2;
                aSpc.isat = isat = 0;
                sW=1;
            }
        }
    }
    while ( sW != 0 );
}

//--------------------------------------------------------------------//
/* HGKsat - if isat=1, calculated Psat(T) or Tsat(P) (iopt=1,2).
*           if isat=0, check T-D or T-P (iopt=1,2) into TOL
*           if yes    isat <- 1  and  calc fitches                  */
auto WaterHGKgems::HGKsat(int& isat, int iopt, int itripl, double Temp,
                     double *Pres, double *Dens, int epseqn) -> void
{
    double  Ptemp=0., dltemp=0., dvtemp=0.;

    if (isat == 1)
    {
        if (iopt == 1)
            pcorr(itripl, Temp, Pres, Dens, &aSta.Dens[1], epseqn);
        else
            tcorr(itripl, &Temp, Pres, Dens, &aSta.Dens[1], epseqn);
    }
    else
    {
        if ((Temp > cr->Tc) || (Temp < tt->Ttr) ||
                ((iopt == 2) && (*Pres > cr->Pc)))
            return;                             /* ????????? */
        else
        {
            pcorr(itripl, Temp, &Ptemp, &dltemp, &dvtemp, epseqn);
            if (((iopt == 2) && (fabs(*Pres-Ptemp) <= to->PTOL)) ||
                    ((iopt == 1) && ((fabs(aSta.Dens[0]-dltemp) <= to->DTOL) ||
                                     (fabs(aSta.Dens[0] - dvtemp) <= to->DTOL))))
            {
                isat = 1;
                *Pres = Ptemp;
                aSta.Dens[0] = dltemp;
                aSta.Dens[1] = dvtemp;
            }
        }
    }
}

//--------------------------------------------------------------------//
// Calc dependent state values
auto WaterHGKgems::calcv3(int iopt, int itripl, double Temp, double *Pres,
                     double *Dens, int epseqn) -> void
{
    double ps=0., dll=0., dvv=0., dguess;

    if (iopt == 1)
    {
        resid(Temp, Dens);
        base(Dens, Temp);
        ideal(Temp);
        *Pres  = a1.rt * *Dens * ac->zb + qq.q0;
    }
    else
    {
        if (Temp < ac->tz)
        {
            pcorr(itripl, Temp, &ps, &dll, &dvv, epseqn);
        }
        else
        {
            ps   = 2.0e4;
            dll  = 0.0e0;
        }
        if (*Pres >  ps)
            dguess = dll;
        else
        {
            dguess = *Pres / Temp / 0.4e0;
        }
        denHGK(&aSta.Dens[0], Pres, dguess, Temp, &fct.dpdd);   /* Dens ???*/
        ideal(Temp);
    }
}

//--------------------------------------------------------------------//
// Calc dependent state values
auto WaterHGKgems::calcv2(int iopt, int itripl, double Temp, double *Pres,
                     double *Dens, int epseqn) -> void
{
    double ps=0., dll=0., dvv=0., dguess;

    if (iopt == 1)
    {
        resid(Temp, Dens);
        base(Dens, Temp);
        ideal(Temp);
        *Pres  = a1.rt * *Dens * ac->zb + qq.q0;
    }
    else
    {
        if (Temp < ac->tz)
        {
            pcorr(itripl, Temp, &ps, &dll, &dvv, epseqn);
        }
        else
        {
            ps   = 2.0e4;
            dll  = 0.0e0;
        }

        if (iopt == 3)
        {
            ps   = 2.0e4;
            dll  = 0.0e0;
        }
//        if (*Pres >  ps)
            dguess = dll;
//        else
//        {
//            dguess = *Pres / Temp / 0.4e0;
//        }
        denHGK(&aSta.Dens[1], Pres, dguess, Temp, &fct.dpdd);   /* Dens ???*/
        ideal(Temp);
    }
}

//--------------------------------------------------------------------//
// Calculation thermodinamic and transport H2O fitches for equation of
// state Haar, Gallagher, & Kell (1984).
auto WaterHGKgems::HGKeqn(int isat, int iopt, int itripl, double Temp,
                     double *Pres, double *Dens0, int epseqn) -> void
{
    a1.rt = ac->gascon * Temp;
    HGKsat(isat, iopt, itripl, Temp, Pres, Dens0, epseqn);
//    if (!aSpc.metastable)
//    {
//        if (isat == 0)
//        {
//            bb(Temp);
//            calcv3(iopt, itripl, Temp, Pres, Dens0, epseqn);
//            thmHGK(Dens0, Temp);
//            dimHGK(isat, itripl, Temp, Pres, Dens0, epseqn);
//        }
//        else
//        {
//            memcpy(&wl, &wr, sizeof(WPROPS));
//            dimHGK(2, itripl, Temp, Pres, &aSta.Dens[1], epseqn);
//        }
//    } else
    {
        bb(Temp);
        calcv2(iopt, itripl, Temp, Pres, &aSta.Dens[1], epseqn);
        thmHGK(&aSta.Dens[1], Temp);
        dimHGK(isat, itripl, Temp, Pres, &aSta.Dens[1], epseqn);

        memcpy(&wl, &wr, sizeof(WPROPS));

        calcv3(iopt, itripl, Temp, Pres, Dens0, epseqn);
        thmHGK(Dens0, Temp);
        dimHGK(isat, itripl, Temp, Pres, Dens0, epseqn);

        if ((aSta.Pres*10 > aSta.Psat) && (Temp < 0.647126e3) && (!aSpc.isat))
        {
            calcv2(3, itripl, Temp, Pres, &aSta.Dens[0], epseqn);
            thmHGK(&aSta.Dens[0], Temp);
            dimHGK(1, itripl, Temp, Pres, &aSta.Dens[0], epseqn);

            double temp = aSta.Dens[0];
            aSta.Dens[0] = aSta.Dens[1];
            aSta.Dens[1] = temp;

//            memcpy(&wl, &wr, sizeof(WPROPS));
//            calcv3(iopt, itripl, Temp, Pres, Dens0, epseqn);
//            thmHGK(Dens0, Temp);
//            dimHGK(isat, itripl, Temp, Pres, Dens0, epseqn);
        }

    }
}

//--------------------------------------------------------------------//
// translate t from deg to users units
auto WaterHGKgems::TdegUS(int it, double t) -> double
{
    double TdegUS=t;
    switch(it)
    {
    case 0:
        TdegUS = t;
        break;
    case 1:
        TdegUS = t - 273.15e0;
        break;
    case 2:
        TdegUS = t * 1.8e0;
        break;
    case 3:
        TdegUS = t * 1.8e0 - 459.67e0;
        break;
    }
    return(TdegUS);
}

//--------------------------------------------------------------------//
// conver - Transform T,D to r,theta  and scaled equations.
auto  WaterHGKgems::conver(double *rho,
                      double Tee, double rho1s, double *rhodi, double error1) -> void
{
    int isig=0;
    double error2;
    double /*sd[2]*/ beta, delta, xk1, cc, besq, p11, aa, xk0,  betai,
    Tstar, dtstin, rhoweg, rhodit, drho, rho1co, twofaz, hold,
    y1, den1, den2, den12, tt, rho1s2, slope;

    /* double alhi, alpha, deli, p1w, p2w, p4w, s00, s20; */
    beta  =co->a[5];
    delta =co->a[10];
    xk1   =co->a[11];
    cc    =co->a[0];
    /* alhi  =co->q[14];
       alpha =co->q[9];      */
    besq  =co->a[8];
    p11   =co->q[8];
    /* deli  =co->q[13];     */
    /* p1w =co->q[17];
       p2w   =co->q[18];
       p4w   =co->q[19];     */
    aa    =co->a[9];
    xk0   =co->a[6];
    /* s00 =co->a[16];       */
    /* s20 =co->a[17];       */
    betai =co->q[15];

    Tstar  = Tee + 1.0;
    dtstin = 1.0 - (1.0 / Tstar);
    par.r1     = dtstin;
    if (dtstin <= 0.0)
    {
        par.r1  = dtstin / (1.0 - besq);
        par.th1 = 1.0;
    }
    else
        par.th1 = 0.0;

    ss(par.r1, par.th1, dv.s, dv.sd);
    *rhodi   = 1.0 + p11 * dtstin;
    rhodit = *rhodi  + cc * dv.s[0] + cc * dv.s[1];
    drho   = *rho - rhodit;
    dv.amu    = 0.0;
    if (dtstin <= 0.0)
    {
        rho1co = xk0 * pow(par.r1,beta) + xk1 * pow(par.r1,betai);
        twofaz = rho1co;
        if (fabs(drho) <= twofaz)
        {
            //rho1s   = (rho1co * drho / fabs(drho)) + cc * dv.s[0];
            par.th1    = drho * fabs(drho);
            // error1 = 1.0;
            a2.r = par.r1;
            a2.th = par.th1;
            return;
        }
    }
    if ( !drho )
    {
        par.th1   = 0.0;
        par.r1    = dtstin;
        //rho1s  = cc * dv.s[0];
    }

    /*   rule for first pass   */
    y1   = dtstin;
    den1 = *rho - rhodit;

    rtheta(&par.r1, &par.th1, den1, y1);
    tt       = par.th1 * par.th1;
    dv.amu  = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
    y1       = dtstin + cc * dv.amu;

    ss(par.r1, par.th1, dv.s, dv.sd);
    rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
    rho1s   = den1 + cc * dv.s[0] + rhoweg;
    error1 = *rho - *rhodi  - rho1s ;
    a2.r  = par.r1;
    a2.th = par.th1;

    if (fabs(error1) < 1.0e-5)
        return;

    /*   rule for second pass  */

    den12 = *rho - *rhodi  - cc * dv.s[1] + rhoweg;
    if (den12 == den1)
        den12 = den1 - 1.0e-6;

    rtheta(&par.r1,&par.th1,den12,y1);
    tt      = par.th1 * par.th1;
    dv.amu = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
    y1      = dtstin + cc * dv.amu;

    ss(par.r1, par.th1, dv.s, dv.sd);
    rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
    rho1s2 = den12 + cc * dv.s[0] + rhoweg;
    error2 = *rho - *rhodi  - rho1s2;
    if (fabs(error2) <= 1.0e-5)
    {
        a2.r  = par.r1;
        a2.th = par.th1;
        //error1 = error2;
        // rho1s   = rho1s2;
        return;
    }

    /*    rule for nth pass   */

    den2   = den12;
    while( ++isig <= 10 )
    {
        slope  = (error2 - error1) / (den2 - den1);
        hold   = den2;
        den2   = den1 - (error1 / slope);

        rtheta(&par.r1,&par.th1,den2,y1);
        tt      = par.th1 * par.th1;
        dv.amu = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
        y1      = dtstin + cc * dv.amu;

        ss(par.r1, par.th1, dv.s, dv.sd);
        rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
        rho1s   = den2 + cc * dv.s[0] + rhoweg;
        error1 = error2;
        error2 = *rho - *rhodi  - rho1s ;
        a2.r  = par.r1;
        a2.th = par.th1;
        if (fabs(error2) < 1.0e-6)
            return;
        den1 = hold;
    }
}

//--------------------------------------------------------------------//
//  ss - calc terms of the summation zat defined dPotl/dT and 1-th
// proizvodnuyu theta (s)  square polynomial.
auto WaterHGKgems::ss(double r, double th, double *s, double *sd) -> void/*  s[2], sd[2] */
{
    double  sx[2];
    double alpha, beta, alhi, beti, s00, s20, s01, s21, tt;
    /* double besq, delta, deli, gami, p00, p01;*/

    alpha= co->q[9];
    beta = co->a[5];
    /* besq = co->a[8];  */
    /* delta= co->a[10];
    deli = co->q[13];    */
    alhi = co->q[14];
    beti = co->q[15];
    /* gami = co->q[16];
    p00  = co->q[10];    */
    /* p01  = co->q[17]; */
    s00  = co->a[16];
    s20  = co->a[17];
    s01  = co->a[18];
    s21  = co->a[19];

    tt    = th * th;
    sx[0] = s00 + s20 * tt;
    sd[0] = 2.0 * s20 * th;
    sx[1] = s01 + s21 * tt;
    sd[1] = 2.0 * s21 * th;
    s[0]  = sx[0] * co->a[9] * co->a[6] * pow(r,(1.0 - alpha));
    s[1]  = sx[1] * co->a[9] * co->a[11] * pow(r,(1.0 - alhi));

    a1.dPdM  = pow(r,beta) * co->a[6] * th + co->a[0] * pow(r,(1.0-alpha)) *
               co->a[9] * co->a[6] * sx[0] + pow(r,beti) * co->a[11] * th +
               co->a[0] * pow(r,(1.0-alhi)) * co->a[9] * co->a[11] * sx[1];
}

//--------------------------------------------------------------------//
// rtheta - Fits data for 1.0 < theta < 1.000001. Calc:
//                        rho = em*theta*(r**beta)
//                        Tee = r*(1.0-besq*theta*theta)
//  Routine given by Moldover (1978): Jour. Res. NBS, v. 84, n. 4, p. 329 - 334.
auto WaterHGKgems::rtheta(double *r, double *theta, double rho, double Tee) -> void
{
    int n=0;
    double  beta, em, besq, absrho, bee, z, z2, z3, c, dz, tr, tmr;

    beta = co->a[5];
    em   = co->a[6];
    besq = co->a[8];
    if (em <= 0.0 || besq <= 1.0)
        goto MET600;

    absrho = fabs(rho);
    if (absrho < 1.0e-12)
        goto MET600;

    bee = sqrt(besq);
    if (fabs(Tee) < 1.0e-12)
        goto  MET495;

    if (Tee < 0.0)
        z = 1.0-(1.0-bee) * Tee / (1.0 - besq) * pow(em / absrho,1.0 / beta);
    else
        z = pow(1.0 + Tee * pow(em / bee / absrho,1.0 / beta), - beta);

    if (z > 1.00234 * bee)
        goto MET496;

    tr = fabs(Tee);
    c  = -rho * bee / em / pow(tr,beta);
    z  = z * rho / fabs(rho);
    while ( ++n <= 16)
    {
        z2 = z * z;
        z3 = 1.0 - z2;
        tmr= fabs(z3);
        dz = z3 * (z + c * pow(tmr,beta)) / (z3 + 2.0 * beta * z2);
        z  = z - dz;
        if (fabs(dz/z) < 1.0e-12)
            goto MET498;
    }

MET601:
    if (fabs(*theta) > 1.0001)
        *theta /= fabs(*theta);
    return;

MET498:
    *theta = z / bee;
    *r     = Tee /(1.0 - z * z);
    *r     = fabs(*r);
    return;

MET495:
    *theta = rho / fabs(rho) / bee;
    *r     = pow(rho / (em * (*theta)),1.0 / beta);
    return;

MET496:
    *theta = rho / fabs(rho);
    *r     = Tee / (1.0 - besq);
    *r     = fabs(*r);
    return;

MET600:
    if (fabs(Tee) < 1.0e-12)
        goto MET601;

    if (Tee < 0.0)
        goto MET496;
    *theta = 1.0e-12;
    *r     = Tee;
}

//--------------------------------------------------------------------//
//   TdPsdT - return  T * (dPsat / dT).
auto WaterHGKgems::TdPsdT(double t_) -> double
{
//    int i=-1;
    double v, w, y, z, q, b = 0.0e0, c = 0.0e0, TdPsdT;

    double a[8]={-.78889166e1,  .25514255e1, -.6716169e1,  .33239495e2,
                 -.10538479e3,  .17435319e3, -.14839348e3, .48631602e2};
    v = t_ / 647.25e0;
    w = 1.0 - v;
    for( int i=0; i<=7; i++  ) // while (i++ <= 7)
    {
        z  = i;
        y  = a[i] * pow(w,(z + 1.0e0)/2.0e0);
        c += y / w * (0.5e0 - 0.5e0 * z - 1.0e0 / v);
        b += y;
    }
    q = b / v;
    TdPsdT = 22.093e0 * exp(q) * c;
    return(TdPsdT);
}

//--------------------------------------------------------------------//
//   TsHGK - return Tsaturation(P).
auto WaterHGKgems::TsHGK(double Ps_) -> double
{
    int k=0;
    double pl, tg, pp, dp, TsHGK;

    TsHGK = 0.0e0;
    if (Ps_ > 22.05e0)
        return(TsHGK);
    pl = 2.302585e0 + log(Ps_);
    tg = 372.83e0 + pl * (27.7589e0 + pl * (2.3819e0 +
                                            pl * (0.24834e0 + pl * 0.0193855e0)));
    /*  1   */
    while ( k++ <=  8)
    {
        if (tg < 273.15e0)
            tg = 273.15e0;
        if (tg > 647.00e0)
            tg = 647.00e0;
        pp = PsHGK(tg);
        dp = TdPsdT(tg);
        if( !(fabs(1.0e0 - pp/Ps_) < 1.0e-5 ) )
            tg *= (1.0e0 + (Ps_ - pp) / dp);
    }
    TsHGK = tg;
    return(TsHGK);
}

//--------------------------------------------------------------------//
/* tcorr - Calc T(P) saturation (t) and P (dL & dV) from  refinement
   of an initial approximation (TsHGK(p)) in accord with  Gl = Gv.  */
auto WaterHGKgems::tcorr(int itripl, double *t, double *p, double *dL, double *dV,
                    int epseqn) -> void
{
    double delg, dp;

    *t = TsHGK(*p);
    if (*t == 0.0e0)
        return;
    *dL = 0.0e0;
    *dV = 0.0e0;
    do
    {
        a1.rt = *t * ac->gascon;
        corr(itripl,*t,p,dL,dV,&delg,epseqn);
        dp = delg * ac->gascon * *t / (1.0e0 / *dV - 1.0e0 / *dL);
        *t *= (1.0e0 - dp / TdPsdT(*t));
    }
    while ( !(fabs(delg) > 1.0e-4) );
}

//--------------------------------------------------------------------//
// calc t/d properties H2O as ideal gas: equat see Woolley (1979).
auto WaterHGKgems::ideal(double t) -> void
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
    id.gi  = - (c[0] / tt + c[1]) * tl;
    id.hi  = (c[1] + c[0] * (1.0e0 - tl) / tt);
    id.cpi = c[1] - c[0] / tt;
    while ( ++i <= 17 )
    {
        emult = pow(tt,((double)i - 5.));
        id.gi  -= c[i] * emult;
        id.hi  += c[i] * (i-5) * emult;
        id.cpi += c[i] * (i-5) * (i-4) * emult;
    }
    id.ai  = id.gi - 1.0e0;
    id.ui  = id.hi - 1.0e0;
    id.cvi = id.cpi - 1.0e0;
    id.si  = id.ui - id.ai;
}

//--------------------------------------------------------------------//
// tpset - calculate values U, S, H, A, G in 3-y point (in J/g from
// Table 2,  Helgeson & Kirkham,  1974a)
auto WaterHGKgems::tpset() ->void
{
    double Utr, Str, Htr, Atr, Gtr;

    Utr = -15766.0e0;
    Str =  3.5144e0;
    Htr = -15971.0e0;
    Atr = -12870.0e0;
    Gtr = -13073.0e0;

    tt->Utri = Utr * un.fh;
    tt->Stri = Str * un.fh;
    tt->Htri = Htr * un.fh;
    tt->Atri = Atr * un.fh;
    tt->Gtri = Gtr * un.fh;

}

auto WaterHGKgems::bb(double t) -> void
{
    int i=0;
    double v[10];
    v[0] = 1.0e0;
    while(++i <= 9)
    {
        v[i] = v[i-1] * ac->tz / t;
    }
    el->b1   = bcn->bp[0] + bcn->bp[1] * log(1.0 / v[1]);
    el->b2   = bcn->bq[0];
    el->b1t  = bcn->bp[1] * v[1] / ac->tz;
    el->b2t  = 0.0e0;
    el->b1tt = 0.0e0;
    el->b2tt = 0.0e0;
    i=1;
    while ( ++i <= 9 )
    {
        el->b1   += bcn->bp[i] * v[i-1];
        el->b2   += bcn->bq[i] * v[i-1];
        el->b1t  -= (i-1) * bcn->bp[i] * v[i-1] / t;
        el->b2t  -= (i-1) * bcn->bq[i] * v[i-1] / t;
        el->b1tt += bcn->bp[i] *(i-1)*(i-1) * v[i-1] / t / t;
        el->b2tt += bcn->bq[i] * (i-1)*(i-1) * v[i-1] / t / t;
    }
    el->b1tt -= el->b1t / t;
    el->b2tt -= el->b2t / t;
}

//--------------------------------------------------------------------//
//calc density (d in g/cm3) and dP/dD (dPdd) as f(p(MPa), t(degK))
// from an initial density guess (dguess).
auto WaterHGKgems::denHGK(double *d, double *p, double dguess, double t, double *dpdd) -> void
{
    double pp, dp, dpdx, x;
    int  i= 0;
    *d  = dguess;
FIRST:
    do
    {
        i++;
        if (*d <= 0.0e0) *d = 1.0e-8;
        if (*d > 1.9e0)  *d = 1.9e0;
        resid(t,d);
        base(d,t);
        pp    = a1.rt * *d * ba.pb + qq.q0;
        *dpdd = a1.rt * (ac->zb + ac->yb * ac->dzb) + qq.q5;
        /*  if  dpdd < 0  assume d in 2-phase region and adjust accordingly */
        if (*dpdd > 0.0) goto OTHER;
        if (dguess >= 0.2967e0)   *d *= 1.02e0;
        if (dguess < 0.2967e0)    *d *= 0.98e0;
    }
    while (i <= 10);
    return;
OTHER:
    {
        dpdx = *dpdd * 1.1e0;
        if (dpdx < 0.1e0)
            dpdx = 0.1e0;
        dp   = fabs(1.0e0 - pp / *p);
        if ((dp < 1.0e-8) || ((dguess > 0.3e0) && (dp < 1.0e-7)) ||
                ((dguess > 0.7e0) && (dp < 1.0e-6)))
            return;
        x = (*p - pp) / dpdx;
        if (fabs(x)  >  0.1e0)
            x *= 0.1e0 / fabs(x);
        *d += x;
        if (*d <= 0.0e0)
            *d = 1.0e-8;
        if ( i <= 30 ) goto FIRST;
    }
}

auto WaterHGKgems::surten(double Tsatur) -> double
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
/* Born92 - calculated Z, Q, Y, X .
*           epseqn = 1 statement Helgeson-Kirkham (1974)
*           epseqn = 2 statement Pitzer (1983)
*           epseqn = 3 statement Uematsu-Franck (1980)
*           epseqn = 4 statement Johnson-Norton (1991) !!!
*           epseqn = 5 statement Archer-Wang (1990)      */
auto WaterHGKgems::Born92(double TK, double Pbars, double Dgcm3, double betab,
                     double *alphaK, double *daldT, double *eps, double *Z,
                     double *Q, double *Y, double *X, int epseqn) -> void
{
    //int iRET = 0;
    double TMAX, PMAX, TOL, TdegC, dedP, dedT, d2edT2;

    TMAX = 1000.0e0;
    PMAX = 5000.0;
    TOL  = 1.0e-3;

    *eps = 0.0e0;
    *Z   = 0.0e0;
    *Y   = 0.0e0;
    *Q   = 0.0e0;
    *X   = 0.0e0;
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
        JN91(TK, Dgcm3, betab, alphaK, daldT, eps, &dedP, &dedT,
             &d2edT2);
        epsBrn(eps, dedP, dedT, d2edT2, Z, Q, Y, X);
    }
}

//--------------------------------------------------------------------//
// din. viscosity H2O in kg/m*s, if Pbars in validity region
// specified  by the  initial if  statement Watson et al. (1980);
//See  statement 3.1-2 and 4.1-5 in Tabl. 1,6,8 in Sengers and Kamgar-Parsi (1984).
auto WaterHGKgems::viscos(double Tk, double Pbars, double Dkgm3, double betaPa) -> double
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
auto WaterHGKgems::thcond(double Tk, double Pbars, double Dkgm3, double alph,
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
/* triple - translate values U, S, H, A, G zero triple point
*  properties (Haar et al., 1984; Levelt Sengers et al., 1983) referenced
*  to triple  point properties data in Helgeson and   Kirkham, 1974a. */
auto WaterHGKgems::triple(double T, WPROPS  *wr) -> void
{
    double TS;

    wr->Sw    = wr->Sw + tt->Stri;
    TS        = T * wr->Sw - tt->Ttr * tt->Stri;
    wr->Gw = wr->Hw - TS + tt->Gtri;
    wr->Aw = wr->Uw - TS + tt->Atri;
    wr->Hw = wr->Hw + tt->Htri;
    wr->Uw = wr->Uw + tt->Utri;
}

//--------------------------------------------------------------------//
auto WaterHGKgems::pcorr(int itripl, double t, double *p, double *dL,
                    double *dV, int epseqn) -> void
{
    double dp;
    *p  = PsHGK(t);
    *dL= 0.0e0;
    *dV = 0.0e0;
    do
    {
        corr(itripl, t, p, dL, dV, &trp.delg, epseqn);
        dp   = trp.delg * ac->gascon * t / (1.0e0 / *dV - 1.0e0 / *dL);
        *p  += dp;
    }
    while ( fabs(trp.delg) > 1.0e-4 );
}

auto WaterHGKgems::resid(double t, double *d) -> void
{
    int i=0, l, j, k, km;
    double  qr[11], qt[10], qzr[9], qzt[9];
    double  e, /* q10,*/ q20, v, dfdt,/* q2a, q5t,*/ d2f, dpt, tex, /* fct,*/
    /*dadt,*/ ddz, zz, del, dex,/* qp,*/ qm, att, tx, tau, ex1, ex2;
    long double q10, fct, q5t, qp, dadt, q2a;
    qr[0]  = 0.0e0;
    qq.q5 = 0.0e0;
    qq.q0 = 0.0e0;
    res.ar = 0.0e0;
    dadt   = 0.0e0;
    res.cvr= 0.0e0;
    res.dpdtr = 0.0e0;
    e     = exp(-ac->aa * *d);
    q10   = (*d) * (*d) * e;
    q20   = 1.0e0 - e;
    qr[1] = q10;
    v     = ac->tz / t;
    qt[0] = t / ac->tz;

    while(++i < 10)
    {
        qr[i+1] = qr[i] * q20;
        qt[i]   = qt[i-1] * v;
    }
    for (i=0; i<=8; i++) /* EQUIVALENCE (qr(3), qzr(1)), (qt(2), qzt(1)) */
    {
        qzr[i] = qr[i+2];
        qzt[i]=qt[i+1];
    }                  /*  as resalt: qzr[0]=qr[2], ... qzr[8]=qr[10];
                                         qzt[0]=qt[1], ... qzt[8]=qt[9]; */
    i=-1;
    while(++i <  nc->nc)
    {
        k  = nc->ii[i] + 1;
        l  = nc->jj[i];
        zz = k;
        if (k == 1)
            qp = nc->g[i] * ac->aa * qr[1] * qzt[l-1];
        else
            qp = nc->g[i] * ac->aa * qzr[k-2] * qzt[l-1];

        qq.q0 += qp;
        qq.q5 += ac->aa * (2.0 / *d - ac->aa * (1.0 - e * (k-1) / q20)) * qp;
        res.ar += nc->g[i] * qzr[k-1] * qzt[l-1] / q10 / zz / a1.rt;
        dfdt    = pow(q20,((double)k)) * (1-l) * qzt[l] / ac->tz / k;
        d2f     = l * dfdt;
        dpt     = dfdt*q10 * ac->aa * k / q20;
        dadt   += nc->g[i] * dfdt;
        res.dpdtr += nc->g[i] * dpt;
        res.cvr   += nc->g[i] * d2f / ac->gascon;
    }
    qp  = 0.0e0;
    q2a = 0.0e0;
    j=35;
    while (++j <= 39 )
    {
        if ( nc->g[j] )
        {
            k   = nc->ii[j];
            km  = nc->jj[j];
            ddz = ad->adz[j-36];
            del = *d / ddz - 1.0e0;
            if (fabs(del) < 1.0e-10)
                del = 1.0e-10;
            ex1   = -ad->aad[j-36] * pow(del,(double)k);

            if (ex1 < to->EXPTOL)
                dex = 0.0e0;
            else
                dex = exp(ex1)  * pow(del,(double)km);

            att   = ad->aat[j-36];
            tx    = ad->atz[j-36];
            tau   = t / tx - 1.0e0;
            ex2   = -att * tau * tau;

            if (ex2 <=  to->EXPTOL)
                tex = 0.0e0;
            else
                tex = exp(ex2);
            q10   = dex * tex;
            qm    = km / del - k * ad->aad[j-36] * pow(del,((double)k-1));
            fct   = qm * *d * *d * q10 / ddz;
            q5t   = fct * (2.0e0 / *d + qm / ddz) - (*d / ddz) * (*d / ddz) *
                    q10 * (km / del / del + k * (k-1) * ad->aad[j-36] *
                           pow(del,((double)k-2)));
            qq.q5    += q5t * nc->g[j];
            qp        += nc->g[j] * fct;
            dadt      -= 2.0e0 * nc->g[j] * att * tau * q10 / tx;
            res.dpdtr -= 2.0e0 * nc->g[j] * att * tau * fct / tx;
            q2a       += t * nc->g[j] * (4.0e0 * att * ex2 + 2.0e0 *
                                         att) * q10 / tx / tx;
            res.ar    += q10 * nc->g[j] / a1.rt;
        }
    }
    res.sr   = -dadt / ac->gascon;
    res.ur   = res.ar + res.sr;
    res.cvr += q2a / ac->gascon;
    qq.q0  += qp;
}

//--------------------------------------------------------------------//
// calc  Abase, Gbase, Sbase, Ubase, Hbase, Cvbase,
// see  Haar , Gallagher & Kell (1979), eq(1)
/*  /ellcon/  /basef/   /aconst/  */
auto WaterHGKgems::base(double *d, double t) -> void
{
    double x, z0, dz0, bb2tt;

    ac->yb = .25e0 * el->b1 * *d;
    x      = 1.0e0 - ac->yb;
    z0     = (1.0e0 + el->g1 * ac->yb + el->g2 * ac->yb * ac->yb) / (x*x*x);
    ac->zb = z0 + 4.0e0 * ac->yb * (el->b2 / el->b1 - el->gf);
    dz0    = (el->g1 + 2.0e0 * el->g2 * ac->yb) / (x * x * x) + 3.0e0 *
             (1.0e0 + el->g1 * ac->yb + el->g2 * ac->yb * ac->yb) / (x*x*x*x);
    ac->dzb= dz0 + 4.0e0 * (el->b2 / el->b1 - el->gf);
    ba.pb = ac->zb;
    ba.ab = -log(x) - (el->g2 - 1.0e0)/x + 28.16666667e0 / x / x +
            4.0e0* ac->yb *(el->b2 / el->b1 - el->gf) + 15.166666667e0 +
            log( *d * t * ac->gascon / .101325e0);
    ba.gb = ba.ab + ac->zb;
    ba.ub = -t * el->b1t * (ac->zb - 1.0e0 - *d * el->b2) / el->b1
            - *d * t * el->b2t;
    ba.sb = ba.ub - ba.ab;
    ba.hb = ac->zb + ba.ub;
    bb2tt  = t * t * el->b2tt;
    ba.cvb= 2.0e0 * ba.ub + (z0 - 1.0e0) * (((t * el->b1t / el->b1)
                                            * (t * el->b1t / el->b1)) - t * t * el->b1tt / el->b1)
            - *d * (bb2tt - el->gf * el->b1tt * t * t) -
            (t * el->b1t / el->b1) * (t * el->b1t / el->b1) * ac->yb * dz0;
    ba.dpdtb = ba.pb / t + *d * (ac->dzb * el->b1t / 4.0e0 + el->b2t -
                                 el->b2 / el->b1 * el->b1t);
}

//--------------------------------------------------------------------//
// calc t/d functions for HGK ( Helmholtz, Gibbs, et all)
auto WaterHGKgems::thmHGK(double *d, double t) -> void
{
    double  z;

    z = ac->zb + qq.q0 / a1.rt/ *d;
    fct.dpdd = a1.rt * (ac->zb + ac->yb * ac->dzb) + qq.q5;
    fct.ad   = ba.ab + res.ar + id.ai - ac->uref / t + ac->sref;
    fct.gd   = fct.ad + z;
    fct.ud   = ba.ub + res.ur + id.ui - ac->uref / t;
    fct.dpdt = a1.rt * *d * ba.dpdtb + res.dpdtr;
    fct.cvd  = ba.cvb + res.cvr + id.cvi;
    fct.cpd  = fct.cvd + t * fct.dpdt * fct.dpdt /
               (*d * *d * fct.dpdd * ac->gascon);
    fct.hd   = fct.ud + z;
    fct.sd   = ba.sb + res.sr + id.si - ac->sref;
    fct.dvdt = fct.dpdt / fct.dpdd / *d / *d;
    fct.cjtt = 1.0e0 / *d - t * fct.dvdt;
    fct.cjth = -fct.cjtt / fct.cpd / ac->gascon;
}

//--------------------------------------------------------------------//
auto WaterHGKgems::dimHGK(int isat,
                     int itripl, double t, double *p, double *d, int epseqn) -> void
{
    double pbars, dkgm3, betaPa, betab, CpJKkg;

    wr.Aw   = fct.ad * a1.rt * un.fh;
    wr.Gw   = fct.gd * a1.rt * un.fh;
    wr.Sw   = fct.sd * ac->gascon * un.fh * un.ft;
    wr.Uw   = fct.ud * a1.rt * un.fh;
    wr.Hw   = fct.hd * a1.rt * un.fh;
    wr.Cvw  = fct.cvd * ac->gascon * un.fh * un.ft;
    wr.Cpw  = fct.cpd * ac->gascon * un.fh * un.ft;
    wr.Speedw = sqrt(fabs(fct.cpd * fct.dpdd * 1.0e3 /
                          fct.cvd)) * un.fs;
    wr.Betaw  = 1.0e0 / (*d * fct.dpdd * un.fp);
    wr.Alphaw = *d * fct.dvdt;
    wr.dAldT  = dalHGK(d,t,wr.Alphaw);                 /*!!!*/
    pbars = *p * 1.0e1;
    dkgm3 = *d * 1.0e3;
    betaPa = wr.Betaw * un.fp / 1.0e6;
    betab  = wr.Betaw * un.fp / 1.0e1;
    CpJKkg = wr.Cpw / un.fh / un.ft * 1.0e3;
    wr.Viscw  = viscos(t,pbars,dkgm3,betaPa) * un.fvd;
    wr.Tcondw = thcond(t,pbars,dkgm3,wr.Alphaw,betaPa) * un.fc * un.ft;

    if ((isat == 0) || (isat == 2))
        wr.Surtenw = 0.0e0;
    else
        wr.Surtenw = surten(t) * un.fst;

    Born92(t,pbars,dkgm3/1.0e3,betab,&wr.Alphaw,&wr.dAldT,
           &wr.Dielw,&wr.ZBorn,&wr.QBorn,&wr.YBorn,&wr.XBorn,epseqn);
    wr.Tdiffw = wr.Tcondw / un.fc / un.ft / (dkgm3 * CpJKkg) * un.fvk;
    if ( wr.Tcondw )
        wr.Prndtlw = wr.Viscw / un.fvd * CpJKkg /
                     (wr.Tcondw / un.fc / un.ft);
    else
        wr.Prndtlw = 0.0e0;
    wr.Visckw = wr.Viscw / un.fvd / dkgm3 * un.fvk;
    wr.Albew  = wr.Alphaw / wr.Betaw;

    if (itripl == 1)
        triple(t, &wr);
}

auto WaterHGKgems::PsHGK(double t) -> double
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

//--------------------------------------------------------------------//
// (dliq & dvap),   (Gl-Gv)/RT  (delg)
auto  WaterHGKgems::corr(int itripl, double t, double *p, double *dL,
                    double *dV, double *delg, int epseqn) -> void
{
    double dguess, gl, gv;
    bb(t);
    dguess = *dL;
    if (*dL <= 0.0e0)
        dguess = 1.11e0 - 0.0004e0 * t;
    denHGK(dL,p,dguess,t, &fct.dpdd);
    ideal(t);
    thmHGK(dL,t);
    dimHGK(1,itripl,t,p,dL,epseqn);
    gl     = fct.gd;
    dguess = *dV;
    if (*dV <= 0.0e0)
        dguess = *p / a1.rt;
    denHGK(dV,p,dguess,t,&fct.dpdd);
    if (*dV < 5.0e-7)
        *dV = 5.0e-7;
    ideal(t);
    thmHGK(dV,t);
/*    FCTS
     *      ୮  (dimensionless); after
     *    pcorr's final call of corr (*delg < 10d-4)   */
    gv    = fct.gd;
    *delg = gl - gv;
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
auto WaterHGKgems::JN91(double T, double D, double beta, double *alpha, double *daldT,
                   double *eps, double *dedP, double *dedT, double *d2edT2) -> void
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

    *eps = 0.0e0;

    while (++k <= 5)
    {
        *eps += c[k-1] * pow(D,(double)(k - 1));
    }
    *dedP = 0.0e0;
    while (++j <=  4)
    {
        *dedP += j * c[j] * pow(D,(double)j);
    }
    *dedP *= beta;
    *dedT = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        *dedT += pow(D,(double)j) * (dcdT[j] - j * *alpha * c[j]);
    }
    *d2edT2 = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        *d2edT2 += pow(D,(double)j) * (dc2dTT[j] - j * (*alpha *
                                       dcdT[j] + c[j] * *daldT) - j * *alpha *
                                       (dcdT[j] - j * *alpha * c[j]));
    }
}

//--------------------------------------------------------------------//
/* epsBrn - calc Brown functions Z, Q, Y, and X from their eps, dedP, dedT
   and d2edT2 counterparts.           */
auto WaterHGKgems::epsBrn(double *eps, double dedP, double dedT,double d2edT2,
                     double *Z, double *Q, double *Y, double *X) -> void
{
    *Z = -1.0e0 / *eps;
    *Q =  1.0e0 / pow(*eps,2.) * dedP;
    *Y =  1.0e0 / pow(*eps,2.) * dedT;
    *X =  1.0e0 / pow(*eps,2.) * d2edT2 - 2.0e0 * *eps * pow(*Y,2.);
}

auto WaterHGKgems::dalHGK(double *d, double t, double alpha) -> double
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
    ac->yb= .25e0 * el->b1 * *d;
    x     = 1.0e0 - ac->yb;
    dydtp = (*d / 4.0e0) * (el->b1t - el->b1 * alpha);
    dbdd  = ac->gascon * t * ((el->b1 / 4.0e0 / x) * (1.0e0 -
                              (el->g2 - 1.0e0) / x + (el->g1 + el->g2 + 1.0e0) /
                              x / x) + el->b2 - el->b1 * el->gf + 1.0e0 / *d);
    db2dd = ac->gascon * t * ((el->b1 * el->b1 / 16.0e0 / x / x) *
                              (1.0e0 - 2.0e0 * (el->g2 - 1.0e0) / x + 3.0e0 *
                               (el->g1 + el->g2 + 1.0e0) / x / x) - 1.0e0 / *d / *d);
    db2ddt = ac->gascon * t * ((el->b1t / 4.0e0 / x / x) *
                               (1.0e0 - (el->g2 - 1.0e0) * (1.0e0 + ac->yb) / x +
                                (el->g1 + el->g2 + 1.0e0) * (1.0e0 + 2.0e0 * ac->yb) /
                                x / x) + el->b2t - el->gf * el->b1t) + dbdd / t;
    db2dtp = dbdd / t + ac->gascon * t * ((el->b1 * dydtp / 4.0e0 /
                                           x / x / x) * (1.0e0 - el->g2 + 2.0e0 * (el->g1 +
                                                                                   el->g2 + 1.0e0) / x) + ((x * el->b1t + el->b1 *
                                                                                                            dydtp) / 4.0e0 / x / x) * (1.0e0 - (el->g2 - 1.0e0) /
                                                                                                                                       x + (el->g1 + el->g2 + 1.0e0) / x / x) + el->b2t -
                                          el->gf * el->b1t + alpha / *d );
    db3ddt = db2dd / t + ac->gascon * t * ((el->b1 * el->b1 *
                                            dydtp / 8.0e0 / x / x / x / x) * (1.0e0 - el->g2 +
                                                                              3.0e0 * (el->g1 + el->g2 + 1.0e0) / x) + (el->b1 *
                                                                                      (x * el->b1t + el->b1 * dydtp) / 8.0e0 / x / x / x) *
                                           (1.0e0 - 2.0e0 * (el->g2 - 1.0e0) / x + 3.0e0 *
                                            (el->g1 + el->g2 + 1.0e0) / x / x) - 2.0e0 * alpha /
                                           *d / *d );
    db3dtt = (db2ddt - dbdd / t) / t + ac->gascon * t * ((el->b1t *
             dydtp / 2.0e0 / x / x / x / x) * (1.0e0 - el->g2 +
                                               (el->g1 + el->g2 + 1.0e0) * (2.0e0 + ac->yb) / x) +
             ((x * el->b1tt + 2.0e0 * el->b1t * dydtp) / 4.0e0 /
              x / x / x) * (1.0e0 - (el->g2 - 1.0e0) * (1+ ac->yb) /
                            x + (el->g1 + el->g2 + 1.0e0) * (1.0e0 + 2.0e0 * ac->yb) /
                            x / x) + el->b2tt - el->gf * el->b1tt ) + (t * db2dtp -
                                    dbdd) / t / t;
    /***********************************************************/

    /*   evaluate derivatives for the residual function  */

    /*      drdd   = q/d/d                  */
    /*      dr2dd  = (q5 - 2.0e0/d*q0)/d/d   */
    /*      dr2ddt = dpdtr/d/d              */

    e1  = exp(-ac->aa * *d);
    e2  = 1.0e0 - e1;
    tzt = ac->tz / t;
    drdd   = 0.0e0;
    dr2dd  = 0.0e0;
    dr2ddt = 0.0e0;
    dr2dtp = 0.0e0;
    dr3ddt = 0.0e0;
    dr3dtt = 0.0e0;
    /*    evaluate terms 1-36      */
    while (++i < nc->nc)
    {
        k = nc->ii[i] + 1.0e0;
        l = nc->jj[i] - 1.0e0;
        km = k - 1.0e0;
        //   lm = l - 1.0e0;
        /*  kp = k + 1.0e0; */
        lp = l + 1.0e0;
        xtzt = pow(tzt,l);
        drdd   = drdd + nc->g[i] * xtzt * pow(e2,km) * e1;
        dr2dd  = dr2dd + nc->g[i] * e1 * xtzt * pow(e2,km) *
                 (km * e1 / e2 - 1.0e0);
        dr2ddt = dr2ddt - nc->g[i] * e1 * l * pow(e2,km) * pow(tzt,lp) /
                 ac->tz;
        dr2dtp = dr2dtp + nc->g[i] * e1 * pow(e2,km) * xtzt *
                 ( *d * alpha - l / t - km * e1 * *d * alpha / e2 );
        dr3ddt = dr3ddt + nc->g[i] * (km * *d * alpha * e1 * e1 * xtzt *
                                      pow(e2,(k-3.0e0)) + e1 * xtzt * pow(e2,km) * (km*e1/e2 -
                                              1.0e0) * (*d * alpha - l / t - km * *d * alpha * e1 / e2) );
        dr3dtt = dr3dtt + nc->g[i] * l * e1 * pow(e2,km) * pow(tzt,lp) /
                 ac->tz * (lp / t + *d * alpha * km * e1 / e2 - *d * alpha );
    }

    /*   evaluate terms 37-40   */

    i=35;
    while (++i <= 39)
    {
        k  = nc->ii[i];
        l  = nc->jj[i];
        km = k - 1.0e0;
        lm = l - 1.0e0;
        /*   kp = k + 1.0e0; */
        //  lp = l + 1.0e0;
        ai = ad->aad[i-36];
        bi = ad->aat[i-36];
        di = ad->adz[i-36];
        ti = ad->atz[i-36];
        tau = t / ti - 1.0e0;
        del = *d / di - 1.0e0;

        if (fabs(del) < 1.0e-10)
            del = 1.0e-10;
        ex1 = -ai * pow(del,k);
        if (ex1 < to->EXPTOL)
            dex = 0.0e0;
        else
            dex = exp(ex1);
        ex2  = -bi * tau * tau;
        if (ex2 <= to->EXPTOL)
            tex = 0.0e0;
        else
            tex = exp(ex2);
        ex12    = dex * tex;
        qm      = l / del - k * ai * pow(del,km);
        xdell   = pow(del,l);
        xdelk   = pow(del,k);
        drdd   += nc->g[i] * xdell * ex12 / di * qm;

        dr2dd  += nc->g[i] * xdell * ex12 / di / di * (qm * qm -
                  l / di / di - ai * k * km * pow(del,k-2.0e0));

        dr2ddt -= nc->g[i] * 2.0e0 * bi * tau * ex12 * xdell / ti / di * qm;

        dr2dtp += nc->g[i] / di * (*d * alpha * xdell * ex12 / di / del/del *
                                   (l + ai * k * km * xdelk) + qm * ( ex12 * (xdell* (k * ai *
                                                                      *d * alpha * pow(del,km) / di - 2.0e0 * bi * tau / ti) - l *
                                                                      *d * alpha * pow(del,lm)/di)));

        term1 = l * *d * alpha / di / del / del + ai * k * km * *d * alpha *
                pow(del,k-2.) / di;
        term2 = 2.* qm * term1 - 2.0e0 * l * *d * alpha / di / del / del /
                del + ai * k * km * (k-2.) * pow(del,k-3.) * *d * alpha / di;
        term3 = qm * qm - l / del / del - ai * k * km * pow(del,k-2.);
        term4 = ai * k * pow(del,k-1.) * *d * alpha / di - 2.0e0 * bi * tau / ti;
        term5 = ex12 * xdell * term4 - ex12 * l * pow(del,l-1.) * *d * alpha / di;

        dr3ddt += nc->g[i] / di / di * (xdell * ex12 * term2 + term3 * term5);

        dr3dtt -= 2.0e0 * nc->g[i] * bi / ti / di * (tau * xdell * ex12 * *d *
                  alpha / del / del / di * (l + ai * k * km * pow(del,k)) + qm *
                  (xdell * ex12 * (ai * k * *d * alpha * tau * pow(del,km) / di +
                                   (1.0e0 - 2.0e0 * bi * tau * tau) / ti - tau * l * *d * alpha /
                                   di / del )));
    }

    /*   compute (d(alpha)/dT)P   */

    dalHGK = ((db3dtt + dr3dtt) * (2.0e0 * (dbdd + drdd) + *d * (db2dd +
                                   dr2dd)) - (db2ddt + dr2ddt) * (2.0e0 * (db2dtp + dr2dtp) +
                                                                  *d * (db3ddt + dr3ddt) - *d * alpha * (db2dd + dr2dd))) /
             (2.0e0 * (dbdd + drdd) + *d * (db2dd + dr2dd)) / (2.0e0 *
                     (dbdd + drdd) + *d * (db2dd + dr2dd));

    return(dalHGK);
}

}

