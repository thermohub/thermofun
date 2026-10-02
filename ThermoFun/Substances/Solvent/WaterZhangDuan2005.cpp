#include "Substances/Solvent/WaterZhangDuan2005.h"
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "GlobalVariables.h"

namespace ThermoFun {

const double waterCriticalTemperature = 647.25; // K
const double waterCriticalVolume      = 55.9480373; // cm^3/mol
const double waterCriticalPressure    = 22.064e+01; // bar
const double RConstant                = 83.14467; // cm^3*bar/K*mol

const double a[] =
{
    0.000000000000,
    3.49824207e-01,
   -2.91046273e+00,
    2.00914688e+00,
    1.12819964e-01,
    7.48997714e-01,
   -8.73207040e-01,
    1.70609505e-02,
   -1.46355822e-02,
    5.79768283e-02,
   -8.41246372e-04,
    4.95186474e-03,
   -9.16248538e-03,
   -1.00358152e-01,
   -1.82674744e-03
};

const double gamma = 1.05999998e-02;

/// The molar volume (the reduced volume) of water at T (K) and P (bar), solved with Newton's iterations
auto waterMolarVolume (real TK, real Pbar, real V0) -> real
{
    // Auxiliary constants for the Newton's iterations
    const int max_iters = 100;
    const double tolerance = 1.0e-08;

    const auto Tcr = waterCriticalTemperature;
//    const auto Pcr = waterCriticalPressure;
    const auto Vcr = waterCriticalVolume;
    const auto B = a[1] + a[2]/pow((TK/Tcr),2) + a[3]/pow((TK/Tcr),3);
    const auto C = a[4] + a[5]/pow((TK/Tcr),2) + a[6]/pow((TK/Tcr),3);
    const auto D = a[7] + a[8]/pow((TK/Tcr),2) + a[9]/pow((TK/Tcr),3);
    const auto E = a[10] + a[11]/pow((TK/Tcr),2) + a[12]/pow((TK/Tcr),3);
    const auto F = a[13]/(TK/Tcr);
    const auto G = a[14]*(TK/Tcr);
    auto Vr = V0;

    for(int i = 1; i <= max_iters; ++i)
    {
        const auto f  = ( 1 + B/Vr + C/pow(Vr,2) + D/pow(Vr,4) + E/pow(Vr,5) +
                          (F/pow(Vr,2) + G/pow(Vr,4))*exp(-gamma/pow(Vr,2)))* RConstant
                        * TK/Pbar - Vr*Vcr;
        const auto df = ( -B/pow(Vr,2) - 2*C/pow(Vr,3) - 4*D/pow(Vr,5) - 5*E/pow(Vr,6) -
                          (2*exp(-gamma/pow(Vr,2))*(-F*gamma*pow(Vr,2) + F*pow(Vr,4) - gamma*G + 2*G*pow(Vr,2)))/pow(Vr,7)) * RConstant
                        * TK/Pbar - Vcr;
        const auto Vr_plus_1 = Vr - f/df;

        if (std::abs(Vr_plus_1.val() - Vr.val()) < tolerance)
        {
            return Vr_plus_1;
        }

        Vr = Vr_plus_1;
    }

    ThermoFun::Exception exception;
    exception.error << "Unable to calculate the molar volume of water, Zhang and Duan (2005) EOS.";
    exception.reason << "The calculations did not converge at temperature "
        << TK.val() << " K and pressure " << Pbar.val() << "Pa.";
    exception.line = __LINE__;
    RaiseError(exception);

    return {};
}

// Duan et al 1992a; F = F92*beta, G = F92*gamma
auto waterFugacityCoeff (real T, real P, real Vr, real V) -> real
{
    real lnFugCoef/*, lnFugCoef2, lnFugCoef3*/;

    const auto Tc = waterCriticalTemperature;
    const auto B = a[1]  + a[2]/pow((T/Tc),2)  + a[3]/pow((T/Tc),3);
    const auto C = a[4]  + a[5]/pow((T/Tc),2)  + a[6]/pow((T/Tc),3);
    const auto D = a[7]  + a[8]/pow((T/Tc),2)  + a[9]/pow((T/Tc),3);
    const auto E = a[10] + a[11]/pow((T/Tc),2) + a[12]/pow((T/Tc),3);
    const auto F = a[13]/(T/Tc);
    const auto G = a[14]*(T/Tc);
    const auto expf = exp(-gamma/pow(Vr,2));
    const auto Z = (P*V)/(RConstant*T);

    const auto H = F/(2*gamma) + G/(2*pow(gamma,2)) - expf*F/(2*gamma) - expf*G/(2*pow(gamma,2)) - expf*G/(pow(Vr,2)*(2*gamma));

//    const auto H2 = G/(2*pow(gamma,2)) * (F*gamma/G + 1 - (F*gamma/G+1+gamma/pow(Vr,2)) * exp(-gamma/pow(Vr,2)));

//    const auto H = (1/(2*gamma))*(F + G/pow(Vr,2) + G/gamma)*exp(-gamma/pow(Vr,2)) - F/(2*gamma) - G/(2*pow(gamma,2));

//    lnFugCoef = Z - 1 - log(Z) + B/Vr + C/(2*pow(Vr,2)) + D/(4*pow(Vr,4)) + E/(5*pow(Vr,5)) - H;

//    lnFugCoef2 = Z - 1 - log(Z) + B/Vr + C/(2*pow(Vr,2)) + D/(4*pow(Vr,4)) + E/(5*pow(Vr,5)) + H2;

    lnFugCoef = Z - 1 - log(Z) + B/Vr + C/(2*pow(Vr,2)) + D/(4*pow(Vr,4)) + E/(5*pow(Vr,5)) + H;

    return lnFugCoef;
}


auto thermoPropertiesWaterZhangDuan2005(real T, real P) -> ThermoPropertiesSubstanceAD
{
    ThermoPropertiesSubstanceAD tps;

    real Vr (18.0684); // cm^3/mol, initial value at 298.15 K, 1 bar
    real FugCoef;

    Vr = 0.3;

    Vr = waterMolarVolume(T, P, Vr);

    const auto V = Vr * waterCriticalVolume;
    FugCoef = exp(waterFugacityCoeff(T, P, Vr, V));

    auto Gres = log(FugCoef)*R_CONSTANT * (T);

    tps.volume = V / 10;

    return tps;
}

auto waterDensityZhangDuan2005(real T, real P) -> real
{
    real Vr;

    Vr = 0.3;
    Vr = waterMolarVolume(T, P, Vr);

    const auto V = Vr * waterCriticalVolume /10;
    const auto D = H2OMolarMass/V*100;

    return D;
}

/// The partial derivatives of the reduced volume Vr(T, P) of the Zhang and Duan (2005) EOS, obtained analytically by the implicit
/// differentiation of f(Vr, T, P) = s - Vcr Vr = 0, with s = R T Z(Vr, T)/P (T in K, P in bar):
///   Z = 1 + B/x + C/x^2 + D/x^4 + E/x^5 + (F/x^2 + G/x^4) exp(-gamma/x^2),  x = Vr,  B..G power laws of T/Tc
///   V_T = -f_T/f_x, V_P = -f_P/f_x, V_TT = -(f_TT + 2 f_xT V_T + f_xx V_T^2)/f_x,
///   V_TP = -(f_TP + f_xT V_P + f_xP V_T + f_xx V_T V_P)/f_x, V_PP = -(f_PP + 2 f_xP V_P + f_xx V_P^2)/f_x
/// (s is proportional to 1/P: f_P = -s/P, f_PP = 2 s/P^2, f_xP = -s_x/P, f_TP = -s_T/P)
struct VolumeDerivatives
{
    real T, P, TT, TP, PP;
};

static auto volumeDerivatives(const real& x, const real& TK, const real& Pbar) -> VolumeDerivatives
{
    const real tau = TK/waterCriticalTemperature;
    const real t2 = pow(tau, -2), t3 = pow(tau, -3), t4 = pow(tau, -4), t5 = pow(tau, -5);
    const double Tc = waterCriticalTemperature;

    // the coefficients and their derivatives with respect to tau
    const real B = a[1] + a[2]*t2 + a[3]*t3,    B1 = -2*a[2]*t3 - 3*a[3]*t4,    B2 = 6*a[2]*t4 + 12*a[3]*t5;
    const real C = a[4] + a[5]*t2 + a[6]*t3,    C1 = -2*a[5]*t3 - 3*a[6]*t4,    C2 = 6*a[5]*t4 + 12*a[6]*t5;
    const real D = a[7] + a[8]*t2 + a[9]*t3,    D1 = -2*a[8]*t3 - 3*a[9]*t4,    D2 = 6*a[8]*t4 + 12*a[9]*t5;
    const real E = a[10] + a[11]*t2 + a[12]*t3, E1 = -2*a[11]*t3 - 3*a[12]*t4,  E2 = 6*a[11]*t4 + 12*a[12]*t5;
    const real F = a[13]/tau,                   F1 = -a[13]*t2,                 F2 = 2*a[13]*t3;
    const real G = a[14]*tau;
    const double G1 = a[14];

    // powers of x and the exponential
    const real x2 = x*x, x3 = x2*x, x4 = x2*x2, x5 = x4*x, x6 = x3*x3, x7 = x6*x;
    const real q = exp(-gamma/x2);
    const real qx = q*2*gamma/x3, qxx = q*(4*gamma*gamma/x6 - 6*gamma/x4);

    const real P0 = B/x + C/x2 + D/x4 + E/x5;
    const real P0x = -B/x2 - 2*C/x3 - 4*D/x5 - 5*E/x6;
    const real P0xx = 2*B/x3 + 6*C/x4 + 20*D/x6 + 30*E/x7;
    const real H = F/x2 + G/x4;
    const real Hx = -2*F/x3 - 4*G/x5;
    const real Hxx = 6*F/x4 + 20*G/x6;

    const real Z = 1 + P0 + H*q;
    const real Zx = P0x + Hx*q + H*qx;
    const real Zxx = P0xx + Hxx*q + 2*Hx*qx + H*qxx;

    // derivatives with respect to tau: Z_tau, Z_tau tau, Z_x tau
    const real H1 = F1/x2 + G1/x4, H1x = -2*F1/x3 - 4*G1/x5, H2 = F2/x2;
    const real Zt = B1/x + C1/x2 + D1/x4 + E1/x5 + H1*q;
    const real Ztt = B2/x + C2/x2 + D2/x4 + E2/x5 + H2*q;
    const real Zxt = -B1/x2 - 2*C1/x3 - 4*D1/x5 - 5*E1/x6 + H1x*q + H1*qx;

    // f = s - Vcr x with s = Rg T Z/P
    const real K = RConstant/Pbar;
    const real s  = K*TK*Z;
    const real sx = K*TK*Zx;
    const real sxx = K*TK*Zxx;
    const real sT = K*Z + K*TK*Zt/Tc;
    const real sTT = 2*K*Zt/Tc + K*TK*Ztt/(Tc*Tc);
    const real sxT = K*Zx + K*TK*Zxt/Tc;

    const real fx = sx - waterCriticalVolume;
    const real fxx = sxx;
    const real fT = sT, fTT = sTT, fxT = sxT;
    const real fP = -s/Pbar, fPP = 2*s/(Pbar*Pbar), fxP = -sx/Pbar, fTP = -sT/Pbar;

    VolumeDerivatives v;
    v.T = -fT/fx;
    v.P = -fP/fx;
    v.TT = -(fTT + 2*fxT*v.T + fxx*v.T*v.T)/fx;
    v.TP = -(fTP + fxT*v.P + fxP*v.T + fxx*v.T*v.P)/fx;
    v.PP = -(fPP + 2*fxP*v.P + fxx*v.P*v.P)/fx;
    return v;
}

auto propertiesWaterZhangDuan2005(const Reaktoro_::Pass& pass) -> PropertiesSolventAD
{
    PropertiesSolventAD ps;

    const double T0 = pass.T.val(), P0 = pass.P.val();   // K, Pa

    // the reduced volume and its first derivatives at the point (analytical), then the volume as a number of the pass
    const real Vconv = waterMolarVolume(real(T0), real(P0/bar_to_Pa), real(0.3));
    const auto v0 = volumeDerivatives(real(Vconv.val()), real(T0), real(P0/bar_to_Pa));
    const real x = Reaktoro_::along(pass, Vconv.val(), v0.T.val(), v0.P.val()/bar_to_Pa);

    // the second derivatives with the derivatives of the pass (the third-order derivatives of the volume)
    const real TK = pass.T, Pbar = pass.P/bar_to_Pa;
    const auto v = volumeDerivatives(x, TK, Pbar);

    // the density rho = kappa/x (kg/m3) and its derivatives with respect to T (K) and P (Pa); v.P, v.TP, v.PP are per bar
    const double kappa = H2OMolarMass*1000.0/waterCriticalVolume;
    const real xT = v.T, xP = v.P/bar_to_Pa;
    const real xTT = v.TT, xTP = v.TP/bar_to_Pa, xPP = v.PP/(bar_to_Pa*bar_to_Pa);

    ps.density   = kappa/x;
    ps.densityT  = -kappa*xT/(x*x);
    ps.densityP  = -kappa*xP/(x*x);
    ps.densityTT = kappa*(2*xT*xT/(x*x*x) - xTT/(x*x));
    ps.densityTP = kappa*(2*xT*xP/(x*x*x) - xTP/(x*x));
    ps.densityPP = kappa*(2*xP*xP/(x*x*x) - xPP/(x*x));
    ps.Alpha     = -ps.densityT/ps.density;
    ps.Beta      = ps.densityP/ps.density; // 1/Pa
    ps.dAldT     = -ps.densityTT/ps.density + ps.Alpha*ps.Alpha;

    return ps;
}

}
