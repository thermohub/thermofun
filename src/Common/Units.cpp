#include "Units.h"
#include "../ThermoProperties.h"


namespace ThermoFun {

auto changeUnitPressure ( ThermoPropertiesSubstance  tpSubst, double &P, double multiplier) -> ThermoPropertiesSubstance
{
    auto p = tpSubst;
    P /=multiplier;

    p.enthalpy.ddp         *= multiplier;
    p.entropy.ddp          *= multiplier;
    p.gibbs_energy.ddp     *= multiplier;
    p.heat_capacity_cp.ddp *= multiplier;
    p.heat_capacity_cv.ddp *= multiplier;
    p.helmholtz_energy.ddp *= multiplier;
    p.internal_energy.ddp  *= multiplier;
    p.volume.ddp           *= multiplier;

    return p;
}
auto changeUnitPressure ( ThermoPropertiesReaction   tpReact, double &P, double multiplier) -> ThermoPropertiesReaction
{
    auto p = tpReact;
    P /=multiplier;

    p.ln_equilibrium_constant.ddp   *= multiplier;
    p.log_equilibrium_constant.ddp  *= multiplier;
    p.reaction_enthalpy.ddp         *= multiplier;
    p.reaction_entropy.ddp          *= multiplier;
    p.reaction_gibbs_energy.ddp     *= multiplier;
    p.reaction_heat_capacity_cp.ddp *= multiplier;
    p.reaction_heat_capacity_cv.ddp *= multiplier;
    p.reaction_helmholtz_energy.ddp *= multiplier;
    p.reaction_internal_energy.ddp  *= multiplier;
    p.reaction_volume.ddp           *= multiplier;

    return p;
}
auto changeUnitPressure ( PropertiesSolvent          pSolv,   double &P, double multiplier) -> PropertiesSolvent
{
    auto p = pSolv;
    P /=multiplier;

    p.cpIdealGas.ddp      *= multiplier;
    p.cpIdealGas.ddp      *= multiplier;
    p.entropyIdealGas.ddp *= multiplier;
    p.gibbsIdealGas.ddp   *= multiplier;
    p.density.ddp         *= multiplier;
    p.densityP            *= multiplier;
    p.densityPP           *= multiplier;
    p.densityT.ddp        *= multiplier;
    p.densityTP           *= multiplier;
    p.densityTT.ddp       *= multiplier;
    p.pressure            /= multiplier;
    p.pressureD.ddp       *= multiplier;
    p.pressureDD.ddp      *= multiplier;
    p.pressureT.ddp       *= multiplier;
    p.pressureTD.ddp      *= multiplier;
    p.pressureTT.ddp      *= multiplier;


    p.Alpha  = -p.densityT/p.density;
    p.Beta   =  p.densityP/p.density;
    p.dAldT  = -p.densityTT/p.density + p.Alpha*p.Alpha;
//    const auto betaT  =  wt.densityTP/wt.density + alpha*beta;
//    const auto betaP  =  wt.densityPP/wt.density - beta*beta;
    p.Albe = p.Alpha / p.Beta;

    return p;
}
auto changeUnitPressure ( ElectroPropertiesSolvent   epSolv,  double &P, double multiplier) -> ElectroPropertiesSolvent
{
    auto p = epSolv;
    P /=multiplier;

    p.epsilon.ddp   *= multiplier;
    p.epsilonP      *= multiplier;
    p.epsilonPP     *= multiplier;
    p.epsilonT.ddp  *= multiplier;
    p.epsilonTP     *= multiplier;
    p.epsilonTT.ddp *= multiplier;

    const auto epsilon2 = p.epsilon * p.epsilon;

    p.bornZ = -1.0/p.epsilon;
    p.bornY = p.epsilonT/epsilon2;
    p.bornQ = p.epsilonP/epsilon2;
    p.bornU = p.epsilonTP/epsilon2 - 2.0*p.bornY*p.bornQ*p.epsilon;
    p.bornN = p.epsilonPP/epsilon2 - 2.0*p.bornQ*p.bornQ*p.epsilon;
    p.bornX = p.epsilonTT/epsilon2 - 2.0*p.bornY*p.bornY*p.epsilon;

    return p;
}
auto changeUnitPressure ( ElectroPropertiesSubstance epSubst, double &P, double multiplier) -> ElectroPropertiesSubstance
{
    auto p = epSubst;
    P /=multiplier;

    p.re.ddp      *= multiplier;
    p.reref.ddp   *= multiplier;
    p.w.ddp       *= multiplier;
    p.wP          *= multiplier;
    p.wPP         *= multiplier;
    p.wT.ddp      *= multiplier;
    p.wTP         *= multiplier;
    p.wTT.ddp     *= multiplier;

    return p;
}
auto changeUnitPressure ( FunctionG                  fG,      double &P, double multiplier) -> FunctionG
{
    auto p = fG;
    P /=multiplier;

    p.g.ddp   *= multiplier;
    p.gP      *= multiplier;
    p.gPP     *= multiplier;
    p.gT.ddp  *= multiplier;
    p.gTP     *= multiplier;
    p.gTT.ddp *= multiplier;

    return p;
}



}
