#include "ThermoModelsSubstance.h"
#include "ThermoProperties.h"
#include "Substance.h"

#include "Substances/EmpiricalCpIntegration.h"
#include "Substances/StandardEntropyCpIntegration.h"
#include "Substances/Solute/SoluteHKFreaktoro.h"
#include "Substances/Solute/SoluteHKFgems.h"
#include "Substances/Solvent/WaterIdealGasWolley.h"
#include "Substances/Solute/SoluteADgems.h"
#include "Substances/Solute/SoluteHollandPowell98.h"
#include "Substances/Solute/SoluteAnderson91.h"
#include "Substances/Solids/SolidMurnaghanHP98.h"
#include "Substances/Solids/SolidBerman88.h"
#include "Substances/Solids/SolidBMGottschalk.h"
#include "Substances/Solids/SolidHPLandau.h"

#include "Substances/Gases/GasCORK.h"
#include "Substances/Gases/GasPRSV.h"
#include "Substances/Gases/GasCGF.h"
#include "Substances/Gases/GasSRK.h"
#include "Substances/Gases/GasPR78.h"
#include "Substances/Gases/GasSTP.h"

// ThermoFun includes
#include "Common/Exception.h"

namespace ThermoFun {


auto checkModelValidity(double T, double P, double Tmax, /*double Tmin,*/ double Pmax, /*double Pmin,*/ Substance species, std::string model) -> void
{
    // Check if given temperature is within the allowed range
    if(T < 0 /*Tmin*/ || T > Tmax)
    {
        Exception exception;
        exception.error << "Out of T bound in model "
              << model << " for substance " << species.symbol();
        exception.reason << "The provided temperature, " << T << " K,"  << "is either negative "
              "or greater than the maximum allowed, " << Tmax << " K.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if(P < 0/*Pmin*/ || P > Pmax)
    {
        Exception exception;
        exception.error << "Out of P bound in model "
              << model << " for substance " << species.symbol();
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is either negative "
              "or greater than the maximum allowed, " << Pmax << " Pa.";
        RaiseError(exception);
    }
}

/// Set the errors and statuses of the properties calculated with the HKF model: each one is not defined if a
/// property it is calculated from (a reference property of the substance or a solvent property) is not defined
static auto setStatusHKF(ThermoPropertiesSubstance& tps, const ThermoPropertiesSubstance& ref, const ElectroPropertiesSolvent& wes) -> void
{
    tps.volume.propagateFrom(wes.bornQ, wes.bornZ);
    tps.entropy.propagateFrom(ref.entropy, wes.bornY, wes.bornZ);
    tps.gibbs_energy.propagateFrom(ref.gibbs_energy, ref.entropy, wes.bornZ);
    tps.enthalpy.propagateFrom(ref.enthalpy, wes.bornZ, wes.bornY);
    tps.heat_capacity_cp.propagateFrom(wes.bornX, wes.bornY, wes.bornZ);
    tps.internal_energy.propagateFrom(tps.enthalpy, tps.volume);
    tps.helmholtz_energy.propagateFrom(tps.internal_energy, tps.entropy);
    tps.heat_capacity_cv.propagateFrom(tps.heat_capacity_cp);
}

/// Copy the errors and statuses of the properties
static auto copyStatus(ThermoPropertiesSubstance& out, const ThermoPropertiesSubstance& in) -> void
{
    auto copy = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
    copy(out.gibbs_energy, in.gibbs_energy);
    copy(out.helmholtz_energy, in.helmholtz_energy);
    copy(out.internal_energy, in.internal_energy);
    copy(out.enthalpy, in.enthalpy);
    copy(out.entropy, in.entropy);
    copy(out.volume, in.volume);
    copy(out.heat_capacity_cp, in.heat_capacity_cp);
    copy(out.heat_capacity_cv, in.heat_capacity_cv);
}

/// Set the statuses of the internal energy and the Helmholtz energy calculated from the enthalpy, entropy and volume
static auto setStatusUA(ThermoPropertiesSubstance& tps) -> void
{
    tps.internal_energy.propagateFrom(tps.enthalpy, tps.volume);
    tps.helmholtz_energy.propagateFrom(tps.internal_energy, tps.entropy);
}

struct ThermoModelsSubstance::Impl
{
    /// The substance instance
    Substance substance;

    Impl()
    {}

    Impl(const Substance& substance)
    : substance(substance)
    {}
};

ThermoModelsSubstance::ThermoModelsSubstance(const Substance& substance)
: pimpl(new Impl(substance))
{}

auto ThermoModelsSubstance::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    MethodGenEoS_Thrift::type method_genEOS = pimpl->substance.methodGenEOS();

    switch( method_genEOS )
    {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( pimpl->substance);
            return CpInt.thermoProperties(T, P);
//            break;
        }
    }

    // Exception
    Exception exception;
    exception.error << "The calculation method was not found.";
    exception.reason << "The calculation method defined for the substance "<< pimpl->substance.symbol() << " is not available.";
    exception.line = __LINE__;
    RaiseError(exception);
}

//=======================================================================================================
// Akinfiev & Diamond EOS for neutral species
// References: Akinfiev & Diamond (2003)
// Added: DM 13.06.2016
//=======================================================================================================

struct SoluteAkinfievDiamondEOS::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteAkinfievDiamondEOS::SoluteAkinfievDiamondEOS(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteAkinfievDiamondEOS::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp,
                                                const ThermoPropertiesSubstance& wtpr, const ThermoPropertiesSubstance& wigpr, const PropertiesSolvent& wpr) -> ThermoPropertiesSubstance
{
    // the properties at the reference temperature and pressure are constants
    const Reaktoro_::Pass reference(T, P, Reaktoro_::Wrt::None);
    const auto wtprAD  = lift(reference, wtpr);
    const auto wigprAD = lift(reference, wigpr);
    const auto wprAD   = lift(reference, wpr);

    auto state = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesAqSoluteAD(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, tps),
                                          lift(pass, wtp), lift(pass, wigp), lift(pass, wp), wtprAD, wigprAD, wprAD);
    });

    // the properties are not defined if the properties they are calculated from are not defined
    state.gibbs_energy.propagateFrom(tps.gibbs_energy, wtp.gibbs_energy, wtp.entropy, wtp.heat_capacity_cp, wtpr.gibbs_energy, wtpr.entropy, wtpr.heat_capacity_cp);
    state.entropy.propagateFrom(tps.entropy, wtp.gibbs_energy, wtp.entropy, wtp.heat_capacity_cp, wtpr.gibbs_energy, wtpr.entropy, wtpr.heat_capacity_cp);
    state.enthalpy.propagateFrom(tps.enthalpy, wtp.gibbs_energy, wtp.entropy, wtp.heat_capacity_cp, wtpr.gibbs_energy, wtpr.entropy, wtpr.heat_capacity_cp);
    state.heat_capacity_cp.propagateFrom(tps.heat_capacity_cp, wtp.gibbs_energy, wtp.entropy, wtp.heat_capacity_cp, wtpr.gibbs_energy, wtpr.entropy, wtpr.heat_capacity_cp);
    state.volume.asCalculated();
    state.internal_energy.propagateFrom(state.enthalpy, state.volume);
    state.helmholtz_energy.propagateFrom(state.internal_energy, state.entropy);
    state.heat_capacity_cv = tps.heat_capacity_cv; // not changed by this model

    return state;
}

//=======================================================================================================
// Calculates the ideal gas properties of pure H2O
// References: Woolley (1979)
// Added: DM 13.06.2016
//=======================================================================================================

struct WaterIdealGasWoolley::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterIdealGasWoolley::WaterIdealGasWoolley(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto WaterIdealGasWoolley::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return waterIdealGas(pass.T, pass.P);
    });

    tps.volume = Reaktoro_::ThermoProperty(); // the volume is not calculated

    return tps;
}


//=======================================================================================================
// HKF equation of state (as implmeneted in GEMS);
// References:
// Added: DM 07.06.2016
//=======================================================================================================

struct SoluteHKFgems::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteHKFgems::SoluteHKFgems(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteHKFgems::thermoProperties(double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance
{
//    checkModelValidity(T - C_to_K, P / bar_to_Pa, 1000, 5000, pimpl->substance, "HKFgems");

    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real t = pass.T - C_to_K;
        const real p = pass.P / bar_to_Pa;
        const auto wpAD  = lift(pass, wp);
        const auto wesAD = lift(pass, wes);

        FunctionG g = gShok2(t, p, wpAD);

        ElectroPropertiesSubstance aes = omeg92(g, pimpl->substance);

        return thermoPropertiesAqSoluteHKFgems(t, p, pimpl->substance, aes, wesAD, wpAD);
    });

    setStatusHKF(tps, pimpl->substance.thermoReferenceProperties(), wes);

    pimpl->substance.checkCalcMethodBounds("HKF model", T, P, tps);
    if (wp.density.val >= 1400 || wp.density.val <= 600)
        setMessage(Reaktoro_::Status::calculated, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval", tps);

    return tps;
}

//=======================================================================================================
// HKF equation of state (as implmeneted in reaktoro);
// References:
// Added: DM 07.06.2016
//=======================================================================================================

struct SoluteHKFreaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteHKFreaktoro::SoluteHKFreaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteHKFreaktoro::thermoProperties(double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance
{
//    checkModelValidity(T, P / bar_to_Pa, 1273.15, 5e08, pimpl->substance, "HKFreaktoro");

    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real t = pass.T;
        const real p = pass.P / bar_to_Pa;
        const auto wpAD  = lift(pass, wp);
        const auto wesAD = lift(pass, wes);

        FunctionG g = functionG(t, p, wpAD);

        ElectroPropertiesSubstance aes = speciesElectroStateHKF(g, pimpl->substance);

        return thermoPropertiesAqSoluteHKFreaktoro(t, p, pimpl->substance, aes, wesAD, wpAD);
    });

    setStatusHKF(tps, pimpl->substance.thermoReferenceProperties(), wes);

    pimpl->substance.checkCalcMethodBounds("HKF model", T, P, tps);
    if (wp.density.val >= 1400 || wp.density.val <= 600)
        setMessage(Reaktoro_::Status::calculated, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval", tps);

    return tps;
}


//=======================================================================================================
// Holland and Powell (1998) modified density model for aqueous species
// References: Holland and Powell (1998)
// Added: DM 16.07.2019
//=======================================================================================================

struct SoluteHollandPowell98::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteHollandPowell98::SoluteHollandPowell98(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteHollandPowell98::thermoProperties(double T, double P, const PropertiesSolvent& wpr, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
////    checkModelValidity(T, P / bar_to_Pa, 1000, 5000, pimpl->substance, "HKFgems");

    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesAqSoluteHP98(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, wpr), lift(pass, wp));
    });

    // the reference properties are used as constants, the isochoric heat capacity is not calculated
    tps.heat_capacity_cv = Reaktoro_::ThermoProperty();

    return tps;
}

//=======================================================================================================
// Anderson et al. (1991) density model for aqueous species
// References: Anderson et al. (1991)
// Added: DM 17.07.2019
//=======================================================================================================

struct SoluteAnderson91::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteAnderson91::SoluteAnderson91(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteAnderson91::thermoProperties(double T, double P, const PropertiesSolvent& wpr, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
////    checkModelValidity(T, P / bar_to_Pa, 1000, 5000, pimpl->substance, "HKFgems");

    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesAqSoluteAN91(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, wpr), lift(pass, wp));
    });

    // the reference properties are used as constants, the isochoric heat capacity is not calculated
    tps.heat_capacity_cv = Reaktoro_::ThermoProperty();

    return tps;
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinMurnaghanEOSHP98::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinMurnaghanEOSHP98::MinMurnaghanEOSHP98(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinMurnaghanEOSHP98::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    bool variableVolume = false;
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesMinMurnaghanEOSHP98(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, tps), &variableVolume);
    });

    const auto ref = pimpl->substance.thermoReferenceProperties();
    copyStatus(out, tps);
    out.gibbs_energy.propagateFrom(tps.gibbs_energy, ref.volume);
    out.enthalpy.propagateFrom(tps.enthalpy, ref.volume);
    if (variableVolume)
    {
        out.entropy.propagateFrom(tps.entropy, ref.volume);
        out.volume.propagateFrom(tps.volume, ref.volume);
        out.heat_capacity_cp.propagateFrom(tps.heat_capacity_cp, ref.volume);
    }
    else // the molar volume is assumed independent of T and P
    {
        out.volume.sta = ref.volume.sta;
        out.volume.err = ref.volume.err;
    }
    setStatusUA(out);

    pimpl->substance.checkCalcMethodBounds("Holland and Powell Murnaghan model", T, P, out);

    return out;
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinBerman88::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinBerman88::MinBerman88(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinBerman88::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    bool applied = false;
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesMinBerman88(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, tps), &applied);
    });

    const auto ref = pimpl->substance.thermoReferenceProperties();
    copyStatus(out, tps);
    out.volume.sta = ref.volume.sta; // the volume is the reference volume
    out.volume.err = ref.volume.err;
    if (applied)
    {
        out.gibbs_energy.propagateFrom(tps.gibbs_energy, ref.volume);
        out.enthalpy.propagateFrom(tps.enthalpy, ref.volume);
        out.entropy.propagateFrom(tps.entropy, ref.volume);
        out.volume.propagateFrom(ref.volume);
        setStatusUA(out);
    }

    pimpl->substance.checkCalcMethodBounds("Berman multisite model", T, P, out);

    return out;
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinBMGottschalk::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinBMGottschalk::MinBMGottschalk(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinBMGottschalk::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    bool applied = false;
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesMinBMGottschalk(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, tps), &applied);
    });

    const auto ref = pimpl->substance.thermoReferenceProperties();
    copyStatus(out, tps);
    if (applied)
    {
        out.volume.propagateFrom(tps.volume, ref.volume);
        out.entropy.propagateFrom(tps.entropy);
        out.gibbs_energy.propagateFrom(tps.gibbs_energy);
        out.enthalpy.propagateFrom(tps.enthalpy);
        setStatusUA(out);
    }

    pimpl->substance.checkCalcMethodBounds("BMGottschalk model", T, P, out);

    return out;
}

//=======================================================================================================
// Integration of empirical heat capacity equation Cp=f(T);
// References:
// Added: DM 26.04.2016
//=======================================================================================================

struct EmpiricalCpIntegration::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

EmpiricalCpIntegration::EmpiricalCpIntegration(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto EmpiricalCpIntegration::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    bool outsideBounds = false;
    auto calc = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesEmpCpIntegration(pass, pimpl->substance, &outsideBounds);
    });

    // the properties not calculated by the model are those of the substance
    ThermoPropertiesSubstance tps = pimpl->substance.thermoProperties();
    const ThermoPropertiesSubstance ref = pimpl->substance.thermoReferenceProperties();

    tps.heat_capacity_cp = calc.heat_capacity_cp;
    tps.gibbs_energy     = calc.gibbs_energy;
    tps.enthalpy         = calc.enthalpy;
    tps.entropy          = calc.entropy;
    tps.volume           = calc.volume;

    // the properties are integrated starting from the reference properties
    tps.gibbs_energy.propagateFrom(ref.gibbs_energy, ref.entropy);
    tps.enthalpy.propagateFrom(ref.enthalpy);
    tps.entropy.propagateFrom(ref.entropy);
    tps.volume.sta = {Reaktoro_::Status::assigned, ""};

    if (outsideBounds)
        setMessage(Reaktoro_::Status::calculated, "Empirical Cp integration: Outside temperature bounds", tps);

    return tps;
}

//=======================================================================================================
// Standard entropy and constant heat capacity integration
// References:
// Added: DM 08.10.2019
//=======================================================================================================

struct EntropyCpIntegration::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

EntropyCpIntegration::EntropyCpIntegration(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto EntropyCpIntegration::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    auto calc = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesEntropyCpIntegration(pass.T, pass.P / bar_to_Pa, pimpl->substance);
    });

    // the properties not calculated by the model are those of the substance
    ThermoPropertiesSubstance tps = pimpl->substance.thermoProperties();
    const ThermoPropertiesSubstance ref = pimpl->substance.thermoReferenceProperties();
    const bool hasS  = ref.entropy.sta.first != Reaktoro_::Status::notdefined;
    const bool hasCp = ref.heat_capacity_cp.sta.first != Reaktoro_::Status::notdefined;

    tps.heat_capacity_cp = calc.heat_capacity_cp;
    tps.gibbs_energy     = calc.gibbs_energy;
    tps.enthalpy         = calc.enthalpy;
    tps.entropy          = calc.entropy;

    // the properties are calculated from the reference properties
    tps.heat_capacity_cp.sta = ref.heat_capacity_cp.sta; tps.heat_capacity_cp.err = ref.heat_capacity_cp.err;
    if (hasS || hasCp) tps.gibbs_energy.propagateFrom(ref.gibbs_energy);
    else { tps.gibbs_energy.sta = ref.gibbs_energy.sta; tps.gibbs_energy.err = ref.gibbs_energy.err; }
    if (hasCp) { tps.entropy.propagateFrom(ref.entropy); tps.enthalpy.propagateFrom(ref.enthalpy); }
    else { tps.entropy.sta = ref.entropy.sta; tps.entropy.err = ref.entropy.err;
           tps.enthalpy.sta = ref.enthalpy.sta; tps.enthalpy.err = ref.enthalpy.err; }

    return tps;
}

//=======================================================================================================
// Holland-Powell phases with Landau transition
// References:
// Added: DM 01.07.2016
//=======================================================================================================

struct HPLandau::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

HPLandau::HPLandau(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto HPLandau::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    bool subcritical = false;
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesHPLandau(pass.T, pass.P / bar_to_Pa, pimpl->substance, lift(pass, tps), &subcritical);
    });

    copyStatus(out, tps);
    out.gibbs_energy.propagateFrom(tps.gibbs_energy);
    out.entropy.propagateFrom(tps.entropy);
    out.enthalpy.propagateFrom(tps.enthalpy);
    out.volume.asCalculated();
    if (subcritical) // the heat capacity is corrected at subcritical T only
        out.heat_capacity_cp.propagateFrom(tps.heat_capacity_cp);
    setStatusUA(out);

    pimpl->substance.checkCalcMethodBounds("Holland and Powell Landau model", T, P, out);

    return out;
}

/// Correct the Gibbs energy and enthalpy of a gas for the pressure (the pressure p and the reference pressure pr in bar)
static auto applyPressure(ThermoPropertiesSubstanceAD tps, const real& t, const real& p, double pr) -> ThermoPropertiesSubstanceAD
{
    tps.gibbs_energy    += tps.volume * (p - (pr / bar_to_Pa));
    tps.enthalpy        += tps.volume * (p - (pr / bar_to_Pa));
    tps.internal_energy  = tps.enthalpy - p*tps.volume;
    tps.helmholtz_energy = tps.internal_energy - (t)*tps.entropy;
    return tps;
}

/// The statuses of the properties after the pressure correction
static auto applyPressureStatus(ThermoPropertiesSubstance& tps) -> void
{
    tps.gibbs_energy.propagateFrom(tps.gibbs_energy, tps.volume);
    tps.enthalpy.propagateFrom(tps.enthalpy, tps.volume);
    setStatusUA(tps);
}

/// Calculate the properties of a gas or fluid with a model that increments the properties of the ideal gas,
/// optionally correcting them for pressure.
/// The model returns the autodiff properties given the temperature (K), the pressure (bar), the substance and
/// the properties of the ideal gas.
template<typename Model>
static auto gasProperties(double T, double P, const Substance& substance, const ThermoPropertiesSubstance& tps, bool apply_p,
                          const std::string& modelName, Model model) -> ThermoPropertiesSubstance
{
    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real t = pass.T;
        const real p = pass.P / bar_to_Pa;
        auto r = model(t, p, substance, lift(pass, tps));
        if (apply_p)
            r = applyPressure(r, t, p, substance.referenceP());
        return r;
    });

    // the Gibbs energy, enthalpy and entropy are incremented, the volume is set to a value
    copyStatus(out, tps);
    out.gibbs_energy.propagateFrom(tps.gibbs_energy);
    out.enthalpy.propagateFrom(tps.enthalpy);
    out.entropy.propagateFrom(tps.entropy);
    out.volume.sta = {Reaktoro_::Status::assigned, ""};
    out.volume.err = 0.0;

    if (apply_p)
        applyPressureStatus(out);

    // last, so that the status propagation does not erase the bounds message
    Substance subst = substance;
    subst.checkCalcMethodBounds(modelName, T, P, out);

    return out;
}

//=======================================================================================================
// CORK
// References:
// Added: DM 19.07.2016
//=======================================================================================================

struct GasCORK::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasCORK::GasCORK(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasCORK::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "CORK compensated-Redlich-Kwong fluid model", thermoPropertiesGasCORK);
}

//=======================================================================================================
// PRSV
// References:
// Added: DM --.07.2016
//=======================================================================================================

struct GasPRSV::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasPRSV::GasPRSV(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasPRSV::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "PRSV Peng-Robinson-Stryjek-Vera fluid model", thermoPropertiesGasPRSV);
}

//=======================================================================================================
// CGF
// References:
// Added: DM --.07.2016
//=======================================================================================================

struct GasCGF::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasCGF::GasCGF(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasCGF::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "Churakov and Gottschalk fluid model", thermoPropertiesGasCGF);
}

//=======================================================================================================
// SRK
// References:
// Added: DM --.07.2016
//=======================================================================================================

struct GasSRK::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasSRK::GasSRK(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasSRK::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "SRK Soave-Redlich-Kwong fluid model", thermoPropertiesGasSRK);
}

//=======================================================================================================
// PR78
// References:
// Added: DM --.07.2016
//=======================================================================================================

struct GasPR78::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasPR78::GasPR78(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasPR78::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "PR78 Peng-Robinson fluid model", thermoPropertiesGasPR78);
}

//=======================================================================================================
// STP
// References:
// Added: DM --.07.2016
//=======================================================================================================

struct GasSTP::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

GasSTP::GasSTP(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto GasSTP::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    return gasProperties(T, P, pimpl->substance, tps, apply_p, "STP Sterner-Pitzer fluid model", thermoPropertiesGasSTP);
}

//=======================================================================================================
// CON
// References:
// Added: DM 20.10.2016
//=======================================================================================================

struct ConMolVol::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

ConMolVol::ConMolVol(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto ConMolVol::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance rtps = pimpl->substance.thermoReferenceProperties();
    if (rtps.volume.sta.first != Reaktoro_::notdefined) // do pressure correction only if the molar volume is given
    {
        auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            const real t = pass.T;
            const real p = pass.P / bar_to_Pa;
            auto state = lift(pass, tps);
            const real V = Reaktoro_::constant(rtps.volume);
            const real dV = V * (p - (pimpl->substance.referenceP() / bar_to_Pa));
            state.volume           = V;
            state.gibbs_energy    += dV;
            state.enthalpy        += dV;
            state.internal_energy  = state.enthalpy - p*state.volume;
            state.helmholtz_energy = state.internal_energy - (t)*state.entropy;
            return state;
        });

        copyStatus(out, tps);
        out.volume.sta = rtps.volume.sta;
        out.volume.err = rtps.volume.err;
        out.gibbs_energy.propagateFrom(tps.gibbs_energy, rtps.volume);
        out.enthalpy.propagateFrom(tps.enthalpy, rtps.volume);
        setStatusUA(out);
        return out;
    }

    return tps;
}

//=======================================================================================================
// OFF
// References:
// Added: DM 20.10.2016
//=======================================================================================================

struct IdealGasLawVol::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

IdealGasLawVol::IdealGasLawVol(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto IdealGasLawVol::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, bool apply_p) -> ThermoPropertiesSubstance
{
    const bool idealGasVolume = ( pimpl->substance.substanceClass() == SubstanceClass::type::GASFLUID ) && P > 0.0;

    auto out = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real t = pass.T;
        const real p = pass.P / bar_to_Pa;
        auto state = lift(pass, tps);

        if (idealGasVolume)
        { // molar volume from the ideal gas law
            state.volume = (t) / p * R_CONSTANT;
        }

        if (apply_p)
            return applyPressure(state, t, p, pimpl->substance.referenceP());
        else
            return state;
    });

    copyStatus(out, tps);
    if (idealGasVolume)
        out.volume.asCalculated();
    if (apply_p)
        applyPressureStatus(out);

    return out;
}


} // namespace ThermoFun
