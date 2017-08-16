#include "ThermoModelsSubstance.h"
#include "ThermoProperties.h"
#include "Substance.h"

#include "Substances//EmpiricalCpIntegration.h"
#include "Substances/Solute/SoluteHKFreaktoro.h"
#include "Substances/Solute/SoluteHKFgems.h"
#include "Substances/Solvent/WaterIdealGasWolley.h"
#include "Substances/Solute/SoluteADgems.h"
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


auto SoluteAkinfievDiamondEOS::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp,
                                                const ThermoPropertiesSubstance& wtpr, const ThermoPropertiesSubstance& wigpr, const PropertiesSolvent& wpr) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    return thermoPropertiesAqSoluteAD(t, p, pimpl->substance, tps, wtp, wigp, wp, wtpr, wigpr, wpr);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

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
    auto t = Reaktoro_::Temperature(T/* + C_to_K*/);
    auto p = Reaktoro_::Pressure(P /* * bar_to_Pa*/);

//    checkModelValidity(t.val, p.val, 1000, 5000, pimpl->substance, "HKFgems");

    FunctionG g = gShok2(t, p, wp);

    ElectroPropertiesSubstance aes = omeg92(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFgems(t, p, pimpl->substance, aes, wes, wp);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

//    checkModelValidity(t.val, p.val, 1273.15, 5e08, pimpl->substance, "HKFreaktoro");

    FunctionG g = functionG(t, p, wp);

    ElectroPropertiesSubstance aes = speciesElectroStateHKF(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFreaktoro(t, p, pimpl->substance, aes, wes, wp);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P /* * bar_to_Pa*/);

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
    auto TK = Reaktoro_::Temperature(T + C_to_K);
    auto Pbar = Reaktoro_::Pressure(P /* * bar_to_Pa*/);

    return thermoPropertiesMinBerman88(TK, Pbar, pimpl->substance, tps);
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
    auto TK = Reaktoro_::Temperature(T + C_to_K);
    auto Pbar = Reaktoro_::Pressure(P);

    return thermoPropertiesMinBMGottschalk(TK, Pbar, pimpl->substance, tps);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

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
    auto TK = Reaktoro_::Temperature(T + C_to_K);
    auto Pbar = Reaktoro_::Pressure(P);

    return thermoPropertiesHPLandau(TK, Pbar, pimpl->substance, tps);
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
auto GasCORK::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasCORK(t, p, pimpl->substance, tps);
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
auto GasPRSV::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasPRSV(t, p, pimpl->substance, tps);
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
auto GasCGF::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasCGF(t, p, pimpl->substance, tps);
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
auto GasSRK::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasSRK(t, p, pimpl->substance, tps);
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
auto GasPR78::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasPR78(t, p, pimpl->substance, tps);
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
auto GasSTP::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T /*+ C_to_K*/);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesGasSTP(t, p, pimpl->substance, tps);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

    ThermoPropertiesSubstance rtps = pimpl->substance.thermoReferenceProperties();

    tps.volume           = rtps.volume;
    tps.gibbs_energy    += rtps.volume * (p - (pimpl->substance.referenceP() / bar_to_Pa));
    tps.enthalpy        += rtps.volume * (p - (pimpl->substance.referenceP() / bar_to_Pa));
    tps.internal_energy  = tps.enthalpy - p*tps.volume;
    tps.helmholtz_energy = tps.internal_energy - (t)*tps.entropy;

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
auto IdealGasLawVol::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

    if(( pimpl->substance.substanceClass() == SubstanceClass::type::GASFLUID ) && P > 0.0 )
    { // molar volume from the ideal gas law
        tps.volume = (t) / p * R_CONSTANT;
    }

    return tps;
}


} // namespace ThermoFun
