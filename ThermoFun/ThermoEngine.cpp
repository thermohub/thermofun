// ThermoFun includes
#include "Common/Exception.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Reaction.h"
#include "Substance.h"
#include "Element.h"
#include "ThermoModelsSubstance.h"
#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/Rounding.hpp"
#include "ElectroModelsSolvent.h"
#include "ThermoModelsReaction.h"

#include "OptimizationUtils.h"
#include <functional>
#include <cmath>
#include <set>
#include <array>

namespace ThermoFun
{

bool iequals(const std::string &a, const std::string &b)
{
    size_t sz = a.size();
    if (b.size() != sz)
        return false;
    for (size_t i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

///
/// \brief The ThermoPreferences struct holds preferences such as the calculation methods for the current substance
///
struct WorkPreferences
{
    Substance workSubstance;
    Reaction workReaction;
    MethodGenEoS_Thrift::type method_genEOS;
    MethodCorrT_Thrift::type method_T;
    MethodCorrP_Thrift::type method_P;

    bool isHydrogen = false;
    bool isH2Ovapor = false;
    bool isH2OSolvent = false;
    bool isReacDC = false;
    bool isReacFromReactants = false;
};

using ThermoPropertiesSubstanceFunction =
    std::function<ThermoPropertiesSubstance(double, double, double &, std::string)>;

using ElectroPropertiesSolventFunction =
    std::function<ElectroPropertiesSolvent(double, double, double &, std::string, int)>;

using PropertiesSolventFunction =
    std::function<PropertiesSolvent(double, double, double &, std::string, int)>;

using ThermoPropertiesReactionFunction =
    std::function<ThermoPropertiesReaction(double, double, double &, std::string)>;

struct ThermoEngine::Impl
{
    /// The database instance
    Database database;

    mutable EnginePreferences preferences;

    mutable WorkPreferences prefs;

    mutable EngineConventions conventions;

    ThermoPropertiesSubstanceFunction thermo_properties_substance_fn;

    ElectroPropertiesSolventFunction electro_properties_solvent_fn;

    PropertiesSolventFunction properties_solvent_fn;

    ThermoPropertiesReactionFunction thermo_properties_reaction_fn;

    Impl()
    {
        set_fn() ;
    }

    Impl(const Database &database)
        : database(database)
    {
        set_fn();
    }

    void set_fn()
    {
        thermo_properties_substance_fn = [=, this](double T, double P_, double &P, std::string symbol) {
            auto x = P_;
            return thermoPropertiesSubstance(T, P, symbol);
        };

        electro_properties_solvent_fn = [=, this](double T, double P_, double &P, std::string symbol, int state) {
            auto x = P_;
            return electroPropertiesSolvent(T, P, symbol, state);
        };

        properties_solvent_fn = [=, this](double T, double P_, double &P, std::string symbol, int state) {
            auto x = P_;
            return propertiesSolvent(T, P, symbol, state);
        };

        thermo_properties_reaction_fn = [=, this](double T, double P_, double &P, std::string symbol) {
            auto x = P_;
            return thermoPropertiesReaction(T, P, symbol);
        };

        if( preferences.enable_memoize ) {
            if( preferences.max_cache_size==0 ) {
                thermo_properties_substance_fn = memoize(thermo_properties_substance_fn);
                electro_properties_solvent_fn = memoize(electro_properties_solvent_fn);
                properties_solvent_fn = memoize(properties_solvent_fn);
                thermo_properties_reaction_fn = memoize(thermo_properties_reaction_fn);
            }
            else {
                thermo_properties_substance_fn = memoizeN(thermo_properties_substance_fn, preferences.max_cache_size);
                electro_properties_solvent_fn = memoizeN(electro_properties_solvent_fn,preferences.max_cache_size);
                properties_solvent_fn = memoizeN(properties_solvent_fn,preferences.max_cache_size);
                thermo_properties_reaction_fn = memoizeN(thermo_properties_reaction_fn,preferences.max_cache_size);
            }
        }
    }

    auto toSteamTables(ThermoPropertiesSubstance &tps, WaterTripleProperties wat) const -> void
    {
        // constants are subtracted: the derivatives, error and status are not changed
        tps.gibbs_energy.val -= wat.Gtr;
        tps.enthalpy.val -= wat.Htr;
        tps.entropy.val -= wat.Str;
        tps.helmholtz_energy.val -= wat.Atr;
        tps.internal_energy.val -= wat.Utr;
    }

    auto toBermanBrown(ThermoPropertiesSubstance &tps, const Substance &subst) const -> void
    {
        const auto Tr = subst.referenceT();
        const auto entropyElements = database.elementalEntropyFormula(subst.formula());
        tps.gibbs_energy.val -= (Tr * entropyElements); // a constant: the derivatives, error and status are not changed
    }

    auto getThermoPreferencesReaction(const Reaction &reaction) const -> WorkPreferences
    {
        WorkPreferences preferences;

        preferences.workReaction = reaction;
        preferences.method_genEOS = preferences.workReaction.methodGenEOS();
        preferences.method_T = preferences.workReaction.method_T();
        preferences.method_P = preferences.workReaction.method_P();

        // if no method is present try to calculate properties of reaction from reactants
        if ((!preferences.method_genEOS && !preferences.method_P && !preferences.method_T))
            preferences.isReacFromReactants = true;

        return preferences;
    }

    auto getThermoPreferencesSubstance(const Substance &substance) const -> WorkPreferences
    {
        WorkPreferences workPreferences;
        workPreferences.workSubstance = substance;
        workPreferences.method_genEOS = workPreferences.workSubstance.methodGenEOS();
        workPreferences.method_T = workPreferences.workSubstance.method_T();
        workPreferences.method_P = workPreferences.workSubstance.method_P();

        // check for H+
        if (workPreferences.workSubstance.symbol() == "H+")
            workPreferences.isHydrogen = true;
        else
            workPreferences.isHydrogen = false;

        // check for H2O vapor
        if (workPreferences.method_genEOS == MethodGenEoS_Thrift::type::CTPM_HKF && workPreferences.method_P == MethodCorrP_Thrift::type::CPM_GAS)
            workPreferences.isH2Ovapor = true;
        else
            workPreferences.isH2Ovapor = false;

        // check for solvent
        if (workPreferences.workSubstance.substanceClass() == SubstanceClass::type::AQSOLVENT /*&& !isH2Ovapor*/)
            workPreferences.isH2OSolvent = true;
        else
            workPreferences.isH2OSolvent = false;

        // set solvent state
        if (workPreferences.workSubstance.aggregateState() == AggregateState::type::GAS)
            preferences.solvent_state = 1; // vapor
        else
            preferences.solvent_state = 0; // liquid

        bool noMethods =
            !workPreferences.method_genEOS &&
            !workPreferences.method_P &&
            !workPreferences.method_T;

        bool constVm =
            !workPreferences.method_genEOS &&
            workPreferences.method_P == MethodCorrP_Thrift::CPM_CON &&
            !workPreferences.method_T;

        if (workPreferences.workSubstance.thermoCalculationType() ==
                SubstanceThermoCalculationType::type::REACDC
            || noMethods || constVm)
        {
            workPreferences.isReacDC = true;
        }
        else
            workPreferences.isReacDC = false;

        // check if substance is aq solute and needs a solvent
        if (workPreferences.workSubstance.substanceClass() == SubstanceClass::type::AQSOLUTE)
        {
            // see if default solvent is in the database if not search for solvent
            if (!database.containsSubstance(preferences.solvent_symbol))
            {
                for (const auto& pair : database.mapSubstances()) {
                    if (pair.second.substanceClass() == SubstanceClass::type::AQSOLVENT) {
                        preferences.solvent_symbol = pair.first;
                    }
                }
            }
        }

        return workPreferences;
    }

    auto thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
    {
        return thermoPropertiesSubstance(T, P, database.getSubstance(substance));
    }

    auto thermoPropertiesSubstance(double T, double &P, const Substance &substance) const -> ThermoPropertiesSubstance
    {
        WorkPreferences pref = getThermoPreferencesSubstance(substance);
        ThermoPropertiesSubstance tps;

        if (pref.isHydrogen)
        {
            tps.volume = 0.0;
            tps.entropy = 0.0;
            tps.enthalpy = 0.0;
            tps.gibbs_energy = 0.0;
            tps.internal_energy = 0.0;
            tps.heat_capacity_cp = 0.0;
            tps.heat_capacity_cv = 0.0;
            tps.helmholtz_energy = 0.0;
            return tps;
        }

        if (!pref.isReacDC)
        {
            if (!pref.isH2OSolvent && !pref.isH2Ovapor)
            {
                // metohd EOS
                switch (pref.method_genEOS)
                {
                case MethodGenEoS_Thrift::type::CTPM_CON:
                {
                    tps = substance.thermoReferenceProperties();
                    break;
                }
                case MethodGenEoS_Thrift::type::CTPM_CPT:
                {
                    tps = EmpiricalCpIntegration(pref.workSubstance).thermoProperties(T, P);
                    break;
                }
                case MethodGenEoS_Thrift::type::CTPM_HKF:
                {
                    tps = SoluteHKFgems(pref.workSubstance).thermoProperties(T, P, properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1), electro_properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                    break;
                }
                case MethodGenEoS_Thrift::type::CTPM_HKFR:
                {
                    tps = SoluteHKFreaktoro(pref.workSubstance).thermoProperties(T, P, properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1), electro_properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                    break;
                }
                case MethodGenEoS_Thrift::type::CTPM_HP98:
                {
                    double Pr = database.getSubstance(preferences.solvent_symbol).referenceP();
                    double Tr = database.getSubstance(preferences.solvent_symbol).referenceT();
                    tps = SoluteHollandPowell98(pref.workSubstance).thermoProperties(T, P, properties_solvent_fn(Tr, Pr, Pr, preferences.solvent_symbol, -1), properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                    break;
                }
                case MethodGenEoS_Thrift::type::CTPM_AN91:
                {
                    double Pr = database.getSubstance(preferences.solvent_symbol).referenceP();
                    double Tr = database.getSubstance(preferences.solvent_symbol).referenceT();
                    tps = SoluteAnderson91(pref.workSubstance).thermoProperties(T, P, properties_solvent_fn(Tr, Pr, Pr, preferences.solvent_symbol, -1), properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                    break;
                }
                    //                default:
                    //                // Exception
                    //                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
                }

                // method T
                if (pref.method_genEOS != MethodGenEoS_Thrift::type::CTPM_CON)
                switch (pref.method_T)
                {
                case MethodCorrT_Thrift::type::CTM_CHP:
                {
                    tps = HPLandau(pref.workSubstance).thermoProperties(T, P, tps);
                    break;
                }
                case MethodCorrT_Thrift::type::CTM_CST:
                {
                    if (pref.method_genEOS != MethodGenEoS_Thrift::type::CTPM_CPT)
                        tps = EntropyCpIntegration(pref.workSubstance).thermoProperties(T, P);
                    break;
                }
                    //                default:
                    //               // Exception
                    //                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
                }

                // method P
                if (pref.method_genEOS != MethodGenEoS_Thrift::type::CTPM_CON)
                switch (pref.method_P)
                {
                case MethodCorrP_Thrift::type::CPM_AKI:
                {
                    double Pr = database.getSubstance(preferences.solvent_symbol).referenceP();
                    double Tr = database.getSubstance(preferences.solvent_symbol).referenceT();
                    tps = SoluteAkinfievDiamondEOS(pref.workSubstance).thermoProperties(T, P, tps, thermo_properties_substance_fn(T, P, P, preferences.solvent_symbol),
                                                                                        WaterIdealGasWoolley(database.getSubstance(preferences.solvent_symbol)).thermoProperties(T, P),
                                                                                        properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1),
                                                                                        thermo_properties_substance_fn(Tr, Pr, Pr, preferences.solvent_symbol),
                                                                                        WaterIdealGasWoolley(database.getSubstance(preferences.solvent_symbol)).thermoProperties(Tr, Pr),
                                                                                        properties_solvent_fn(Tr, Pr, Pr, preferences.solvent_symbol, -1));
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
                    tps = GasCORK(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_PRSV:
                {
                    tps = GasPRSV(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_EMP:
                {
                    tps = GasCGF(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_SRK:
                {
                    tps = GasSRK(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_PR78:
                {
                    tps = GasPR78(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_STP:
                {
                    tps = GasSTP(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_OFF:
                {
                    tps = IdealGasLawVol(pref.workSubstance).thermoProperties(T, P, tps, preferences.apply_pressure_correction_to_gas_props);
                    break;
                }
                case MethodCorrP_Thrift::type::CPM_CON: // Molar volume assumed independent of T and P
                {
                    tps = ConMolVol(pref.workSubstance).thermoProperties(T, P, tps);
                    break;
                }
                    //                default:
                    //                // Exception
                    //                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
                }
            }

            auto water_triple_reference = database.conventions().water_triple_point_reference;

            if (pref.isH2OSolvent || pref.isH2Ovapor)
            {
                if (pref.method_genEOS == MethodGenEoS_Thrift::type::CTPM_CON)
                {
                    tps = substance.thermoReferenceProperties();
                } else
                switch (pref.method_T)
                {
                case MethodCorrT_Thrift::type::CTM_WAT:
                {
                    tps = WaterHGK(pref.workSubstance).thermoPropertiesSubstance(T, P, preferences.solvent_state, water_triple_reference);
                    break;
                }
                case MethodCorrT_Thrift::type::CTM_WAR:
                {
                    tps = WaterHGKreaktoro(pref.workSubstance).thermoPropertiesSubstance(T, P, preferences.solvent_state, water_triple_reference);
                    break;
                }
                case MethodCorrT_Thrift::type::CTM_WWP:
                {
                    tps = WaterWP95reaktoro(pref.workSubstance).thermoPropertiesSubstance(T, P, preferences.solvent_state, water_triple_reference);
                    break;
                }
                case MethodCorrT_Thrift::type::CTM_WZD:
                {
                    tps = WaterZhangDuan2005(pref.workSubstance).thermoPropertiesSubstance(T, P, preferences.solvent_state);
                    break;
                }
                default:
                    switch (pref.method_genEOS)
                    {
                    case MethodGenEoS_Thrift::type::CTPM_CPT:
                    {
                        tps = EmpiricalCpIntegration(pref.workSubstance).thermoProperties(T, P);
                        break;
                    }
                        //                    default:
                    }
                    //                default:
                    //                // Exception
                    //                errorMethodNotFound("substance", pref.workSubstance.symbol(), __LINE__);
                }
            }

            /// Convetion convert
            if (pref.isH2OSolvent)
            {
                if (iequals(conventions.water_properties, "STEAM_TABLES"))
                {
                    toSteamTables(tps, waterTripleData.at(water_triple_reference));
                }
            }
            else
            {
                if (iequals(conventions.apparent_properties, "BERMAN_BROWN"))
                {
                    toBermanBrown(tps, pref.workSubstance);
                }
            }
        }
        else // substance properties calculated using the properties of a reaction
        {
            tps = reacDCthermoProperties(T, P, pref.workSubstance);
        }

        // check properties
        tps = fallbackThermoPropertiesSubstance(tps, pref.workSubstance);

        return tps;
    }

    auto fallbackThermoPropertiesSubstance(ThermoPropertiesSubstance tps, Substance subst) const -> ThermoPropertiesSubstance
    {
        auto tpref = subst.thermoReferenceProperties();
        // leave volume as is in substance record if no function to calculate it exists
        if (preferences.fallback_to_reference_properties)
        {
            if (tps.volume.sta.first == Reaktoro_::Status::notdefined)
                tps.volume = tpref.volume;
            if (tps.gibbs_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.gibbs_energy = tpref.gibbs_energy;
            if (tps.enthalpy.sta.first == Reaktoro_::Status::notdefined)
                tps.enthalpy = tpref.enthalpy;
            if (tps.entropy.sta.first == Reaktoro_::Status::notdefined)
                tps.entropy = tpref.entropy;
            if (tps.heat_capacity_cp.sta.first == Reaktoro_::Status::notdefined)
                tps.heat_capacity_cp = tpref.heat_capacity_cp;
            if (tps.heat_capacity_cv.sta.first == Reaktoro_::Status::notdefined)
                tps.heat_capacity_cv = tpref.heat_capacity_cv;
            if (tps.internal_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.internal_energy = tpref.internal_energy;
            if (tps.helmholtz_energy.sta.first == Reaktoro_::Status::notdefined)
                tps.helmholtz_energy = tpref.helmholtz_energy;
        }
        return tps;
    }

    auto electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
    {
        return electroPropertiesSolvent(T, P, database.getSubstance(solvent), state);
    }

    auto electroPropertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> ElectroPropertiesSolvent
    {
        WorkPreferences pref = getThermoPreferencesSubstance(solvent);
        if (state>-1 && state<2)
            preferences.solvent_state = state;
        PropertiesSolvent ps = propertiesSolvent(T, P, solvent, preferences.solvent_state);
        ElectroPropertiesSolvent eps;

        if (pref.isH2OSolvent)
        {
            switch (pref.method_genEOS)
            {
            case MethodGenEoS_Thrift::type::CTPM_WJNR:
            {
                eps = WaterJNreaktoro(pref.workSubstance).electroPropertiesSolvent(T, P, ps, preferences.solvent_state);
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_WJNG:
            {
                eps = WaterJNgems(pref.workSubstance).electroPropertiesSolvent(T, P /*, ps*/, preferences.solvent_state);
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_WSV14:
            {
                eps = WaterElectroSverjensky2014(pref.workSubstance).electroPropertiesSolvent(T, P /*, ps*/, preferences.solvent_state);
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_WF97:
            {
                eps = WaterElectroFernandez1997(pref.workSubstance).electroPropertiesSolvent(T, P /*, ps*/, preferences.solvent_state);
                break;
            }
                //            default:
                //            // Exception
                //            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
            }
        }
        return eps;
    }

    auto propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
    {
        return propertiesSolvent(T, P, database.getSubstance(solvent), state);
    }

    auto propertiesSolvent(double T, double &P, const Substance &solvent, int state) const -> PropertiesSolvent
    {
        WorkPreferences pref = getThermoPreferencesSubstance(solvent);
        if (state > -1 && state <2)
            preferences.solvent_state = state;
        PropertiesSolvent ps;

        if (pref.isH2OSolvent)
        {
            switch (pref.method_T)
            {
            case MethodCorrT_Thrift::type::CTM_WAT:
            {
                ps = WaterHGK(pref.workSubstance).propertiesSolvent(T, P, preferences.solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WAR:
            {
                ps = WaterHGKreaktoro(pref.workSubstance).propertiesSolvent(T, P, preferences.solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WWP:
            {
                ps = WaterWP95reaktoro(pref.workSubstance).propertiesSolvent(T, P, preferences.solvent_state);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_WZD:
            {
                ps = WaterZhangDuan2005(pref.workSubstance).propertiesSolvent(T, P, preferences.solvent_state);
                break;
            }
                //            default:
                //            // Exception
                //            errorMethodNotFound("solvent", pref.workSubstance.symbol(), __LINE__);
            }
        }
        return ps;
    }

    auto reacDCthermoProperties(double T, double &P, Substance subst) const -> ThermoPropertiesSubstance
    {
        ThermoPropertiesSubstance tps, reacTps;
        ThermoPropertiesReaction tpr;
        std::string reactionSymbol = subst.reactionSymbol();
        Reaction reaction;
        std::map<std::string, double> reactants;

        // if reaction involves gases we switch on the P correction to the thermo props
        preferences.apply_pressure_correction_to_gas_props = true;

        if (!reactionSymbol.empty())
        {
            reaction = database.getReaction(reactionSymbol);

            tpr = thermo_properties_reaction_fn(T, P, P, reactionSymbol); /*thermoPropertiesReaction(T, P, reactionSymbol);*/

            reactants = reaction.reactants();

            // the properties of the other reactants
            std::vector<std::pair<ThermoPropertiesSubstance, double>> others;
            for (const auto& reactant : reactants)
            {
                if (reactant.first != subst.symbol())
                    others.push_back({thermo_properties_substance_fn(T, P, P, reactant.first) /* thermoPropertiesSubstance(T, P, reactant.first);*/, reactant.second});
            }

            const double coeff = reactants[subst.symbol()];

            // the properties of the substance from the properties of the reaction and of the other reactants
            tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
                ThermoPropertiesSubstanceAD a;
                a.enthalpy         = pass(tpr.reaction_enthalpy);
                a.entropy          = pass(tpr.reaction_entropy);
                a.gibbs_energy     = pass(tpr.reaction_gibbs_energy);
                a.heat_capacity_cp = pass(tpr.reaction_heat_capacity_cp);
                a.heat_capacity_cv = pass(tpr.reaction_heat_capacity_cv);
                a.helmholtz_energy = pass(tpr.reaction_helmholtz_energy);
                a.internal_energy  = pass(tpr.reaction_internal_energy);
                a.volume           = pass(tpr.reaction_volume);

                for (const auto& other : others)
                {
                    a.enthalpy         -= pass(other.first.enthalpy) * other.second;
                    a.entropy          -= pass(other.first.entropy) * other.second;
                    a.gibbs_energy     -= pass(other.first.gibbs_energy) * other.second;
                    a.heat_capacity_cp -= pass(other.first.heat_capacity_cp) * other.second;
                    a.heat_capacity_cv -= pass(other.first.heat_capacity_cv) * other.second;
                    a.helmholtz_energy -= pass(other.first.helmholtz_energy) * other.second;
                    a.internal_energy  -= pass(other.first.internal_energy) * other.second;
                    a.volume           -= pass(other.first.volume) * other.second;
                }

                a.enthalpy         = a.enthalpy / coeff;
                a.entropy          = a.entropy / coeff;
                a.gibbs_energy     = a.gibbs_energy / coeff;
                a.heat_capacity_cp = a.heat_capacity_cp / coeff;
                a.heat_capacity_cv = a.heat_capacity_cv / coeff;
                a.helmholtz_energy = a.helmholtz_energy / coeff;
                a.internal_energy  = a.internal_energy / coeff;
                a.volume           = a.volume / coeff;
                return a;
            });

            // each property is not defined if the property of the reaction or of any other reactant is not defined
            auto setStatus = [&](Reaktoro_::ThermoProperty& p, const Reaktoro_::ThermoProperty& fromReaction,
                                 Reaktoro_::ThermoProperty ThermoPropertiesSubstance::* member)
            {
                p.propagateFrom(fromReaction);
                for (const auto& other : others)
                    p.propagateFrom(p, other.first.*member);
            };
            setStatus(tps.enthalpy,         tpr.reaction_enthalpy,          &ThermoPropertiesSubstance::enthalpy);
            setStatus(tps.entropy,          tpr.reaction_entropy,           &ThermoPropertiesSubstance::entropy);
            setStatus(tps.gibbs_energy,     tpr.reaction_gibbs_energy,      &ThermoPropertiesSubstance::gibbs_energy);
            setStatus(tps.heat_capacity_cp, tpr.reaction_heat_capacity_cp,  &ThermoPropertiesSubstance::heat_capacity_cp);
            setStatus(tps.heat_capacity_cv, tpr.reaction_heat_capacity_cv,  &ThermoPropertiesSubstance::heat_capacity_cv);
            setStatus(tps.helmholtz_energy, tpr.reaction_helmholtz_energy,  &ThermoPropertiesSubstance::helmholtz_energy);
            setStatus(tps.internal_energy,  tpr.reaction_internal_energy,   &ThermoPropertiesSubstance::internal_energy);
            setStatus(tps.volume,           tpr.reaction_volume,            &ThermoPropertiesSubstance::volume);
        }
        else
        {
            subst.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_CON);
            return thermoPropertiesSubstance(T, P, subst);//thermo_properties_substance_fn(T, P, P, subst.symbol()); //
            errorReactionNotDefined(subst.symbol(), __LINE__, __FILE__);
        }

        preferences.apply_pressure_correction_to_gas_props = false;

        // pressure correction on rdc assuming constant molar volume
        if ((subst.method_P() == MethodCorrP_Thrift::type::CPM_CON) &&
            (reaction.method_P()== MethodCorrP_Thrift::CPM_OFF || !reaction.method_P()))
        {
            // check properites
            tps = fallbackThermoPropertiesSubstance(tps, subst);
            tps = ConMolVol(subst).thermoProperties(T, P, tps);
        }

        return tps;
    }

    auto thermoPropertiesReaction(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
    {
        // if the thermoPropertiesReaction function is called using a reaction equation
        if (!database.containsReaction(symbol) && (symbol.find("=") != std::string::npos))
        {
            Reaction reaction;
            reaction.fromEquation(symbol);
            return thermoPropertiesReaction(T, P, reaction);
            //database.addReaction(reaction);
        }
        return thermoPropertiesReaction(T, P, database.getReaction(symbol));
    }

    auto thermoPropertiesReaction(double T, double &P, const Reaction &reaction) const -> ThermoPropertiesReaction
    {
        ThermoPropertiesReaction tpr;
        WorkPreferences pref = getThermoPreferencesReaction(reaction);

        if (!pref.isReacFromReactants)
        {
            switch (pref.method_genEOS)
            {
            case MethodGenEoS_Thrift::type::CTPM_CON:
            {
                tpr = reaction.thermoReferenceProperties();
                break;
            }
            case MethodGenEoS_Thrift::type::CTPM_REA:
            {
                pref.method_T = MethodCorrT_Thrift::type::CTM_LGK;
                break;
            }
            }

            if (pref.method_genEOS != MethodGenEoS_Thrift::type::CTPM_CON)
            switch (pref.method_T)
            {
            case MethodCorrT_Thrift::type::CTM_LGX:
            case MethodCorrT_Thrift::type::CTM_LGK:
            case MethodCorrT_Thrift::type::CTM_EK0:
            case MethodCorrT_Thrift::type::CTM_EK1:
            case MethodCorrT_Thrift::type::CTM_EK3:
            case MethodCorrT_Thrift::type::CTM_EK2:
            {
                tpr = Reaction_LogK_fT(pref.workReaction).thermoProperties(T, P, pref.method_T);
                break;
            }
            case MethodCorrT_Thrift::type::CTM_DMD: // Dolejs-Maning 2010 density model
            {
                tpr = ReactionDolejsManning10(pref.workReaction).thermoProperties(T, P, properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                break;
            }
            case MethodCorrT_Thrift::type::CTM_DKR: // Marshall-Franck density model
            {
                tpr = ReactionFrantzMarshall(pref.workReaction).thermoProperties(T, P, properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1));
                break;
            }
            case MethodCorrT_Thrift::type::CTM_MRB: // Calling modified Ryzhenko-Bryzgalin model TW KD 08.2007
            {
                return ReactionRyzhenkoBryzgalin(pref.workReaction).thermoProperties(T, P, properties_solvent_fn(T, P, P, preferences.solvent_symbol, -1)); // NOT TESTED!!!
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

            if (pref.method_genEOS != MethodGenEoS_Thrift::type::CTPM_CON)
            switch (pref.method_P)
            {

            case MethodCorrP_Thrift::type::CPM_VKE:
            case MethodCorrP_Thrift::type::CPM_VBE:

            {
                tpr = Reaction_Vol_fT(pref.workReaction).thermoProperties(T, P, tpr);
                break;
            }
            case MethodCorrP_Thrift::type::CPM_NUL:
            case MethodCorrP_Thrift::type::CPM_CON:
            {
                auto Pref = pref.workReaction.referenceP() / 1e5;
                auto Vref = pref.workReaction.thermoReferenceProperties().reaction_volume;

                const auto tprIn = tpr;
                tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
                    auto a = lift(pass, tprIn);
                    const real Pbar = pass.P / 1e5; // the pressure of the pass (with its derivative)
                    const real VP = a.reaction_volume * (Pbar - Pref);
                    a.reaction_gibbs_energy += VP;
                    a.reaction_enthalpy += VP;
                    a.log_equilibrium_constant -= pass(Vref) * (Pbar - Pref) / (R_CONSTANT * pass.T) / lg_to_ln;
                    a.reaction_entropy = (a.reaction_enthalpy - a.reaction_gibbs_energy) / pass.T;
                    a.reaction_internal_energy = a.reaction_enthalpy - Pbar * a.reaction_volume;
                    a.reaction_helmholtz_energy = a.reaction_internal_energy - pass.T * a.reaction_entropy;
                    return a;
                });

                // the properties that were not changed keep their error and status
                auto keep = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
                keep(tpr.ln_equilibrium_constant, tprIn.ln_equilibrium_constant);
                keep(tpr.reaction_volume, tprIn.reaction_volume);
                keep(tpr.reaction_heat_capacity_cp, tprIn.reaction_heat_capacity_cp);
                keep(tpr.reaction_heat_capacity_cv, tprIn.reaction_heat_capacity_cv);
                tpr.reaction_gibbs_energy.propagateFrom(tprIn.reaction_gibbs_energy, tprIn.reaction_volume);
                tpr.reaction_enthalpy.propagateFrom(tprIn.reaction_enthalpy, tprIn.reaction_volume);
                tpr.log_equilibrium_constant.propagateFrom(tprIn.log_equilibrium_constant, Vref);
                tpr.reaction_entropy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_gibbs_energy);
                tpr.reaction_internal_energy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_volume);
                tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_internal_energy, tpr.reaction_entropy);

                // first-order propagation of the errors (independent inputs)
                {
                    const double dP = P / 1e5 - Pref;
                    const double RT = R_CONSTANT * T;
                    tpr.reaction_gibbs_energy.setError({{1.0, tprIn.reaction_gibbs_energy.err}, {dP, tprIn.reaction_volume.err}});
                    tpr.reaction_enthalpy.setError({{1.0, tprIn.reaction_enthalpy.err}, {dP, tprIn.reaction_volume.err}});
                    tpr.log_equilibrium_constant.setError({{1.0, tprIn.log_equilibrium_constant.err}, {dP/(RT*lg_to_ln), Vref.err}});
                    tpr.reaction_entropy.setError({{1.0/T, tpr.reaction_enthalpy.err}, {1.0/T, tpr.reaction_gibbs_energy.err}});
                    tpr.reaction_internal_energy.setError({{1.0, tpr.reaction_enthalpy.err}, {P/1e5, tpr.reaction_volume.err}});
                    tpr.reaction_helmholtz_energy.setError({{1.0, tpr.reaction_internal_energy.err}, {T, tpr.reaction_entropy.err}});
                }
                //dU/dT=C (v)

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
        }
        else
            tpr = thermoPropertiesReactionFromReactants(T, P, reaction);

        return tpr; 
    }

    auto thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
    {
        return thermoPropertiesReactionFromReactants(T, P, database.getReaction(symbol));
    }

    auto thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
    {
        ThermoPropertiesReaction tpr;
        tpr.reaction_heat_capacity_cp = 0.0;
        tpr.reaction_gibbs_energy = 0.0;
        tpr.reaction_enthalpy = 0.0;
        tpr.reaction_entropy = 0.0;
        tpr.reaction_volume = 0.0;
        tpr.ln_equilibrium_constant = 0.0;
        tpr.log_equilibrium_constant = 0.0;
        tpr.reaction_heat_capacity_cv = 0.0;
        tpr.reaction_internal_energy = 0.0;
        tpr.reaction_helmholtz_energy = 0.0;

        preferences.apply_pressure_correction_to_gas_props = true;

        // the properties of the reactants
        std::vector<std::pair<ThermoPropertiesSubstance, double>> components;
        std::vector<std::string> symbols;

        for (auto &reactant : reaction.reactants())
        {
            auto coeff      = reactant.second;
            auto substance  = reactant.first;
            auto s = database.getSubstance(substance);

            // check if substance is correctly defined
            if (!s.methodGenEOS() && !s.method_P() && !s.method_T() && s.reactionSymbol() == reaction.symbol())
            {
                errorMethodNotFound("reaction", reaction.symbol(), __LINE__, __FILE__);
            }

            components.push_back({thermo_properties_substance_fn(T, P,P, substance) /*thermoPropertiesSubstance(T, P, substance);*/, coeff});
            symbols.push_back(substance);
        }

        tpr = ReactionFromReactantsProperties(reaction).thermoProperties(T, P, components, symbols);

        preferences.apply_pressure_correction_to_gas_props = false;
        return tpr;
    }

    //=================================================================================================
    // Propagation of the errors of the reference properties and of the coefficients of the models
    //=================================================================================================

    /// A copy of the engine used to calculate with perturbed parameters (without memoization and error propagation)
    mutable std::shared_ptr<ThermoEngine> perturbed_engine;

    auto perturbedEngine() const -> ThermoEngine&
    {
        if (!perturbed_engine)
            perturbed_engine.reset(new ThermoEngine(database));
        auto& impl = *perturbed_engine->pimpl;
        const bool reset = impl.preferences.enable_memoize;
        impl.preferences = preferences;
        impl.preferences.enable_memoize = false;
        impl.preferences.propagate_parameter_errors = false;
        impl.preferences.round_to_uncertainty = false;
        impl.conventions = conventions;
        if (reset) impl.set_fn(); // without memoization
        return *perturbed_engine;
    }

    static auto fields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.gibbs_energy, &p.helmholtz_energy, &p.internal_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp, &p.heat_capacity_cv};
    }
    static auto fields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.reaction_gibbs_energy, &p.reaction_helmholtz_energy, &p.reaction_internal_energy, &p.reaction_enthalpy, &p.reaction_entropy,
                &p.reaction_volume, &p.reaction_heat_capacity_cp, &p.reaction_heat_capacity_cv, &p.ln_equilibrium_constant, &p.log_equilibrium_constant};
    }
    static auto referenceFields(ThermoPropertiesSubstance& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.gibbs_energy, &p.enthalpy, &p.entropy, &p.volume, &p.heat_capacity_cp};
    }
    static auto referenceFields(ThermoPropertiesReaction& p) -> std::vector<Reaktoro_::ThermoProperty*>
    {
        return {&p.log_equilibrium_constant, &p.reaction_gibbs_energy, &p.reaction_enthalpy, &p.reaction_entropy, &p.reaction_volume, &p.reaction_heat_capacity_cp};
    }

    /// The records (substances and reactions of the database) a calculation depends on
    struct Records
    {
        std::set<std::string> substances, reactions;
    };

    auto collectSubstance(Records& r, const std::string& symbol) const -> void
    {
        if (r.substances.count(symbol) || !database.containsSubstance(symbol)) return;
        r.substances.insert(symbol);
        const auto reaction = database.getSubstance(symbol).reactionSymbol();
        if (!reaction.empty()) collectReaction(r, reaction);
    }

    auto collectReaction(Records& r, const std::string& symbol) const -> void
    {
        if (r.reactions.count(symbol) || !database.containsReaction(symbol)) return;
        r.reactions.insert(symbol);
        for (const auto& reactant : database.getReaction(symbol).reactants())
            collectSubstance(r, reactant.first);
    }

    /// One parameter with an uncertainty
    struct Parameter
    {
        int record;           // 0: substance of the database, 1: reaction of the database, 2: given substance, 3: given reaction
        std::string symbol;
        int reference;        // index of the reference property, or -1 for a coefficient
        std::string key;      // the coefficient (see UncertainCoefficient)
        double sigma;
    };

    /// Add the effect of the uncertain parameters to the errors of the properties calculated by eval: the standard error
    /// of a property is the quadrature sum of (y(p+s) - y(p-s))/2 over the parameters p with an error s (first-order
    /// propagation, independent parameters; parameters shared by several records are perturbed together)
    template<class Props, class Eval>
    auto addParameterErrors(Props& result, const Records& records, const Substance* givenS, const Reaction* givenR, Eval eval) const -> void
    {
        using Sub = Substance;
        using Rea = Reaction;
        std::vector<Parameter> parameters;

        auto listSubstance = [&](Sub s, int kind, const std::string& symbol) {
            auto ref = s.thermoReferenceProperties();
            auto refs = referenceFields(ref);
            for (size_t i = 0; i < refs.size(); ++i)
                if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
            auto p = s.thermoParameters();
            for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
        };
        auto listReaction = [&](Rea r, int kind, const std::string& symbol) {
            auto ref = r.thermoReferenceProperties();
            auto refs = referenceFields(ref);
            for (size_t i = 0; i < refs.size(); ++i)
                if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
            auto p = r.thermoParameters();
            for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
        };
        for (const auto& symbol : records.substances) listSubstance(database.getSubstance(symbol), 0, symbol);
        for (const auto& symbol : records.reactions) listReaction(database.getReaction(symbol), 1, symbol);
        if (givenS) listSubstance(*givenS, 2, givenS->symbol());
        if (givenR) listReaction(*givenR, 3, givenR->symbol());
        if (parameters.empty()) return;

        auto& engine = perturbedEngine();
        auto& db = engine.pimpl->database;

        auto perturb = [&](const Parameter& q, double delta) -> Props {
            // the value of the parameter is changed in a copy of its record
            Substance tmpS; Reaction tmpR;
            const bool isSub = (q.record == 0 || q.record == 2);
            if (q.record == 0) tmpS = db.getSubstance(q.symbol);
            if (q.record == 1) tmpR = db.getReaction(q.symbol);
            if (q.record == 2) tmpS = *givenS;
            if (q.record == 3) tmpR = *givenR;
            const Substance origS = tmpS; const Reaction origR = tmpR;

            auto apply = [&](auto& record) {
                if (q.reference >= 0)
                {
                    auto ref = record.thermoReferenceProperties();
                    referenceFields(ref)[q.reference]->val += delta;
                    record.setThermoReferenceProperties(ref);
                }
                else
                {
                    auto par = record.thermoParameters();
                    for (const auto& c : uncertainCoefficients(par))
                        if (c.key == q.key) { *c.value += delta; break; }
                    record.setThermoParameters(par);
                }
            };
            if (isSub) apply(tmpS); else apply(tmpR);

            Props out;
            try
            {
                if (q.record == 0) db.setSubstance(tmpS);
                if (q.record == 1) db.setReaction(tmpR);
                out = eval(engine, q.record == 2 ? &tmpS : givenS, q.record == 3 ? &tmpR : givenR);
            }
            catch (...)
            {
                if (q.record == 0) db.setSubstance(origS);
                if (q.record == 1) db.setReaction(origR);
                throw;
            }
            if (q.record == 0) db.setSubstance(origS);
            if (q.record == 1) db.setReaction(origR);
            return out;
        };

        std::vector<double> sum2(fields(result).size(), 0.0);
        for (const auto& q : parameters)
        {
            try
            {
                auto plus  = perturb(q, +q.sigma);
                auto minus = perturb(q, -q.sigma);
                auto fp = fields(plus), fm = fields(minus);
                for (size_t k = 0; k < fp.size(); ++k)
                {
                    if (fp[k]->sta.first == Reaktoro_::Status::notdefined || fm[k]->sta.first == Reaktoro_::Status::notdefined) continue;
                    const double d = 0.5 * (fp[k]->val - fm[k]->val);
                    if (std::isfinite(d)) sum2[k] += d * d;
                }
            }
            catch (...) {} // no contribution of a parameter for which the calculation fails
        }
        auto fr = fields(result);
        for (size_t k = 0; k < fr.size(); ++k) fr[k]->err = std::sqrt(sum2[k]);
    }

    /// Round the values of the properties to the decimals of their uncertainties (NEA TDB rules) if requested
    template<class Props>
    auto roundResults(Props& result) const -> void
    {
        if (!preferences.round_to_uncertainty) return;
        for (auto* f : fields(result))
            if (f->sta.first != Reaktoro_::Status::notdefined)
            {
                double value = f->val, error = f->err;
                rounding::toUncertainty(value, error, preferences.uncertainty_significant_digits);
                f->val = value;
                f->err = error;
            }
    }

    auto recordsOfSubstance(const std::string& symbol) const -> Records { Records r; collectSubstance(r, symbol); return r; }
    auto recordsOfReaction(const std::string& symbol) const -> Records { Records r; collectReaction(r, symbol); return r; }
    auto recordsOfReactants(const Reaction& reaction) const -> Records
    {
        Records r;
        for (const auto& reactant : reaction.reactants()) collectSubstance(r, reactant.first);
        return r;
    }
};

ThermoEngine::ThermoEngine(const std::string filename)
    : pimpl()
{
    Database db(filename);
    pimpl.reset(new Impl(db));
}

ThermoEngine::ThermoEngine(const Database &database)
    : pimpl(new Impl(database))
{
}

ThermoEngine::ThermoEngine(const ThermoEngine &other)
    : pimpl(new Impl(*other.pimpl))
{
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance
{
    const double P0 = P;
    auto tps = pimpl->thermo_properties_substance_fn(T, P, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tps, pimpl->recordsOfSubstance(substance), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesSubstance(T, p, substance); });
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, std::string solvent, int state) const -> ElectroPropertiesSolvent
{
    return pimpl->electro_properties_solvent_fn(T, P, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, std::string solvent, int state) const -> PropertiesSolvent
{
    return pimpl->properties_solvent_fn(T, P, P, solvent, state);
}

auto ThermoEngine::thermoPropertiesSubstance(double T, double &P, const Substance& substance) const -> ThermoPropertiesSubstance
{
    const double P0 = P;
    auto tps = pimpl->thermoPropertiesSubstance(T, P, substance);
    if (pimpl->preferences.propagate_parameter_errors)
    {
        ThermoEngine::Impl::Records records;
        if (!substance.reactionSymbol().empty()) pimpl->collectReaction(records, substance.reactionSymbol());
        pimpl->addParameterErrors(tps, records, &substance, nullptr,
            [&](ThermoEngine& e, const Substance* s, const Reaction*) { double p = P0; return e.thermoPropertiesSubstance(T, p, *s); });
    }
    pimpl->roundResults(tps);
    return tps;
}

auto ThermoEngine::electroPropertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> ElectroPropertiesSolvent
{
    return pimpl->electroPropertiesSolvent(T, P, solvent, state);
}

auto ThermoEngine::propertiesSolvent(double T, double &P, const Substance&  solvent, int state) const -> PropertiesSolvent
{
    return pimpl->propertiesSolvent(T, P, solvent, state);
}

// Reaction
auto ThermoEngine::thermoPropertiesReaction(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction
{
    const double P0 = P;
    auto tpr = pimpl->thermo_properties_reaction_fn(T, P, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReaction(reaction), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesReaction(T, p, reaction); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, std::string symbol) const -> ThermoPropertiesReaction
{
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReactionFromReactants(T, P, symbol);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReaction(symbol), nullptr, nullptr,
            [&](ThermoEngine& e, const Substance*, const Reaction*) { double p = P0; return e.thermoPropertiesReactionFromReactants(T, p, symbol); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReaction(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReaction(T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReactants(reaction), nullptr, &reaction,
            [&](ThermoEngine& e, const Substance*, const Reaction* r) { double p = P0; return e.thermoPropertiesReaction(T, p, *r); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction
{
    const double P0 = P;
    auto tpr = pimpl->thermoPropertiesReactionFromReactants(T, P, reaction);
    if (pimpl->preferences.propagate_parameter_errors)
        pimpl->addParameterErrors(tpr, pimpl->recordsOfReactants(reaction), nullptr, &reaction,
            [&](ThermoEngine& e, const Substance*, const Reaction* r) { double p = P0; return e.thermoPropertiesReactionFromReactants(T, p, *r); });
    pimpl->roundResults(tpr);
    return tpr;
}

auto ThermoEngine::setSolventSymbol(const std::string solvent_symbol) -> void
{
    pimpl->preferences.solvent_symbol = solvent_symbol;
}

auto ThermoEngine::solventSymbol() -> std::string&
{
    return pimpl->preferences.solvent_symbol;
}

EnginePreferences& ThermoEngine::preferences() {
    return pimpl->preferences;
}

const EnginePreferences& ThermoEngine::preferences() const {
    return pimpl->preferences;
}

auto ThermoEngine::database() const -> const Database &
{
    return pimpl->database;
}

auto ThermoEngine::appendData(std::string filename) -> void
{
    pimpl->database.appendData(filename);
    pimpl->perturbed_engine.reset();
}

auto ThermoEngine::appendData(std::vector<std::string> jsonRecords, std::string _label = "unknown label") -> void
{
    pimpl->database.appendData(jsonRecords, _label);
    pimpl->perturbed_engine.reset();
}

auto ThermoEngine::parseSubstanceFormula(std::string formula) const -> std::map<Element, double>
{
    return pimpl->database.parseSubstanceFormula(formula);
}

EngineConventions& ThermoEngine::conventions() {
    return pimpl->conventions;
}

const EngineConventions& ThermoEngine::conventions() const {
    return pimpl->conventions;
}

} // namespace ThermoFun
