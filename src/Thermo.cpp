// TCorrPT includes
#include "Common/Exception.h"
#include "Thermo.h"

namespace TCorrPT {

struct Thermo::Impl
{
    /// The database instance
    Database database;

    Solvent solvent;

    Impl()
    {}

    Impl(const Database& database)
    : database(database)
    {}
};

Thermo::Thermo(const Database& database)
: pimpl(new Impl(database))
{}

auto Thermo::thermoPropertiesSubstance(double T, double &P, std::string substance) -> ThermoPropertiesSubstance
{
    ThermoPreferences         pref = getThermoPreferences(substance);
    ThermoPropertiesSubstance tps;

    calculateSolvent(pref.solventSymbol, T, P, pimpl->solvent);

    if (pref.isHydrogen)
    {
        return tps;
    }

    if (!pref.isH2OSolvent && !pref.isH2Ovapor)
    {
        // metohd EOS
        switch( pref.method_genEOS )
        {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( pref.workSubstance );
                                   tps = CpInt.thermoProperties(T, P);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKF:
        {
            if (pref.solventSymbol.empty() || pref.solventSymbol == "*")
            {
                errorSolventNotDefined("solvent", pref.workSubstance.symbol(), __LINE__);
            }
            SoluteHKFgems aqHKF( pref.workSubstance );
                          tps = aqHKF.thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKFR:
        {
            if (pref.solventSymbol.empty() || pref.solventSymbol == "*")
            {
                errorSolventNotDefined("solvent", pref.workSubstance.symbol(), __LINE__);
            }
            SoluteHKFreaktoro aqHKF( pref.workSubstance );
                              tps = aqHKF.thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
            break;
        }
            // Exception
            errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
        }

        // method T
        switch ( pref.method_T )
        {
        case MethodCorrT_Thrift::type::CTM_CHP:
        {
            HPLandau hpLandau ( pref.workSubstance );
                     tps =    hpLandau.thermoProperties(T, P, tps);
            break;
        }
            // Exception
            errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
        }

        // method P
        switch ( pref.method_P )
        {
        case MethodCorrP_Thrift::type::CPM_AKI:
        {
            if (pref.solventSymbol.empty() || pref.solventSymbol == "*")
            {
                errorSolventNotDefined("solvent", pref.workSubstance.symbol(), __LINE__);
            }
            SoluteAkinfievDiamondEOS  aqAD  (pref.workSubstance);
                                      tps   = aqAD.thermoProperties(T, P, tps, pimpl->solvent.thermoProperties,
                                              pimpl->solvent.thermoIdealGasProperties, pimpl->solvent.properties);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CEH:
        {
            MinMurnaghanEOSHP98 minHP (pref.workSubstance);
                                tps = minHP.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBE:
        {
            MinBerman88 minBM (pref.workSubstance);
                        tps   = minBM.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBM:
        {
            MinBMGottschalk minBMG (pref.workSubstance);
                            tps    = minBMG.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CORK:
        {
            GasCORK gasCORK (pref.workSubstance);
                    tps     = gasCORK.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PRSV:
        {
            GasPRSV gasPRSV (pref.workSubstance);
                    tps     = gasPRSV.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_EMP:
        {
            GasCGF gasCGF (pref.workSubstance);
                   tps    = gasCGF.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_SRK:
        {
            GasSRK gasSRK (pref.workSubstance);
                   tps    = gasSRK.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PR78:
        {
            GasPR78 gasPR78 (pref.workSubstance);
                    tps     = gasPR78.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_STP:
        {
            GasSTP gasSTP (pref.workSubstance);
                   tps    = gasSTP.thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CON: // Molar volume assumed independent of T and P
        {
            ThermoPropertiesSubstance rtps = pref.workSubstance.thermoReferenceProperties();
            auto                      t    = Reaktoro_::Temperature(T + C_to_K);
            auto                      p    = Reaktoro_::Pressure(P);

            tps.volume           = rtps.volume;
            tps.gibbs_energy    += rtps.volume * (p - (pref.workSubstance.referenceP() / bar_to_Pa));
            tps.enthalpy        += rtps.volume * (p - (pref.workSubstance.referenceP() / bar_to_Pa));
            tps.internal_energy  = tps.enthalpy - p*tps.volume;
            tps.helmholtz_energy = tps.internal_energy - (t)*tps.entropy;

            break;
        }
        case MethodCorrP_Thrift::type::CPM_OFF:
        {
            auto t = Reaktoro_::Temperature(T + C_to_K);
            auto p = Reaktoro_::Pressure(P);

            if( pref.isGasFluid && P > 0.0 )
            { // molar volume from the ideal gas law
                tps.volume = (t) / p * R_CONSTANT;
            }
            break;
        }
            // Exception
            errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
        }
    }

    if (pref.isH2OSolvent || pref.isH2Ovapor)
    {
        switch(pref.method_T)
        {
        case MethodCorrT_Thrift::type::CTM_WAT:
        {
            WaterHGK water ( pref.workSubstance );
                     tps   = water.thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            WaterHGKreaktoro water ( pref.workSubstance );
                             tps   = water.thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            WaterWP95reaktoro water ( pref.workSubstance );
                              tps   = water.thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            WaterZhangDuan2005 water ( pref.workSubstance );
                               tps   = water.thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
            // Exception
            errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
        }
    }
   return tps;
}


auto Thermo::electroPropertiesSolvent(double T, double &P, std::string substance) -> ElectroPropertiesSolvent
{
    ThermoPreferences        pref = getThermoPreferences(substance);
    PropertiesSolvent        ps;
    ElectroPropertiesSolvent eps;

    if (pref.isH2OSolvent)
    {
        switch(pref.method_genEOS)
        {
        case MethodGenEoS_Thrift::type::CTPM_WJNR:
        {
            WaterJNreaktoro water (pref.workSubstance);
                            ps    = propertiesSolvent(T, P, pref.workSubstance.symbol());
                            eps   = water.electroPropertiesSolvent(T, P, ps);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WJNG:
        {
            WaterJNgems water (pref.workSubstance);
                        eps   = water.electroPropertiesSolvent(T, P/*, ps*/ );
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WSV14:
        {
            WaterElectroSverjensky2014 water (pref.workSubstance);
                                       eps   = water.electroPropertiesSolvent(T, P/*, ps*/);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WF97:
        {
            WaterElectroFernandez1997 water (pref.workSubstance);
                                       eps   = water.electroPropertiesSolvent(T, P/*, ps*/);
            break;
        }
            // Exception
            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
        }
    }

   return eps;
}

auto Thermo::propertiesSolvent(double T, double &P, std::string solvent) -> PropertiesSolvent
{
    ThermoPreferences pref = getThermoPreferences(solvent);
    PropertiesSolvent ps;

    if (pref.isH2OSolvent)
    {
        switch(pref.method_T)
        {
        case MethodCorrT_Thrift::type::CTM_WAT:
        {
            WaterHGK water ( pref.workSubstance );
                     ps    = water.propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            WaterHGKreaktoro water ( pref.workSubstance );
                             ps    = water.propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            WaterWP95reaktoro water ( pref.workSubstance );
                              ps    = water.propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            WaterZhangDuan2005 water ( pref.workSubstance );
                               ps    = water.propertiesSolvent(T, P, pref.solventState);
            break;
        }
            // Exception
            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
        }
    }
   return ps;
}

auto Thermo::getThermoPreferences(std::string substance) -> ThermoPreferences
{
    ThermoPreferences preferences;

    preferences.workSubstance    = pimpl->database.getSubstance(substance);
    preferences.solventSymbol = preferences.workSubstance.SolventSymbol();
    preferences.method_genEOS    = preferences.workSubstance.methodGenEOS();
    preferences.method_T         = preferences.workSubstance.method_T();
    preferences.method_P         = preferences.workSubstance.method_P();

    // check for H+
    if (preferences.workSubstance.name() == "H+")
        preferences.isHydrogen = true;
    else
        preferences.isHydrogen = false;

    // check for H2O vapor
    if (preferences.method_genEOS == MethodGenEoS_Thrift::type::CTPM_HKF && preferences.method_P == MethodCorrP_Thrift::type::CPM_GAS)
        preferences.isH2Ovapor = true;
    else
        preferences.isH2Ovapor = false;

    // check for solvent
    if (preferences.workSubstance.substanceClass() == SubstanceClass::type::AQSOLVENT /*&& !isH2Ovapor*/)
        preferences.isH2OSolvent = true;
    else
        preferences.isH2OSolvent = false;

    // check if substance is gas fluid
    if (preferences.workSubstance.substanceClass() == SubstanceClass::type::GASFLUID )
        preferences.isGasFluid = true;
    else
        preferences.isGasFluid = false;

    if (preferences.workSubstance.aggregateState() == AggregateState::type::GAS)
        preferences.solventState = 1;  // vapor
    else
        preferences.solventState = 0;  // liquid

    return preferences;
}

auto Thermo::calculateSolvent(std::string solventSymbol, double T, double &P, Solvent &solvent)-> void
{
    if ((solvent.T != T || solvent.P != P || solvent.solventSymbol != solventSymbol) &&  (solventSymbol != "*") && (!solventSymbol.empty()))
    {
        solvent.solventSymbol = solventSymbol;
        solvent.T = T;
        solvent.P = P;

        solvent.electroProperties = electroPropertiesSolvent(T, P, solventSymbol);
        solvent.properties        = propertiesSolvent(T, P, solventSymbol);
        solvent.thermoProperties  = thermoPropertiesSubstance(T, P, solventSymbol);

        WaterIdealGasWoolley      H2Oig (pimpl->database.getSubstance(solventSymbol));
        solvent.thermoIdealGasProperties = H2Oig.thermoProperties(T, P);
    }
}

} // namespace TCorrPT
