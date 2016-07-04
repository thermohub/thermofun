#include "ThermoModelsSubstance.h"
#include "Common/EmpiricalCpIntegration.h"
#include "Solute/SoluteHKFreaktoro.h"
#include "Solute/SoluteHKFgems.h"
#include "Solvent/WaterIdealGasWolley.h"
#include "Solute/SoluteADgems.h"
#include "Solids/SolidMurnaghanHP98.h"
#include "Solids/SolidBerman88.h"
#include "Solids/SolidBMGottschalk.h"
#include "Solids/SolidHPLandau.h"

// TCorrPT includes
#include "Common/Exception.h"

namespace TCorrPT {


auto checkModelValidity(double T, double P, double Tmax, /*double Tmin,*/ double Pmax, /*double Pmin,*/ Substance species, string model) -> void
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
            break;
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


auto SoluteAkinfievDiamondEOS::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return thermoPropertiesAqSoluteAD(t, p, pimpl->substance, tps, wtp, wigp, wp);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return waterIdealGas(t, p);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    checkModelValidity(t.val, p.val, 1273.15, 5e08, pimpl->substance, "HKFgems");

    FunctionG g = gShok2(t, p, wp);

    ElectroPropertiesSubstance aes = omeg92(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFgems(t, p, pimpl->substance, aes, wes);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    checkModelValidity(t.val, p.val, 1273.15, 5e08, pimpl->substance, "HKFreaktoro");

    FunctionG g = functionG(t, p, wp);

    ElectroPropertiesSubstance aes = speciesElectroStateHKF(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFreaktoro(t, p, pimpl->substance, aes, wes);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P /* * bar_to_Pa*/);

    return thermoPropertiesMinMurnaghanEOSHP98(t, p, pimpl->substance, tps);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P /* * bar_to_Pa*/);

    return thermoPropertiesMinBerman88(t, p, pimpl->substance, tps);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P);

    return thermoPropertiesMinBMGottschalk(t, p, pimpl->substance, tps);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P);

    return thermoPropertiesEmpCpIntegration(t, p, pimpl->substance);
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
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P);

    return thermoPropertiesHPLandau(t, p, pimpl->substance, tps);
}


} // namespace TCorrPT
