#include "ThermoCalculations.h"
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

auto getThermoPreferences(Substance substance) -> ThermoPreferences
{
    ThermoPreferences preferences;

    preferences.workSubstance    = substance;
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

auto calcThermoPropertiesSubstance(double T, double &P, Substance substance, Substance solvent) -> ThermoPropertiesSubstance
{
    ThermoPreferences         pref = getThermoPreferences(substance);
    ThermoPropertiesSubstance tps;

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
                PropertiesSolvent ps = calcPropertiesSolvent(T,P,solvent);
                ElectroPropertiesSolvent eps = calcElectroPropertiesSolvent(T, P, solvent);
                tps = SoluteHKFgems(pref.workSubstance).thermoProperties(T, P, ps, eps);
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_HKFR:
            {
                PropertiesSolvent ps = calcPropertiesSolvent(T,P,solvent);
                ElectroPropertiesSolvent eps = calcElectroPropertiesSolvent(T, P, solvent);
                tps = SoluteHKFreaktoro(pref.workSubstance).thermoProperties(T, P, ps, eps);
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
                PropertiesSolvent ps = calcPropertiesSolvent(T,P,solvent);
                ThermoPropertiesSubstance tpss = calcThermoPropertiesSubstance(T, P, solvent, solvent);
                ThermoPropertiesSubstance igp = WaterIdealGasWoolley(solvent).thermoProperties(T, P);
                tps = SoluteAkinfievDiamondEOS(pref.workSubstance).thermoProperties(T, P, tps, tpss,
                                               igp, ps);
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
    } /*else // substance properties calculated using the properties of a reaction
    {
        tps = reacDCthermoProperties(T, P, pref.workSubstance);
    }*/
   return tps;
}


auto calcElectroPropertiesSolvent(double T, double &P, Substance solvent) -> ElectroPropertiesSolvent
{
    ThermoPreferences        pref = getThermoPreferences(solvent);
    ElectroPropertiesSolvent eps;

    if (pref.isH2OSolvent)
    {
        switch(pref.method_genEOS)
        {
        case MethodGenEoS_Thrift::type::CTPM_WJNR:
        {
            PropertiesSolvent        ps = calcPropertiesSolvent(T, P, solvent);
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

auto calcPropertiesSolvent(double T, double &P, Substance solvent) -> PropertiesSolvent
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
auto thermoPropertiesReaction (double T, double &P, Reaction reaction, Substance solvent) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    auto reac = reaction;
    auto methodT = reac.method_T();
    auto methodP = reac.method_P();
    PropertiesSolvent ps = calcPropertiesSolvent(T, P, solvent);


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
        tpr = ReactionFrantzMarshall(reac).thermoProperties(T, P, ps);
        break;
    }
    case MethodCorrT_Thrift::type::CTM_MRB: // Calling modified Ryzhenko-Bryzgalin model TW KD 08.2007
    {
        return tpr = ReactionRyzhenkoBryzgalin(reac).thermoProperties(T, P, ps); // NOT TESTED!!!
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


}
