#include "Substances/Solvent/WaterIdealGasWolley.h"
#include "GlobalVariables.h"

namespace ThermoFun {

auto waterIdealGas (Reaktoro_::Temperature t, Reaktoro_::Pressure p) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tp;
    int i=1;
    Reaktoro_::ThermoScalar  tt, tl, emult, Gig, Hig, Cpig, Cvig, Sig, Aig, Uig;
    double c[18]={ .19730271018e2,    .209662681977e2,  -.483429455355e0,
                   .605743189245e1,   .2256023885e2,    -.987532442e1,
                   -.43135538513e1,    .458155781e0,     -.47754901883e-1,
                   .41238460633e-2,  -.27929052852e-3,   .14481695261e-4,
                   -.56473658748e-6,   .16200446e-7,     -.3303822796e-9,
                   .451916067368e-11,-.370734122708e-13, .137546068238e-15 };

    Reaktoro_::ThermoScalar dH0k;  // enthapy of ideal gas water at 0 K (kJ)
    dH0k += (-182161.88);

//    tt  = t / 1.0e2;
//    tl  = log(tt);
    Gig  = - (c[0] / (t / 1.0e2) + c[1]) * log(t / 1.0e2);
    Hig  = (c[1] + c[0] * (1.0e0 - log(t / 1.0e2)) / (t / 1.0e2));
    Cpig = c[1] - c[0] / (t / 1.0e2);

    while ( ++i <= 17 )
    {
        emult = pow((t / 1.0e2),((double)i - 5.));
        Gig  -= c[i] * emult;
        Hig  += c[i] * (i-5) * emult;
        Cpig += c[i] * (i-5) * (i-4) * emult;
    }

    Aig  = Gig - 1.0e0;
    Uig  = Hig - 1.0e0;
    Cvig = Cpig - 1.0e0;
    Sig  = Uig - Aig;

    tp.enthalpy = Hig * R_CONSTANT * t + dH0k;
    tp.entropy  = Sig * R_CONSTANT;
    tp.gibbs_energy = Gig * R_CONSTANT * t + dH0k;
    tp.heat_capacity_cp = Cpig * R_CONSTANT;
    tp.heat_capacity_cv = Cvig * R_CONSTANT;
    tp.helmholtz_energy = Aig * R_CONSTANT * t + dH0k;
    tp.internal_energy = Uig * R_CONSTANT * t + dH0k;

    return tp;
}


}
