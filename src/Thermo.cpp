// ThermoFun includes
#include "Common/Exception.h"
#include "Thermo.h"
#include "Database.h"
#include "Reaction.h"
#include "Substance.h"
#include "ThermoModelsSubstance.h"
#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "ElectroModelsSolvent.h"
#include "ThermoModelsReaction.h"

namespace ThermoFun {

///
/// \brief The ThermoPreferences struct holds preferences such as the calculation methods for the current substance
///
struct ThermoPreferences
{
    Substance workSubstance;
    Reaction  workReaction;
    MethodGenEoS_Thrift::type method_genEOS;
    MethodCorrT_Thrift::type  method_T;
    MethodCorrP_Thrift::type  method_P;

    unsigned solventState = 0; // 0: liquid; 1: vapor

    bool isHydrogen     = false;
    bool isH2Ovapor     = false;
    bool isH2OSolvent   = false;
    bool isReacDC       = false;
};

///
/// \brief The Solvent struct hold the solvent proeprties at T and P
///
struct Solvent
{
    PropertiesSolvent         properties;
    ThermoPropertiesSubstance thermoProperties;
    ElectroPropertiesSolvent  electroProperties;
    ThermoPropertiesSubstance thermoIdealGasProperties;

    string symbol;

    double T, P;
};

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

//    calculateSolvent(pref.solventSymbol, T, P, pimpl->solvent);
    calculatePropertiesSolvent(T, P);

    if (pref.isHydrogen)
    {
        return tps;
    }

    if (!pref.isReacDC)
    {
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
                checkSolvent(pref.workSubstance.symbol());
                tps = SoluteHKFgems(pref.workSubstance).thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_HKFR:
            {
                checkSolvent(pref.workSubstance.symbol());
                tps = SoluteHKFreaktoro(pref.workSubstance).thermoProperties(T, P, pimpl->solvent.properties, pimpl->solvent.electroProperties);
                break;
            }
//                default:
//                // Exception
//                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
            }

            // method T
            switch ( pref.method_T )
            {
            case MethodCorrT_Thrift::type::CTM_CHP:
            {
                tps = HPLandau(pref.workSubstance).thermoProperties(T, P, tps);
                break;
            }
//                default:
//               // Exception
//                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
            }

            // method P
            switch ( pref.method_P )
            {
            case MethodCorrP_Thrift::type::CPM_AKI:
            {
                checkSolvent(pref.workSubstance.symbol());
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
//                default:
//                // Exception
//                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
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
//                default:
//                // Exception
//                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
            }
        }
    } else // substance proeprties calculated using the properties of a reaction
    {
        tps = reacDCthermoProperties(T, P, pref.workSubstance);
    }
   return tps;
}


auto Thermo::electroPropertiesSolvent(double T, double &P, std::string substance) -> ElectroPropertiesSolvent
{
    ThermoPreferences        pref = getThermoPreferences(substance);
    PropertiesSolvent        ps = propertiesSolvent(T, P, substance);;
    ElectroPropertiesSolvent eps;

    if (pref.isH2OSolvent)
    {
        switch(pref.method_genEOS)
        {
        case MethodGenEoS_Thrift::type::CTPM_WJNR:
        {
            checkSolvent(pref.workSubstance.symbol());
            eps = WaterJNreaktoro(pref.workSubstance).electroPropertiesSolvent(T, P, ps);
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
//            default:
//            // Exception
//            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
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
//            default:
//            // Exception
//            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
        }
    }
   return ps;
}

// Reaction
auto Thermo::thermoPropertiesReaction (double T, double &P, std::string reaction) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    auto reac = pimpl->database.getReaction(reaction);
    auto methodT = reac.method_T();
    auto methodP = reac.method_P();

    calculatePropertiesSolvent(T, P);

    switch (methodT)
    {
    case MethodCorrT_Thrift::type::CTM_LGX:
    case MethodCorrT_Thrift::type::CTM_LGK:
    case MethodCorrT_Thrift::type::CTM_EK0:
    case MethodCorrT_Thrift::type::CTM_EK1:
    case MethodCorrT_Thrift::type::CTM_EK3:
    case MethodCorrT_Thrift::type::CTM_EK2:
    {
        tpr = Reaction_LogK_fT(reac).thermoProperties(T, P, methodT);
        break;
    }
    case MethodCorrT_Thrift::type::CTM_DKR: // Marshall-Franck density model
    {
        checkSolvent(reaction);
        tpr = ReactionFrantzMarshall(reac).thermoProperties(T, P, pimpl->solvent.properties);
        break;
    }
    case MethodCorrT_Thrift::type::CTM_MRB: // Calling modified Ryzhenko-Bryzgalin model TW KD 08.2007
    {
        checkSolvent(reaction);
        return tpr = ReactionRyzhenkoBryzgalin(reac).thermoProperties(T, P, pimpl->solvent.properties); // NOT TESTED!!!
        break;
    }
    case MethodCorrT_Thrift::type::CTM_IKZ:
    {
        // calc_r_interp( q, p, CE, CV );
        break;
    }
//    default:
//        // Exception
//        errorMethodNotFound("reaction", reac.name(), __LINE__);
    }


    switch (methodP)
    {

    case MethodCorrP_Thrift::type::CPM_VKE:
    case MethodCorrP_Thrift::type::CPM_VBE:

    {
        tpr = Reaction_Vol_fT(reac).thermoProperties(T, P);
        break;
    }
    case MethodCorrP_Thrift::type::CPM_NUL:
    case MethodCorrP_Thrift::type::CPM_CON:
    {
        //    if( CV == CPM_CON || CV == CPM_NUL )
        //    {
        //            P_Pst = aW.twp->P - Pst;
        //            VP = Vst * P_Pst;
        //				// VT = Vst * T_Tst;
        //            aW.twp->dG += VP;
        //            aW.twp->dH += VP;
        //    }
        //    // Calculating pressure correction to logK
        //    aW.twp->lgK -= aW.twp->dV * (aW.twp->P - aW.twp->Pst) / aW.twp->RT / lg_to_ln;
        break;
    }
//    default:
//    // Exception
//    errorMethodNotFound("reaction", reac.name(), __LINE__);
    }

// make a new method P ???
// line 1571 m_reac2.cpp
//    if(( rc[q].pstate[0] == CP_GAS || rc[q].pstate[0] == CP_GASI ) && aW.twp->P > 0.0 )
//    { // molar volume from the ideal gas law
//        aW.twp->dV = T / aW.twp->P * R_CONSTANT;
//    }
    //     // Calculating pressure correction to logK
//    aW.twp->lgK -= aW.twp->dV * (aW.twp->P - aW.twp->Pst) / aW.twp->RT / lg_to_ln;

    return tpr;
}


auto Thermo::reacDCthermoProperties(double T, double &P, Substance subst) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps, reacTps;
    ThermoPropertiesReaction  tpr;
    string reactionSymbol = subst.reactionSymbol();
    Reaction reaction;
    typedef std::map<std::string, double>::iterator it;
    std::map<std::string, double> reactans;

    if (!reactionSymbol.empty())
    {

        reaction = pimpl->database.getReaction(reactionSymbol);

        tpr = thermoPropertiesReaction(T, P, reactionSymbol);

        tps.enthalpy         = tpr.reaction_enthalpy;
        tps.entropy          = tpr.reaction_entropy;
        tps.gibbs_energy     = tpr.reaction_gibbs_energy;
        tps.heat_capacity_cp = tpr.reaction_heat_capacity_cp;
        tps.heat_capacity_cv = tpr.reaction_heat_capacity_cv;
        tps.helmholtz_energy = tpr.reaction_helmholtz_energy;
        tps.internal_energy  = tpr.reaction_internal_energy;
        tps.volume           = tpr.reaction_volume;

        reactans = reaction.reactants();


        for(it iterator = reactans.begin(); iterator != reactans.end(); iterator++)
        {
            if (iterator->first != subst.symbol())
            {
                reacTps = thermoPropertiesSubstance(T, P, iterator->first);
                tps.enthalpy         -= reacTps.enthalpy*iterator->second;
                tps.entropy          -= reacTps.entropy*iterator->second;
                tps.gibbs_energy     -= reacTps.gibbs_energy*iterator->second;
                tps.heat_capacity_cp -= reacTps.heat_capacity_cp*iterator->second;
                tps.heat_capacity_cv -= reacTps.heat_capacity_cv*iterator->second;
                tps.helmholtz_energy -= reacTps.helmholtz_energy*iterator->second;
                tps.internal_energy  -= reacTps.internal_energy*iterator->second;
                tps.volume           -= reacTps.volume*iterator->second;
            }
        }

        tps.enthalpy         = tps.enthalpy/reactans[subst.symbol()];
        tps.entropy          = tps.entropy/reactans[subst.symbol()];
        tps.gibbs_energy     = tps.gibbs_energy/reactans[subst.symbol()];
        tps.heat_capacity_cp = tps.heat_capacity_cp/reactans[subst.symbol()];
        tps.heat_capacity_cv = tps.heat_capacity_cv/reactans[subst.symbol()];
        tps.helmholtz_energy = tps.helmholtz_energy/reactans[subst.symbol()];
        tps.internal_energy  = tps.internal_energy/reactans[subst.symbol()];
        tps.volume           = tps.volume/reactans[subst.symbol()];

    } else
    {
        errorReactionNotDefined(subst.symbol(), __LINE__, __FILE__);
    }


    return tps;
}

auto Thermo::thermoPropertiesReactionFromReactants (double T, double &P, std::string symbol) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction  tpr;
    tpr.reaction_heat_capacity_cp   = 0.0;
    tpr.reaction_gibbs_energy       = 0.0;
    tpr.reaction_enthalpy           = 0.0;
    tpr.reaction_entropy            = 0.0;
    tpr.reaction_volume             = 0.0;
    tpr.ln_equilibrium_constant     = 0.0;
    tpr.log_equilibrium_constant    = 0.0;

    Reaction reaction = pimpl->database.getReaction(symbol); 
    string message = "Calculated from the reaction components: " + reaction.symbol() + "; ";

    for (auto &reactant : reaction.reactants())
    {
        auto coeff = reactant.second;
        auto substance = reactant.first;
        auto tps = thermoPropertiesSubstance(T, P, substance);

        tpr.reaction_heat_capacity_cp += tps.heat_capacity_cp*coeff;
        tpr.reaction_gibbs_energy += tps.gibbs_energy*coeff;
        tpr.reaction_enthalpy += tps.enthalpy*coeff;
        tpr.reaction_entropy += tps.entropy*coeff;
        tpr.reaction_volume += tps.volume*coeff;
        tpr.ln_equilibrium_constant = tpr.reaction_gibbs_energy / -(R_CONSTANT*(T+C_to_K));
        tpr.log_equilibrium_constant = tpr.ln_equilibrium_constant * ln_to_lg;

        setMessage(tps.heat_capacity_cp.sta.first, "Cp of component " + substance, message+tps.heat_capacity_cp.sta.second, tpr.reaction_heat_capacity_cp.sta.second);
        setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.reaction_gibbs_energy.sta.second);
        setMessage(tps.enthalpy.sta.first,         "H0 of component " + substance, message+tps.enthalpy.sta.second,         tpr.reaction_enthalpy.sta.second);
        setMessage(tps.entropy.sta.first,          "S0 of component " + substance, message+tps.entropy.sta.second,          tpr.reaction_entropy.sta.second);
        setMessage(tps.volume.sta.first,           "V0 of component " + substance, message+tps.volume.sta.second,           tpr.reaction_volume.sta.second);
        setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.log_equilibrium_constant.sta.second);
        setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.ln_equilibrium_constant.sta.second);
    }
    return tpr;
}

auto Thermo::getThermoPreferences(std::string substance) -> ThermoPreferences
{
    ThermoPreferences preferences;

    preferences.workSubstance    = pimpl->database.getSubstance(substance);
//    preferences.solventSymbol    = preferences.workSubstance.SolventSymbol();
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

    // check if the substance is reaction dependent
    if (preferences.workSubstance.thermoCalculationType() == SubstanceThermoCalculationType::type::REACDC)
        preferences.isReacDC = true;
    else
        preferences.isReacDC = false;

    // make check if substance is aq solute and needs a solvent

    return preferences;
}

auto Thermo::calculateSolvent(std::string solventSymbol, double T, double &P, Solvent &solvent)-> void
{
    if ((solvent.T != T || solvent.P != P || solvent.symbol != solventSymbol) &&  (solventSymbol != "*") && (!solventSymbol.empty()))
    {
        solvent.symbol                   = solventSymbol;
        solvent.T                        = T;
        solvent.P                        = P;
        solvent.properties               = propertiesSolvent(T, P, solventSymbol);
        solvent.electroProperties        = electroPropertiesSolvent(T, P, solventSymbol);
        solvent.thermoProperties         = thermoPropertiesSubstance(T, P, solventSymbol);
        solvent.thermoIdealGasProperties = WaterIdealGasWoolley(pimpl->database.getSubstance(solventSymbol)).thermoProperties(T, P);
    }
}

auto Thermo::calculatePropertiesSolvent(double T, double &P)-> void
{
    if ((pimpl->solvent.T != T || pimpl->solvent.P != P) &&
        (pimpl->solvent.symbol != "*") && (!pimpl->solvent.symbol.empty()))
    {
        pimpl->solvent.T                        = T;
        pimpl->solvent.properties               = propertiesSolvent(T, P, pimpl->solvent.symbol);
        pimpl->solvent.P                        = P;
        pimpl->solvent.electroProperties        = electroPropertiesSolvent(T, P, pimpl->solvent.symbol);
        pimpl->solvent.thermoProperties         = thermoPropertiesSubstance(T, P, pimpl->solvent.symbol);
        pimpl->solvent.thermoIdealGasProperties = WaterIdealGasWoolley(pimpl->database.getSubstance(pimpl->solvent.symbol)).thermoProperties(T, P);
    }
}

auto Thermo::checkSolvent(std::string symbol) -> void
{
    if (pimpl->solvent.symbol.empty() || pimpl->solvent.symbol == "*")
    {
        errorSolventNotDefined("solvent", symbol, __LINE__, __FILE__);
    }
}


auto Thermo::setSolventSymbol(const std::string solvent_symbol) ->void
{
    pimpl->solvent.symbol = solvent_symbol;
}

auto Thermo::solventSymbol( ) const -> std::string
{
    return pimpl->solvent.symbol;
}

} // namespace ThermoFun
