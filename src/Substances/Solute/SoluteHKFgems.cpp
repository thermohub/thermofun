#include "Substances/Solute/SoluteHKFgems.h"
#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"


namespace ThermoFun {

//void TDComp::calc_thkf( AQSREF& arf, double P, double T, double Dw, double betaw,
//                        double alphaw, double daldTw, double Z, double Q,
//                        double Y, double X, int geqn)

const double
ZPrTr = -0.1278034682e-1,
        YPrTr = -0.5798650444e-4,
                Pref = 0.1e1,
                       Tref = 0.29815e3,
                              eta = 0.166027e6,
                                    theta = 0.228e3,
                                            psi = 0.26e4,
                                                  gref = 0.0e0;


auto thermoPropertiesAqSoluteHKFgems(Reaktoro_::Temperature TC, Reaktoro_::Pressure Pbar, Substance subst, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes, const PropertiesSolvent &wp) -> ThermoPropertiesSubstance
{
    // Get the HKF thermodynamic data of the species
    auto hkf = subst.thermoParameters().HKF_parameters;
    auto refProp = subst.thermoReferenceProperties();

    auto T = Reaktoro_::Temperature (TC.val+C_to_K);

    if (hkf.size() == 0)
    {
        Exception exception;
        exception.error << "Error in HKFgems EOS";
        exception.reason << "The HKF paramteres for "<< subst.symbol() << " are not defined or are not correclty initialized.";
        exception.line = __LINE__;
        RaiseError(exception);
    }
    if (hkf[0] == 0.0 || hkf[1] == 0.0 || hkf[2] == 0.0)
    {
        Exception exception;
        exception.error << "Error in HKFgems EOS";
        exception.reason << "The HKF paramteres for "<< subst.symbol() << " are not defined or are not correclty initialized.";
        exception.line = __LINE__;
        RaiseError(exception);
    }

//    auto Pbar = P * 1e-05; // Pa to bar

    // Auxiliary variables
//    const auto Pbar = P * 1.0e-05;
//    const auto Tr   = referenceTemperature;
//    const auto Pr   = referencePressure;
//    const auto Zr   = referenceBornZ;
//    const auto Yr   = referenceBornY;
    const auto Gf   = refProp.gibbs_energy / cal_to_J;
    const auto Hf   = refProp.enthalpy / cal_to_J;
    const auto Sr   = refProp.entropy / cal_to_J;
    const auto a1   = hkf[0];
    const auto a2   = hkf[1];
    const auto a3   = hkf[2];
    const auto a4   = hkf[3];
    const auto c1   = hkf[4];
    const auto c2   = hkf[5];
    const auto wr   = hkf[6];
    const auto W    = aes.w;
    const auto dwdT   = aes.wT;
    const auto dwdP   = aes.wP*1e05; // from 1/Pa to 1/bar
    const auto d2wdT2 = aes.wTT;
    const auto Z    = wes.bornZ;
    const auto Y    = wes.bornY;
    const auto Q    = wes.bornQ*1e05; // from 1/Pa to 1/bar
    const auto X    = wes.bornX;

    auto VQterm = 0.4184004e2 * (-W * Q + (-Z - 1.0) * dwdP);

    // the leading constant converts cal/(mol*bar) -> cm3/mol

    auto V   = 0.4184004e2 * (a1 + a2 / (psi + Pbar) + a3 /
                            ( T - theta) + a4 / (psi + Pbar) / (T - theta)) + VQterm;

    auto SYterm = W * Y - (-Z - 1.0) * dwdT - wr * YPrTr;
    auto S = Sr + c1 * log(T / Tref) - c2 / theta
                * (1.0 / (T - theta) - 1.0 / (Tref - theta) + (1.0 / theta)
                * log(Tref * (T - theta) / T / (Tref - theta))) + (a3
                * (Pbar - Pref) + a4 * log((psi + Pbar) / (psi+Pref)))
                * pow((1.0 / (T-theta)),2.) + SYterm;

    auto CpXtrm = W * T * X + 2.0 * T * Y * dwdT + T * (Z + 1.0) * d2wdT2;
    auto Cp = c1 + c2 / pow((T - theta),2.) - (2.0 * T
                / pow((T-theta),3.)) * (a3 * (Pbar - Pref) + a4
                * log((psi + Pbar) / (psi + Pref))) + CpXtrm;

    auto HYterm = W * (-Z - 1.0) + W * T * Y - T * (-Z - 1.0) *dwdT
                - wr * (-ZPrTr - 1.0) - wr * Tref * YPrTr;
    auto H = Hf + c1 * (T - Tref) - c2 * (1.0
                / (T - theta) - 1.0 / (Tref - theta)) + a1 * (Pbar
                - Pref) + a2 * log((psi + Pbar) / (psi + Pref))
                + (a3 * (Pbar - Pref) + a4 * log((psi + Pbar)
                / (psi + Pref))) * ((2.0 * T - theta) / pow((T - theta),2.))
                + HYterm;

    auto GZterm = W * (-Z - 1.0e0) - wr * (-ZPrTr - 1.0e0) + wr
                 * YPrTr * (T - Tref);
    auto G = Gf - Sr * (T - Tref) - c1 * (T * log(T
                / Tref) - T + Tref) + a1 * (Pbar - Pref) + a2
                * log((psi + Pbar) / (psi + Pref)) - c2 * ((1.0 / (T - theta)
                - 1.0 / (Tref - theta)) * ((theta - T) / theta) - T / pow(theta,2.)
                * log((Tref * (T - theta)) / (T * (Tref - theta)))) + (1.0
                / (T - theta)) * (a3 * (Pbar - Pref) + a4 * log((psi + Pbar)
                / (psi + Pref))) + GZterm;
    // GZterm = W * (-Z - 1.0e0);


    auto U = H - Pbar*V;

    auto A = U - T*S;

    // Convert the thermodynamic properties of the gas to the standard units
    V  *= 1e-01; // J/bar
    G  *= cal_to_J;
    H  *= cal_to_J;
    S  *= cal_to_J;
    U  *= cal_to_J;
    A  *= cal_to_J;
    Cp *= cal_to_J;

    ThermoPropertiesSubstance tps;
    tps.volume           = V;
    tps.gibbs_energy     = G;
    tps.enthalpy         = H;
    tps.entropy          = S;
    tps.internal_energy  = U;
    tps.helmholtz_energy = A;
    tps.heat_capacity_cp = Cp;
    tps.heat_capacity_cv = tps.heat_capacity_cp; // approximate Cp = Cv for an aqueous solution

    subst.checkCalcMethodBounds("HKF model", TC.val, Pbar.val, tps);
    if (wp.density >= 1400 || wp.density<=600)
    {
        setMessage(Reaktoro_::Status::calculated, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval", tps);
    }

///    aW.twp->gfun = g;  // solvent g-function - passed for b_gamma=f(T,P) 07.06.05

    return tps;
}

//-------------------------------------------------------------------------
// gShok2- Calc  g, dgdP, dgdT, d2gdT2 use equations in Shock et al. (1991)
// units:  T (C), D (g/cm3), beta, dgdP (bars-1)
// alpha, dgdT (K-1), daldT, d2gdT2 (K-2)
auto gShok2(Reaktoro_::Temperature T, Reaktoro_::Pressure P, const PropertiesSolvent &ps ) -> FunctionG
{
    Reaktoro_::ThermoScalar a, b, dgdD, /*dgdD2,*/ dadT, dadTT, dbdT, dbdTT, dDdT, dDdP,
                dDdTT, Db, dDbdT, dDbdTT, ft, dftdT, dftdTT, fp, dfpdP,
                f, dfdP, dfdT, d2fdT2, tempy;

//    auto TdegC = T -273.15; // K to C
//    auto Pbar = P * 1e-05; // Pa in bar

//    // Check if the point (T,P) is inside region III or the shaded region in Fig. 6 of
//    // Shock and others (1992), on page 809. In this case, we assume the g function to be zero.
//    if(ps.density > 1000.0 || ps.density < 350.0)
//        return funcG;

    double C[6]  = {-0.2037662e+01,  0.5747000e-02, -0.6557892e-05,
                    0.6107361e+01, -0.1074377e-01,  0.1268348e-04 };
    double cC[3] = { 0.3666666e+02, -0.1504956e-09,  0.5017997e-13 };

    const auto D        = ps.density / 1000; // in g/cm3
    const auto beta     = ps.Beta;
    const auto alpha    = ps.Alpha;
    const auto daldT    = ps.dAldT;

    FunctionG g;

    if (D.val >= 1.4)
    {
        Exception exception;
        exception.error << "Error in gShock2";
        exception.reason << "water density higher than 1.4 g*cm-3, Dw = "
            << D.val << "g*cm-3.";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    // Check if the point (T,P) is inside region III or the shaded region in Fig. 6 of
    // Shock and others (1992), on page 809. In this case, we assume the g function to be zero.
//    if(ps.density > 1000.0 || ps.density < 350.0)
//        return g;

    const auto pw = fabs(1.0e0 - D.val); // insert Sveta 19/02/2000

    a = C[0] + C[1]*T + C[2]*pow(T,2.);
    b = C[3] + C[4]*T + C[5]*pow(T,2.);
    g.g = a * pow(pw, b.val);

    dgdD = - a*b*pow(pw,(b.val - 1.0e0));
    // dgdD2 = a * b * (b - 1.0e0) * pow((1.0e0 - D),(b - 2.0e0));

    dadT = C[1] + 2.0*C[2]*T;
    dadTT = 2.0*C[2];
    dbdT = C[4] + 2.0*C[5]*T;
    dbdTT = 2.0*C[5];

    dDdT = - D * alpha;
    dDdP = D * beta;
    dDdTT = - D * (daldT - pow(alpha,2.));
        // Db = pow((1.0 - D),b);  Fixed by DAK 01.11.00
    Db = pow( pw , b.val );
    dDbdT = -b * pow(pw,(b.val - 1.0)) * dDdT + log(pw) * Db  * dbdT;


    dDbdTT = -(b * pow(pw,(b.val-1.0)) * dDdTT + pow(pw,(b.val - 1.0)) * dDdT * dbdT
                + b * dDdT * ( -(b - 1.0) * pow(pw,(b.val - 2.0)) * dDdT
                + log(pw) * pow(pw,(b.val - 1.0)) * dbdT))
                + log(pw) * pow(pw,b.val) * dbdTT
                - pow(pw,b.val) * dbdT * dDdT / (1.0 - D)
                + log(pw) * dbdT * dDbdT;

    g.gP = dgdD * dDdP; // from bar to Pa not necessary!!
    g.gT = a * dDbdT + Db * dadT;
    g.gTT = a * dDbdTT + 2.0e0 * dDbdT * dadT + Db * dadTT;

    if((T < 155.0) || (P > 1000.0) || (T > 355.0))
        return g;

    tempy = ((T - 155.0) / 300.0);
    ft = pow(tempy,4.8) + cC[0] * pow(tempy,16.);
    dftdT = 4.8e0 / 300.0 * pow(tempy,3.8) + 16.0 / 300.0 * cC[0] * pow(tempy,15.);
    dftdTT = 3.8 * 4.8 / (300.0 * 300.0) * pow(tempy,2.8)
             + 15.0 * 16.0 / (300.0 * 300.0) * cC[0] * pow(tempy,14.);
    fp = cC[1] * pow((1000.0 - P),3.) + cC[2] * pow((1000.0 - P),4.);
    dfpdP  = -3.0 * cC[1] * pow((1000.0 - P),2.) - 4.0 * cC[2] * pow((1000.0 - P),3.);

    f = ft * fp;
    dfdP = ft * dfpdP;
    dfdT = fp * dftdT;
    d2fdT2 = fp * dftdTT;

    g.g -= f;
    g.gP -= dfdP/1e05; // from bar to Pa
    g.gT -= dfdT;
    g.gTT -= d2fdT2;

    return g;
}

auto omeg92(FunctionG g, Substance species) -> ElectroPropertiesSubstance
{
    Reaktoro_::ThermoScalar reref, re, Z3, Z4;
    const auto chg = species.charge();

    auto hkf = species.thermoParameters().HKF_parameters;
    const auto wref = hkf[6] /*hkf.wref*/;

    ElectroPropertiesSubstance eps;

    if( chg == 0.0 )  // neutral aqueous species
    {
        eps.w   = wref;
        eps.wP  = 0.0;
        eps.wT  = 0.0;
        eps.wTT = 0.0;
    }
    else  // charged aqueous species
    {
        reref = pow(chg,2.) / (wref / eta + chg / (3.082 + gref));
        re = reref + fabs(chg) * g.g;
        eps.w = eta * (pow(chg,2.) / re - chg / (3.082 + g.g));
        Z3 = fabs(pow(chg,3.)) / pow(re,2.) - chg / pow((3.082 + g.g),2.);
        Z4 = fabs(pow(chg,4.)) / pow(re,3.) - chg / pow((3.082 + g.g),3.);
        eps.wP  = -eta * Z3 * g.gP;
        eps.wT  = -eta * Z3 * g.gT;
        eps.wTT = 2.0 * eta * Z4 * pow(g.gT,2.) - eta * Z3 * g.gTT;
    }
    return eps;
}

}
