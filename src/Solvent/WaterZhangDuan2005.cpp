#include "Solvent/WaterZhangDuan2005.h"
#include "Common/Exception.h"

namespace TCorrPT {

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

auto waterMolarVolume (Reaktoro_::Temperature T, Reaktoro_::Pressure P, Reaktoro_::ThermoScalar V0) -> Reaktoro_::ThermoScalar
{
    // Auxiliary constants for the Newton's iterations
    const int max_iters = 100;
    const double tolerance = 1.0e-08;
    const auto Tc = waterCriticalTemperature;
    const auto Pc = waterCriticalPressure;
    const auto Vc = waterCriticalVolume;
    const auto B = a[1] + a[2]/pow((T/Tc),2) + a[3]/pow((T/Tc),3);
    const auto C = a[4] + a[5]/pow((T/Tc),2) + a[6]/pow((T/Tc),3);
    const auto D = a[7] + a[8]/pow((T/Tc),2) + a[9]/pow((T/Tc),3);
    const auto E = a[10] + a[11]/pow((T/Tc),2) + a[12]/pow((T/Tc),3);
    const auto F = a[13]/(T/Tc);
    const auto G = a[14]*(T/Tc);
    auto Vr = V0;

    for(int i = 1; i <= max_iters; ++i)
    {
        const auto f  = ( 1 + B/Vr + C/pow(Vr,2) + D/pow(Vr,4) + E/pow(Vr,5) +
                          (F/pow(Vr,2) + G/pow(Vr,4))*exp(-gamma/pow(Vr,2)))* RConstant
                        * T/P - Vr*Vc;
        const auto df = ( -B/pow(Vr,2) - 2*C/pow(Vr,3) - 4*D/pow(Vr,5) - 5*E/pow(Vr,6) -
                          (2*exp(-gamma/pow(Vr,2))*(-F*gamma*pow(Vr,2) + F*pow(Vr,4) - gamma*G + 2*G*pow(Vr,2)))/pow(Vr,7)) * RConstant
                        * T/P - Vc;
        const auto Vr_plus_1 = Vr - f/df;

        if (std::abs(Vr_plus_1.val - Vr.val) < tolerance)
        {
            const auto Z = 1 + B/Vr + C/pow(Vr,2) + D/pow(Vr,4) + E/pow(Vr,5) +
                           (F/pow(Vr,2) + G/pow(Vr,4))*exp(-gamma/pow(Vr,2));
            return Vr_plus_1;
        }

        Vr = Vr_plus_1;
    }


    TCorrPT::Exception exception;
    exception.error << "Unable to calculate the molar volume of water, Zhang and Duan (2005) EOS.";
    exception.reason << "The calculations did not converge at temperature "
        << T.val << " K and pressure " << P.val << "Pa.";
    exception.line = __LINE__;
    RaiseError(exception);

    return {};
}

auto waterFugacityCoeff (Reaktoro_::Temperature T, Reaktoro_::Pressure P, Reaktoro_::ThermoScalar Vr, Reaktoro_::ThermoScalar V) -> Reaktoro_::ThermoScalar
{
    Reaktoro_::ThermoScalar lnFugCoef, lnFugCoef2;

    const auto Tc = waterCriticalTemperature;
    const auto B = a[1]  + a[2]/pow((T/Tc),2)  + a[3]/pow((T/Tc),3);
    const auto C = a[4]  + a[5]/pow((T/Tc),2)  + a[6]/pow((T/Tc),3);
    const auto D = a[7]  + a[8]/pow((T/Tc),2)  + a[9]/pow((T/Tc),3);
    const auto E = a[10] + a[11]/pow((T/Tc),2) + a[12]/pow((T/Tc),3);
    const auto F = a[13]/(T/Tc);
    const auto G = a[14]*(T/Tc);
    const auto expf = exp(-gamma/pow(Vr,2));
    const auto Z = (P*V)/(RConstant*T);

//    const auto H = F/(2*gamma) + G/(2*pow(gamma,2)) - expf*F/(2*gamma) - expf*G/(2*pow(gamma,2)) - expf*G/pow(Vr,2);

    const auto H2 = G/(2*pow(gamma,2)) * (F*gamma/G + 1 - (F*gamma/G+1+gamma/pow(Vr,2)) * exp(-gamma/pow(Vr,2)));

    const auto H = (1/(2*gamma))*(F + G/pow(Vr,2) + G/gamma)*exp(-gamma/pow(Vr,2)) - F/(2*gamma) - G/(2*pow(gamma,2));

    lnFugCoef = Z - 1 - log(Z) + B/Vr + C/(2*pow(Vr,2)) + D/(4*pow(Vr,4)) + E/(5*pow(Vr,5)) - H;

    lnFugCoef2 = Z - 1 - log(Z) + B/Vr + C/(2*pow(Vr,2)) + D/(4*pow(Vr,4)) + E/(5*pow(Vr,5)) + H2;

    return lnFugCoef;
}


auto thermoPropertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;

    Reaktoro_::ThermoScalar Vr (18.0684); // cm^3/mol, initial value at 298.15 K, 1 bar
    Reaktoro_::ThermoScalar FugCoef;

    Vr = 0.3;

    Vr = waterMolarVolume(T, P, Vr);

    const auto V = Vr * waterCriticalVolume;
    FugCoef = exp(waterFugacityCoeff(T, P, Vr, V));

    tps.volume = V / 10;

    return tps;
}

auto waterDensityZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> Reaktoro_::ThermoScalar
{
    Reaktoro_::ThermoScalar Vr;

    Vr = 0.3;
    Vr = waterMolarVolume(T, P, Vr);

    const auto V = Vr * waterCriticalVolume /10;
    const auto D = H2OMolarMass/V*100;

    return D;
}

auto propertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> PropertiesSolvent
{
    PropertiesSolvent ps;

    Reaktoro_::ThermoScalar Vr (18.0684); // cm^3/mol, initial value at 298.15 K, 1 bar

    Vr = 0.3;

    Vr = waterMolarVolume(T, P, Vr);

    const auto V = Vr * waterCriticalVolume /10;
    const auto D = H2OMolarMass/V*100;

    ps.density  = D /* * 1000*/;
    ps.densityT = ps.density.ddt;
    ps.densityP = ps.density.ddp;
    ps.Alpha    = -ps.densityT/ps.density;
    ps.Beta     = ps.densityP/ps.density/1e+05; // from 1/bar to 1/Pa

    // finite difference
    Reaktoro_::ThermoScalar Vr_plus;
    Reaktoro_::Temperature T_plus (T.val+T.val*0.001);
    Vr_plus = waterMolarVolume(T_plus, P, Vr);
    const auto V_plus = Vr_plus * waterCriticalVolume/10;
    const auto D_plus = H2OMolarMass/V_plus *100;

    Reaktoro_::ThermoScalar Vr_minus;
    Reaktoro_::Temperature T_minus (T.val-T.val*0.001);
    Vr_minus = waterMolarVolume(T_minus, P, Vr);
    const auto V_minus = Vr_minus * waterCriticalVolume/10;
    const auto D_minus = H2OMolarMass/V_minus * 100;

    const auto VdT = (V_plus - V_minus) / ((T_plus-T_minus));
    const auto DdT = (D_plus - D_minus) / ((T_plus-T_minus));

    const auto Dd2T = (D_plus + D_minus - 2*D)/pow(((T_plus-T_minus)*0.5),2);
    const auto Vd2T = (V_plus + V_minus - 2*V)/pow(((T_plus-T_minus)*0.5),2);

    ps.densityTT = Dd2T;
    ps.dAldT     = (-ps.densityTT/ps.density).val + ps.Alpha*ps.Alpha;

    return ps;
}

}
