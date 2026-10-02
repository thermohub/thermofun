#include "Substances/Solvent/WaterZhangDuan2005.h"
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "GlobalVariables.h"
#include "Common/Jets.hpp"

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

static auto valueOf(const real& x) -> double { return x.val(); }
static auto valueOf(const jets::Dual& x) -> double { return autodiff::val(x); }

/// The molar volume (the reduced volume) of water at T (K) and P (bar), solved with Newton's iterations. The number type is
/// `real` (the value and the first derivative along a direction) or `jets::Dual` (all the derivatives up to the third
/// order). After the convergence of the value `extraSteps` more steps are made: a Newton step doubles the order up to which
/// the derivatives of the number are converged.
template<class Number>
auto waterMolarVolume (Number TK, Number Pbar, Number V0, int extraSteps = 0) -> Number
{
    // Auxiliary constants for the Newton's iterations
    const int max_iters = 100;
    const double tolerance = 1.0e-08;

    const auto Tcr = waterCriticalTemperature;
//    const auto Pcr = waterCriticalPressure;
    const auto Vcr = waterCriticalVolume;
    const Number B = a[1] + a[2]/pow((TK/Tcr),2) + a[3]/pow((TK/Tcr),3);
    const Number C = a[4] + a[5]/pow((TK/Tcr),2) + a[6]/pow((TK/Tcr),3);
    const Number D = a[7] + a[8]/pow((TK/Tcr),2) + a[9]/pow((TK/Tcr),3);
    const Number E = a[10] + a[11]/pow((TK/Tcr),2) + a[12]/pow((TK/Tcr),3);
    const Number F = a[13]/(TK/Tcr);
    const Number G = a[14]*(TK/Tcr);
    Number Vr = V0;

    auto step = [&](const Number& Vr) -> Number
    {
        const Number f  = ( 1 + B/Vr + C/pow(Vr,2) + D/pow(Vr,4) + E/pow(Vr,5) +
                          (F/pow(Vr,2) + G/pow(Vr,4))*exp(-gamma/pow(Vr,2)))* RConstant
                        * TK/Pbar - Vr*Vcr;
        const Number df = ( -B/pow(Vr,2) - 2*C/pow(Vr,3) - 4*D/pow(Vr,5) - 5*E/pow(Vr,6) -
                          (2*exp(-gamma/pow(Vr,2))*(-F*gamma*pow(Vr,2) + F*pow(Vr,4) - gamma*G + 2*G*pow(Vr,2)))/pow(Vr,7)) * RConstant
                        * TK/Pbar - Vcr;
        return Vr - f/df;
    };

    for(int i = 1; i <= max_iters; ++i)
    {
        const Number Vr_plus_1 = step(Vr);

        if (std::abs(valueOf(Vr_plus_1) - valueOf(Vr)) < tolerance)
        {
            Number result = Vr_plus_1;
            for (int k = 0; k < extraSteps; ++k)
                result = step(result);
            return result;
        }

        Vr = Vr_plus_1;
    }

    ThermoFun::Exception exception;
    exception.error << "Unable to calculate the molar volume of water, Zhang and Duan (2005) EOS.";
    exception.reason << "The calculations did not converge at temperature "
        << valueOf(TK) << " K and pressure " << valueOf(Pbar) << "Pa.";
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

/// The density of water (kg/m3) as a function of T (K) and P (Pa) in the number type of the jets
static auto densityZhangDuan2005(jets::Dual T, jets::Dual P) -> jets::Dual
{
    jets::Dual Vr = waterMolarVolume<jets::Dual>(T, P/bar_to_Pa, jets::Dual(0.3), 3);
    jets::Dual V = Vr * waterCriticalVolume / 10;
    return H2OMolarMass/V*100;
}

auto propertiesWaterZhangDuan2005(const Reaktoro_::Pass& pass) -> PropertiesSolventAD
{
    PropertiesSolventAD ps;

    // the exact derivatives of the density up to the third order (no finite differences): the EOS is solved with
    // higher-order autodiff numbers in T and P (Pa); the third-order derivatives are the derivatives of the second-order ones
    const auto j = jets::jet(densityZhangDuan2005, pass.T.val(), pass.P.val());

    ps.density   = jets::along(pass, j.f,   j.fT,   j.fP);
    ps.densityT  = jets::along(pass, j.fT,  j.fTT,  j.fTP);
    ps.densityP  = jets::along(pass, j.fP,  j.fTP,  j.fPP);
    ps.densityTT = jets::along(pass, j.fTT, j.fTTT, j.fTTP);
    ps.densityTP = jets::along(pass, j.fTP, j.fTTP, j.fTPP);
    ps.densityPP = jets::along(pass, j.fPP, j.fTPP, j.fPPP);
    ps.Alpha     = -ps.densityT/ps.density;
    ps.Beta      = ps.densityP/ps.density; // 1/Pa
    ps.dAldT     = -ps.densityTT/ps.density + ps.Alpha*ps.Alpha;

    return ps;
}

}
