#include "Substances/Solute/SoluteADgems.h"
#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesAqSoluteAD(Reaktoro_::Temperature Tk, Reaktoro_::Pressure Pbar, Substance subst,
                                ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp,
                                const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp,
                                const ThermoPropertiesSubstance& wtpr, const ThermoPropertiesSubstance& wigpr,
                                const PropertiesSolvent& wpr) -> ThermoPropertiesSubstance
{
    Reaktoro_::ThermoScalar Geos, Veos, Seos, CPeos, Heos;
    Reaktoro_::ThermoScalar Gids, /*Vids,*/ Sids, CPids, Hids;
    Reaktoro_::ThermoScalar Geos298, Veos298, Seos298, CPeos298, Heos298;

    auto Tr = 298.15;
    auto Pr = 1.0;

    ThermoPropertiesSubstance state = tps;

    const auto ADparam = subst.thermoParameters().Cp_nonElectrolyte_coeff;

    if (ADparam.size() == 0)
    {
        Exception exception;
        exception.error << "Error in Akinfiev Diamond EOS";
        exception.reason << "There are no model parameters given for "<<subst.symbol() << ".";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    auto Gig    = wigpr.gibbs_energy;
    auto Sig    = wigpr.entropy;
    auto CPig   = wigpr.heat_capacity_cp;

    auto Gw    = wtpr.gibbs_energy;
    auto Sw    = wtpr.entropy;
    auto CPw   = wtpr.heat_capacity_cp;
    auto rho   = wpr.density/1000;
    auto alp   = wpr.Alpha;
    auto bet   = wpr.Beta*1e05;
    auto dalpT = wpr.dAldT;

    Akinfiev_EOS_increments(Tr, Pr, Gig, Sig, CPig, Gw, Sw, CPw, rho, alp, bet, dalpT, ADparam,
                       Geos298, Veos298, Seos298, CPeos298, Heos298 );

    // Getting back ideal gas properties corrected for T of interest
    // by substracting properties of hydration at Tr, Pr
    Gids    = state.gibbs_energy    -= Geos298;
 // Vids    = aW.twp->V             -= Veos298;
    Sids    = state.entropy         -= Seos298;
 // CPids   = aW.twp->Cp            -= CPeos298;
    CPids   = state.heat_capacity_cp;
    Hids    = state.enthalpy        -= Heos298;

    // Properties of water at T,P of interest, modified 06.02.2008 (TW)
    Gig     = wigp.gibbs_energy;   // converting normalized ideal gas values
    Sig     = wigp.entropy;
    CPig    = wigp.heat_capacity_cp;
    Gw      = wtp.gibbs_energy;
    Sw      = wtp.entropy;
    CPw     = wtp.heat_capacity_cp;
    rho     = wp.density / 1000;
    alp     = wp.Alpha;
    bet     = wp.Beta*1e05;
    dalpT   = wp.dAldT;

    Akinfiev_EOS_increments(Tk, Pbar, Gig, Sig, CPig, Gw, Sw, CPw, rho, alp, bet, dalpT, ADparam,
                       Geos, Veos, Seos, CPeos, Heos );

    // Getting dissolved gas properties corrected for T,P of interest
    // by adding properties of hydration at T,P
    Gids.ddp = 0.0; Sids.ddp = 0.0; CPids.ddp = 0.0;
    state.gibbs_energy     = Gids + Geos + Seos298.val*(Tk.val-Tr);  // S(T-Tr) corrected for dSh at Tr,Pr
    // aW.twp->V = Vids + Veos;
    state.volume           = Veos;
    state.entropy          = Sids + Seos;
    state.heat_capacity_cp = CPids + CPeos;
    state.enthalpy         = Hids + Heos;
    state.internal_energy  = state.enthalpy - Pbar*state.volume;
    state.helmholtz_energy = state.internal_energy - Tk*state.entropy;

    subst.checkCalcMethodBounds("Akinfiev and Diamond model", Tk.val, Pbar.val*1e5, tps);

    return state;
}

void Akinfiev_EOS_increments(Reaktoro_::Temperature Tk, Reaktoro_::Pressure /*P*/, Reaktoro_::ThermoScalar Gig, Reaktoro_::ThermoScalar Sig, Reaktoro_::ThermoScalar CPig,
        Reaktoro_::ThermoScalar Gw, Reaktoro_::ThermoScalar Sw, Reaktoro_::ThermoScalar CPw, Reaktoro_::ThermoScalar rho, Reaktoro_::ThermoScalar alp, Reaktoro_::ThermoScalar bet, Reaktoro_::ThermoScalar dalpT, vd ADparam,
        Reaktoro_::ThermoScalar &Geos, Reaktoro_::ThermoScalar &Veos, Reaktoro_::ThermoScalar &Seos, Reaktoro_::ThermoScalar &CPeos, Reaktoro_::ThermoScalar &Heos )
{

    Reaktoro_::ThermoScalar derP, derT, der2T;
    Reaktoro_::ThermoScalar deltaB, lnKH, Nw, xi, aa, bb, RT;
    Reaktoro_::ThermoScalar fug, vol, drhoT, drhoP, d2rhoT, lnfug, Gres, Sres, CPres;
    const Reaktoro_::ThermoScalar RR (83.1451); const Reaktoro_::ThermoScalar R_CONST (R_CONSTANT);
    const Reaktoro_::ThermoScalar MW (18.01528);

    RT = Tk*R_CONST;
    // EOS coefficients
    xi = ADparam[0];
    aa = ADparam[1];
    bb = ADparam[2];

    Gres = Gw-Gig;
    Sres = Sw-Sig;
    CPres = CPw - CPig;
    lnfug = Gres/RT;
    fug = exp(lnfug);
    vol = MW/rho;
    drhoT = - alp*rho;
    drhoP = bet*rho;
    d2rhoT = rho*(pow(alp,2.)-dalpT);

    // calculation of infinite dilution properties
    Nw = 1000./MW;
    deltaB = 0.5*(aa + bb*pow((1000./Tk),0.5));
    lnKH = (1.-xi)*log(fug) + xi*log(RR*Tk/MW*rho) + 2.*rho*deltaB;

    Geos = - R_CONST*Tk*log(Nw) + (1.-xi)*R_CONST*Tk*log(fug) + R_CONST*Tk*xi*log(RR*Tk/MW*rho)
                + R_CONST*Tk*rho*(aa+bb*pow((1000./Tk),0.5));
    derP = aa*Tk*drhoP + bb*pow(10.,1.5)*pow(Tk,0.5)*drhoP;
    derT = aa*(rho+Tk*drhoT) + bb*(0.5*pow(10.,1.5)*pow(Tk,-0.5)*rho + pow(10.,1.5)*pow(Tk,0.5)*drhoT);
    der2T = aa*(2.*drhoT+Tk*d2rhoT) + bb*((-0.25)*pow(10.,1.5)*pow(Tk,-1.5)*rho
                + pow(10.,1.5)*pow(Tk,-0.5)*drhoT + pow(10.,1.5)*pow(Tk,0.5)*d2rhoT);

    Veos = (vol*(1.-xi) + xi*RR*Tk*(1./rho)*drhoP + RR*derP)/10.;
    Seos = (1.-xi)*(Sres) + R_CONST*log(Nw) - R_CONST*(xi + xi*log(RR*Tk/MW) + xi*log(rho)
                + xi*Tk*(1./rho)*drhoT) - R_CONST*derT;
    CPeos = (1.-xi)*(CPres) - R_CONST*(xi + 2.*xi*Tk*(1./rho)*drhoT
                - xi*pow(Tk,2.)/pow(rho,2.)*pow(drhoT,2.) + xi*pow(Tk,2.)/rho*d2rhoT) - R_CONST*Tk*der2T;
    Heos = Geos + Tk*Seos;
}

}
