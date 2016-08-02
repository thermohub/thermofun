#include "WaterElectroSverjensky2014.h"
#include "WaterZhangDuan2005.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

const double a[] =
{
     0.0000000000,
    -1.576377e-03,
     6.810288e-02,
     7.548755e-01
};

const double b[] =
{
     0.0000000000,
    -8.016651e-05,
    -6.871618e-02,
     4.747973
};

auto epsilon(Reaktoro_::Temperature T, Reaktoro_::ThermoScalar RHO) -> Reaktoro_::ThermoScalar
{
   return exp(b[1]*T + b[2]*pow(T,0.5) + b[3])*pow(RHO,(a[1]*T + a[2]*pow(T,0.5) + a[3]));
}

auto electroPropertiesWaterSverjensky2014(PropertiesSolvent ps, Reaktoro_::Temperature T, Reaktoro_::Pressure P) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent eps;

    const auto RHO = ps.density /1000;

    eps.epsilon = epsilon(T, RHO);

    const auto epsilon2 = eps.epsilon * eps.epsilon;

    eps.bornZ = -1.0/eps.epsilon;
    eps.bornY = eps.epsilon.ddt/epsilon2;
    eps.bornQ = eps.epsilon.ddp/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;

    // numerical approximation of epsilonT
    Reaktoro_::Temperature T_plus (T.val+T.val*0.001);
    Reaktoro_::Temperature T_plusK (T_plus.val + 273.15);
    const auto RHO_plus = waterDensityZhangDuan2005(T_plusK, P) / 1000;
    const auto eps_plus = epsilon(T_plus, RHO_plus);

    Reaktoro_::Temperature T_minus (T.val-T.val*0.001);
    Reaktoro_::Temperature T_minusK (T_minus.val + 273.15);
    const auto RHO_minus = waterDensityZhangDuan2005(T_minusK, P) / 1000;
    const auto eps_minus = epsilon(T_minus, RHO_minus);

    const auto epsilonT = (eps_plus - eps_minus) / ((T_plus-T_minus));

    eps.bornX = epsilonT.ddt/epsilon2 - 2.0*eps.bornY*eps.bornY*eps.epsilon;

    eps.epsilonT = epsilonT;
    eps.epsilonP = eps.epsilon.ddp * 1e-05; // from bar to Pa
    eps.epsilonTT= epsilonT.ddt;

    return eps;
}

}
