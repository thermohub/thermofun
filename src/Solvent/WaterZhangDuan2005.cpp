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
    auto Vn = V0;

    for(int i = 1; i <= max_iters; ++i)
    {
        const auto f  = ( 1 + B/Vn + C/pow(Vn,2) + D/pow(Vn,4) + E/pow(Vn,5) +
                          (F/pow(Vn,2) + G/pow(Vn,4))*exp(-gamma/pow(Vn,2)))* RConstant
                        * T/P - Vn*Vc;
        const auto df = ( -B/pow(Vn,2) - 2*C/pow(Vn,3) - 4*D/pow(Vn,5) - 5*E/pow(Vn,6) -
                          (2*exp(-gamma/pow(Vn,2))*(-F*gamma*pow(Vn,2) + F*pow(Vn,4) - gamma*G + 2*G*pow(Vn,2)))/pow(Vn,7)) * RConstant
                        * T/P - Vc;
        const auto Vn_plus_1 = Vn - f/df;

        if (std::abs(Vn_plus_1.val - Vn.val) < tolerance)
        {
            return Vn_plus_1;
        }

        Vn = Vn_plus_1;
    }

    TCorrPT::Exception exception;
    exception.error << "Unable to calculate the molar volume of water, Zhang and Duan (2005) EOS.";
    exception.reason << "The calculations did not converge at temperature "
        << T.val << " K and pressure " << P.val << "Pa.";
    exception.line = __LINE__;
    RaiseError(exception);

    return {};
}


auto thermoPropertiesWaterZhangDuan2005(Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;

    Reaktoro_::ThermoScalar V0 (18.0684); // cm^3/mol, initial value at 298.15 K, 1 bar

    V0 = 0.3;

    V0 = waterMolarVolume(T, P, V0);

    const auto V = V0 * waterCriticalVolume;


    return tps;
}

}
