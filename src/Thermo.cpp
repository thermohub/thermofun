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
            tps = EmpiricalCpIntegration(pref.workSubstance).thermoProperties(T, P);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKF:
        {
            tps = SoluteHKFgems(pref.workSubstance).thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_HKFR:
        {
            tps = SoluteHKFreaktoro(pref.workSubstance).thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
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
            tps = HPLandau(pref.workSubstance).thermoProperties(T, P, tps);
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
            tps = SoluteAkinfievDiamondEOS(pref.workSubstance).thermoProperties(T, P, tps, pimpl->solvent.thermoProperties,
                                           pimpl->solvent.thermoIdealGasProperties, pimpl->solvent.properties);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CEH:
        {
            tps = MinMurnaghanEOSHP98(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBE:
        {
            tps = MinBerman88(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_VBM:
        {
            tps = MinBMGottschalk(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CORK:
        {
            tps = GasCORK(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PRSV:
        {
            tps = GasPRSV(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_EMP:
        {
            tps = GasCGF(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_SRK:
        {
            tps = GasSRK(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_PR78:
        {
            tps = GasPR78(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_STP:
        {
            tps = GasSTP(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_CON: // Molar volume assumed independent of T and P
        {
            tps = ConMolVol(pref.workSubstance).thermoProperties(T, P, tps);
            break;
        }
        case MethodCorrP_Thrift::type::CPM_OFF:
        {
            tps = IdealGasLawVol(pref.workSubstance).thermoProperties(T, P, tps);
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
            tps = WaterHGK(pref.workSubstance).thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            tps = WaterHGKreaktoro(pref.workSubstance).thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            tps = WaterWP95reaktoro(pref.workSubstance).thermoPropertiesSubstance(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            tps = WaterZhangDuan2005(pref.workSubstance).thermoPropertiesSubstance(T, P, pref.solventState);
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
            eps = WaterJNreaktoro(pref.workSubstance).electroPropertiesSolvent(T, P, pimpl->solvent.properties);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WJNG:
        {
            eps = WaterJNgems(pref.workSubstance).electroPropertiesSolvent(T, P/*, ps*/ );
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WSV14:
        {
            eps = WaterElectroSverjensky2014(pref.workSubstance).electroPropertiesSolvent(T, P/*, ps*/);
            break;
        }
        case MethodGenEoS_Thrift::type::CTPM_WF97:
        {
            eps = WaterElectroFernandez1997(pref.workSubstance).electroPropertiesSolvent(T, P/*, ps*/);
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
            ps = WaterHGK(pref.workSubstance).propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WAR:
        {
            ps = WaterHGKreaktoro(pref.workSubstance).propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WWP:
        {
            ps = WaterWP95reaktoro(pref.workSubstance).propertiesSolvent(T, P, pref.solventState);
            break;
        }
        case MethodCorrT_Thrift::type::CTM_WZD:
        {
            ps = WaterZhangDuan2005(pref.workSubstance).propertiesSolvent(T, P, pref.solventState);
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
    preferences.solventSymbol    = preferences.workSubstance.SolventSymbol();
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

    // set solvent state
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
        solvent.solventSymbol            = solventSymbol;
        solvent.T                        = T;
        solvent.P                        = P;
        solvent.properties               = propertiesSolvent(T, P, solventSymbol);
        solvent.electroProperties        = electroPropertiesSolvent(T, P, solventSymbol);
        solvent.thermoProperties         = thermoPropertiesSubstance(T, P, solventSymbol);
        solvent.thermoIdealGasProperties = WaterIdealGasWoolley(pimpl->database.getSubstance(solventSymbol)).thermoProperties(T, P);
    }
}

auto Thermo::setSolventSymbolForAllAqSubst(const std::string solvent_symbol) ->void
{
    pimpl->database.setAllAqSubstanceSolventSymbol(solvent_symbol);
}

} // namespace TCorrPT
