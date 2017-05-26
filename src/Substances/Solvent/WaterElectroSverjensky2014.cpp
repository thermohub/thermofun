#include "WaterElectroSverjensky2014.h"
#include "Thermo.h"
#include "Database.h"


namespace ThermoFun {

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

auto epsilonS(Reaktoro_::Temperature T, Reaktoro_::ThermoScalar RHO) -> Reaktoro_::ThermoScalar
{
   return exp(b[1]*T + b[2]*pow(T,0.5) + b[3])*pow(RHO,(a[1]*T + a[2]*pow(T,0.5) + a[3]));
}

auto electroPropertiesWaterSverjensky2014(/*PropertiesSolvent ps,*/ Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent wep;

    Database db; db.addSubstance(substance);
    Thermo   th(db);

    auto psol = th.propertiesSolvent(T.val, P.val, substance.symbol());

    const auto RHO = psol.density /1000;
    const auto eps = epsilonS(T, RHO);
    const auto epsilon2 = eps * eps;

    // numerical approximation of epsilonT and epsilonTT
    Reaktoro_::Temperature T_plus (T.val+T.val*0.001);
    auto RHO_plus = th.propertiesSolvent(T_plus.val, P.val, substance.symbol()).density / 1000;
    auto eps_plus = epsilonS(T_plus, RHO_plus);

    Reaktoro_::Temperature T_minus (T.val-T.val*0.001);
    auto RHO_minus  = th.propertiesSolvent(T_minus.val, P.val, substance.symbol()).density / 1000;
    auto eps_minus = epsilonS(T_minus, RHO_minus);

    const auto epsilonT  = (eps_plus - eps_minus) / ((T_plus-T_minus));
    const auto epsilonTT = (eps_plus + eps_minus - 2*eps)/pow(((T_plus-T_minus)*0.5),2);

    // numerical approximation of epsilonP and epsilonPP
    Reaktoro_::Pressure P_plus (P.val+P.val*0.001);
    RHO_plus = th.propertiesSolvent(T.val, P_plus.val, substance.symbol()).density / 1000;
    eps_plus = epsilonS(T, RHO_plus);

    Reaktoro_::Pressure P_minus (P.val-P.val*0.001);
    RHO_minus = th.propertiesSolvent(T.val, P_minus.val, substance.symbol()).density / 1000;
    eps_minus = epsilonS(T, RHO_minus);

    const auto epsilonP  = (eps_plus - eps_minus) / ((P_plus-P_minus));
    const auto epsilonPP = (eps_plus + eps_minus - 2*eps)/pow(((P_plus-P_minus)*0.5),2);

    wep.epsilon   = eps;
    wep.epsilonP  = epsilonP;
    wep.epsilonPP = epsilonPP;
    wep.epsilonT  = epsilonT;
    wep.epsilonTT = epsilonTT;
    wep.bornZ = -1.0/wep.epsilon;
    wep.bornY = wep.epsilonT/epsilon2;
    wep.bornQ = wep.epsilonP/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
    wep.bornX = wep.epsilonTT/epsilon2 - 2.0*wep.bornY*wep.bornY*wep.epsilon;

    return wep;
}

}
