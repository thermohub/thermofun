#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace ThermoFun {

struct Interface::Impl
{
    /// The thermo instance
    Thermo                                              thermo;

    OutputSettings                                      outSettings;

    std::vector<std::vector<double>>                    tp_pairs;

    std::map<int, std::string>                          propNames;

    std::map<const std::string, std::string>            propUnits  = defaultPropertyUnits;

    std::map<const std::string, int>                    propDigits = defaultPropertyDigits;

    std::vector<string>                                 substSymbols;

    std::vector<std::vector<Reaktoro_::ThermoScalar>>   substResults;

    std::vector<string>                                 reacSymbols;

    std::vector<std::vector<Reaktoro_::ThermoScalar>>   reacResults;

    unsigned int                                        count = 0;

    Impl(const Database& database)
    : thermo(Thermo(database))
    {}


public:
};

Interface::Interface(const Database& database)
    : pimpl(new Impl(database))
{}

auto Interface::calculateProperties() -> Output
{
    calculateResultsSubst();

    return Output (*this);
}

auto Interface::calculateProperties(const std::string substSymbol, const double T, const double P, const std::string propName) -> Output
{
    addSubstance(substSymbol);

    addTP_pair(T,P);

    addProperty(propName);

    calculateResultsSubst();

    return Output (*this);
}

auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTP_pair(T, P);

    calculateResultsSubst();

    return Output (*this);
}

auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTP_pairs(Tmin, Tmax, Tstep, Pmin, Pmax, Pstep);

    calculateResultsSubst();

    return Output (*this);
}
auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tp_pairs) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTP_pairs(tp_pairs);

    calculateResultsSubst();

    return Output (*this);
}

// Reactions

auto Interface::calcPropReactions() -> Output
{
    calculateResultsReac();

    return Output (*this);
}

auto Interface::calcPropReactions(const std::string reacSymbol, const double T, const double P, const std::string propName) -> Output
{
    addReaction(reacSymbol);

    addTP_pair(T,P);

    addProperty(propName);

    calculateResultsReac();

    return Output (*this);
}

auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTP_pair(T, P);

    calculateResultsReac();

    return Output (*this);
}

auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTP_pairs(Tmin, Tmax, Tstep, Pmin, Pmax, Pstep);

    calculateResultsReac();

    return Output (*this);
}
auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tp_pairs) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTP_pairs(tp_pairs);

    calculateResultsReac();

    return Output (*this);
}

auto Interface::selectResultsSubst ( ThermoPropertiesSubstance tps ) -> std::vector<Reaktoro_::ThermoScalar>
{
    std::vector<Reaktoro_::ThermoScalar> resultsSubst;
    std::map<int, std::string>::iterator it;

    for(it = pimpl->propNames.begin(); it != pimpl->propNames.end(); it++)
    {
        if (it->second == "gibbs_energy")       {resultsSubst.push_back(tps.gibbs_energy);}
        if (it->second == "helmholtz_energy")   {resultsSubst.push_back(tps.helmholtz_energy);}
        if (it->second == "internal_energy")    {resultsSubst.push_back(tps.internal_energy);}
        if (it->second == "enthalpy")           {resultsSubst.push_back(tps.enthalpy);}
        if (it->second == "entropy")            {resultsSubst.push_back(tps.entropy);}
        if (it->second == "volume")             {resultsSubst.push_back(tps.volume);}
        if (it->second == "heat_capacity_cp")   {resultsSubst.push_back(tps.heat_capacity_cp);}
        if (it->second == "heat_capacity_cv")   {resultsSubst.push_back(tps.heat_capacity_cv);}
    }
    return resultsSubst;
}

auto Interface::selectResultsReac ( ThermoPropertiesReaction tpr ) -> std::vector<Reaktoro_::ThermoScalar>
{
    std::vector<Reaktoro_::ThermoScalar> resultsReac;
    std::map<int, std::string>::iterator it;

    for(it = pimpl->propNames.begin(); it != pimpl->propNames.end(); it++)
    {
        if (it->second == "reaction_gibbs_energy")       {resultsReac.push_back(tpr.reaction_gibbs_energy);}
        if (it->second == "reaction_helmholtz_energy")   {resultsReac.push_back(tpr.reaction_helmholtz_energy);}
        if (it->second == "reaction_internal_energy")    {resultsReac.push_back(tpr.reaction_internal_energy);}
        if (it->second == "reaction_enthalpy")           {resultsReac.push_back(tpr.reaction_enthalpy);}
        if (it->second == "reaction_entropy")            {resultsReac.push_back(tpr.reaction_entropy);}
        if (it->second == "reaction_volume")             {resultsReac.push_back(tpr.reaction_volume);}
        if (it->second == "reaction_heat_capacity_cp")   {resultsReac.push_back(tpr.reaction_heat_capacity_cp);}
        if (it->second == "reaction_heat_capacity_cv")   {resultsReac.push_back(tpr.reaction_heat_capacity_cv);}
        if (it->second == "logKr")                       {resultsReac.push_back(tpr.log_equilibrium_constant);}
        if (it->second == "lnK0")                        {resultsReac.push_back(tpr.ln_equilibrium_constant);}
    }
    return resultsReac;
}

auto Interface::calculateResultsSubst( ) -> void
{
    pimpl->substResults.empty(); unsigned tp = pimpl->tp_pairs.size();
    pimpl->substResults.resize(pimpl->substSymbols.size() * pimpl->tp_pairs.size());
    for (unsigned j=0; j<pimpl->tp_pairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->substSymbols.size(); i++)
        {
            pimpl->substResults[(tp*i)+(j)] = selectResultsSubst(pimpl->thermo.thermoPropertiesSubstance(pimpl->tp_pairs[j][0], pimpl->tp_pairs[j][1], pimpl->substSymbols[i]));
        }
    }
}

auto Interface::calculateResultsReac( ) -> void
{
    pimpl->reacResults.empty(); unsigned tp = pimpl->tp_pairs.size();
    pimpl->reacResults.resize(pimpl->reacSymbols.size() * pimpl->tp_pairs.size());
    for (unsigned j=0; j<pimpl->tp_pairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->reacSymbols.size(); i++)
        {
            pimpl->reacResults[(tp*i)+(j)] = selectResultsReac(pimpl->thermo.thermoPropertiesReaction(pimpl->tp_pairs[j][0], pimpl->tp_pairs[j][1], pimpl->reacSymbols[i]));
        }
    }
}

auto Interface::selectResultsSubst_vTpSym(std::vector<std::vector<ThermoPropertiesSubstance>> vTps ) -> void
{
    pimpl->substResults.empty(); unsigned tp = pimpl->tp_pairs.size();
    pimpl->substResults.resize(pimpl->substSymbols.size() * pimpl->tp_pairs.size());
    for (unsigned j=0; j<pimpl->tp_pairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->substSymbols.size(); i++)
        {
            pimpl->substResults[(tp*i)+(j)] = selectResultsSubst(vTps[j][i]);
        }
    }
}

auto Interface::selectResultsReac_vTpSym(std::vector<std::vector<ThermoPropertiesReaction>> vTpr ) -> void
{
    pimpl->reacResults.empty(); unsigned tp = pimpl->tp_pairs.size();
    pimpl->reacResults.resize(pimpl->reacSymbols.size() * pimpl->tp_pairs.size());
    for (unsigned j=0; j<pimpl->tp_pairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->reacSymbols.size(); i++)
        {
            pimpl->reacResults[(tp*i)+(j)] = selectResultsReac(vTpr[j][i]);
        }
    }
}

// Add functions
auto Interface::addSubstance (const std::string &substSymbol) -> void
{
    if (std::find(pimpl->substSymbols.begin(), pimpl->substSymbols.end(), substSymbol) == pimpl->substSymbols.end())
        pimpl->substSymbols.push_back(substSymbol);
    // add exception ??
}

auto Interface::addSubstances (const std::vector<string> &substSymbols) -> void
{
    for (unsigned i = 0; i < substSymbols.size(); i++)
    {
        addSubstance(substSymbols[i]);
    }
}

auto Interface::addReaction (const std::string &reacSymbol) -> void
{
    if (std::find(pimpl->reacSymbols.begin(), pimpl->reacSymbols.end(), reacSymbol) == pimpl->reacSymbols.end())
        pimpl->reacSymbols.push_back(reacSymbol);
    // add exception ??
}

auto Interface::addReactions (const std::vector<string> &reacSymbols) -> void
{
    for (unsigned i = 0; i < reacSymbols.size(); i++)
    {
        addReaction(reacSymbols[i]);
    }
}

auto Interface::addProperty (const std::string &propName) -> void
{
    std::map<const std::string, const std::string>::const_iterator it;
    it = defaultPropertyNames.find(propName);
    if ( it != defaultPropertyNames.end())
    {
        if (!pimpl->propNames.empty())
        {
            pimpl->propNames.insert(std::pair<int,std::string>(pimpl->propNames.rbegin()->first+1,propName));
        } else
            pimpl->propNames.insert(std::pair<int,std::string>(1,propName));
    }

    // write exception prop not existing
}

auto Interface::addProperties (const std::vector<string> &propNames) -> void
{
    std::map<const std::string, const std::string>::const_iterator it;

    for (unsigned i = 0; i<propNames.size(); i++)
    {
        addProperty(propNames[i]);
    }
}

auto Interface::addTP_pair (const double &T, const double &P) -> void
{
    std::vector<double> one_pair = {T, P};
    pimpl->tp_pairs.push_back(one_pair);
}

auto Interface::addTP_pairs (const double &Tmin, const double &Tmax, const double &Tstep,
                  const double &Pmin, const double &Pmax, const double &Pstep) -> void
{
    double t = Tmin;
    double p = Pmin;
    do
    {
       do
        {
            addTP_pair(t,p);
            p = p + Pstep;
        } while (p <= Pmax);
       t = t + Tstep;
    } while (t <= Tmax);
}

auto Interface::addTP_pairs (const std::vector<std::vector<double>> &TP_pairs) -> void
{
    for (unsigned i=0; i <TP_pairs.size(); i++)
    {
        addTP_pair(TP_pairs[i][0], TP_pairs[i][1]);
    }
    pimpl->tp_pairs = TP_pairs;
}

auto Interface::addDigits (const std::map<const std::string, int> &propDigits)-> void
{
    pimpl->propDigits = propDigits;
}

// clear functions
auto Interface::clearSubstances () -> void
{
    pimpl->substSymbols.clear();
}
auto Interface::clearProperties () -> void
{
    pimpl->propNames.clear();
}
auto Interface::clearTP_pairs () -> void
{
    pimpl->tp_pairs.clear();
}

// set functions
auto Interface::setOutputSettings(const OutputSettings &value) -> void
{
    pimpl->outSettings = value;
}

auto Interface::setSolventSymbolForAqSubst(const std::string solvent_symbol) -> void
{
    pimpl->thermo.setSolventSymbolForAllAqSubst(solvent_symbol);
}

// get functions
auto Interface::substanceSymbols() -> const std::vector<string>
{
    return pimpl->substSymbols;
}

auto Interface::reactionSymbols() -> const std::vector<string>
{
    return pimpl->reacSymbols;
}

auto Interface::outputSettings() -> const OutputSettings
{
    return pimpl->outSettings;
}

auto Interface::TP_pairs() -> const std::vector<std::vector<double> >
{
    return pimpl->tp_pairs;
}

auto Interface::propNames() -> const map<int, std::string>
{
    return pimpl->propNames;
}

auto Interface::propUnits() -> const std::map<const std::string, std::string>
{
    return pimpl->propUnits;
}

auto Interface::propDigits() -> const std::map<const std::string, int>
{
    return pimpl->propDigits;
}

auto Interface::resultsSubst() -> const std::vector<std::vector<Reaktoro_::ThermoScalar> >
{
    return pimpl->substResults;
}

auto Interface::resultsReac() -> const std::vector<std::vector<Reaktoro_::ThermoScalar> >
{
    return pimpl->reacResults;
}

}

