#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "ThermoProperties.h"
#include "Database.h"
#include "Interfaces/Output.h"
#include "Thermo.h"

namespace ThermoFun {

struct Interface::Impl
{
    /// The thermo instance
    Thermo                                              thermo;

    OutputSettings                                      outSettings;

    std::vector<std::vector<double>>                    tpPairs;

    std::map<int, std::string>                          propNames;

    std::map<std::string, std::string>                  propUnits  = defaultPropertyUnits;

    std::map<std::string, int>                          propDigits = defaultPropertyDigits;

    std::map<int, std::string>                          solventPropNames;

    std::map<std::string, std::string>                  solventPropUnits  = defaultSolventPropertyUnits;

    std::map<std::string, int>                          solventPropDigits = defaultSolventPropertyDigits;

    std::vector<string>                                 substSymbols;

    std::vector<std::vector<Reaktoro_::ThermoScalar>>   substResults;

    std::vector<Reaktoro_::ThermoScalar>                solventProp;

    std::string                                         solventSymbol;

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

    addTPpair(T,P);

    addProperty(propName);

    calculateResultsSubst();

    return Output (*this);
}

auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTPpair(T, P);

    calculateResultsSubst();

    return Output (*this);
}

auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTPpairs(Tmin, Tmax, Tstep, Pmin, Pmax, Pstep);

    calculateResultsSubst();

    return Output (*this);
}
auto Interface::calculateProperties(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tpPairs) -> Output
{
    addSubstances(substanceSymbols);

    addProperties(thermoProperties);

    addTPpairs(tpPairs);

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

    addTPpair(T,P);

    addProperty(propName);

    calculateResultsReac();

    return Output (*this);
}

auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTPpair(T, P);

    calculateResultsReac();

    return Output (*this);
}

auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTPpairs(Tmin, Tmax, Tstep, Pmin, Pmax, Pstep);

    calculateResultsReac();

    return Output (*this);
}
auto Interface::calcPropReactions(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tpPairs) -> Output
{
    addReactions(reactionSymbols);

    addProperties(thermoProperties);

    addTPpairs(tpPairs);

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

auto Interface::selectPropertiesSolvent ( PropertiesSolvent ps, ElectroPropertiesSolvent eps ) -> std::vector<Reaktoro_::ThermoScalar>
{
    std::vector<Reaktoro_::ThermoScalar> resultsSolvent;
    std::map<int, std::string>::iterator it;

    for(it = pimpl->solventPropNames.begin(); it != pimpl->solventPropNames.end(); it++)
    {
        if (it->second == "density")        {resultsSolvent.push_back(ps.density);}
        if (it->second == "densityT")       {resultsSolvent.push_back(ps.densityT);}
        if (it->second == "densityP")       {resultsSolvent.push_back(ps.densityP);}
        if (it->second == "densityTT")      {resultsSolvent.push_back(ps.densityTT);}
        if (it->second == "densityTP")      {resultsSolvent.push_back(ps.densityTP);}
        if (it->second == "densityPP")      {resultsSolvent.push_back(ps.densityPP);}
        if (it->second == "alpha")          {resultsSolvent.push_back(ps.Alpha);}
        if (it->second == "beta")           {resultsSolvent.push_back(ps.Beta);}
        if (it->second == "alphaT")         {resultsSolvent.push_back(ps.dAldT);}
        if (it->second == "gibbsIdealGas")  {resultsSolvent.push_back(ps.gibbsIdealGas);}
        if (it->second == "entropyIdealGas"){resultsSolvent.push_back(ps.entropyIdealGas);}
        if (it->second == "cpIdealGas")     {resultsSolvent.push_back(ps.cpIdealGas);}

        if (it->second == "epsilon")        {resultsSolvent.push_back(eps.epsilon);}
        if (it->second == "epsilonT")       {resultsSolvent.push_back(eps.epsilonT);}
        if (it->second == "epsilonP")       {resultsSolvent.push_back(eps.epsilonP);}
        if (it->second == "epsilonTT")      {resultsSolvent.push_back(eps.epsilonTT);}
        if (it->second == "epsilonTP")      {resultsSolvent.push_back(eps.epsilonTP);}
        if (it->second == "epsilonPP")      {resultsSolvent.push_back(eps.epsilonPP);}
        if (it->second == "bornZ")          {resultsSolvent.push_back(eps.bornZ);}
        if (it->second == "bornY")          {resultsSolvent.push_back(eps.bornY);}
        if (it->second == "bornQ")          {resultsSolvent.push_back(eps.bornQ);}
        if (it->second == "bornN")          {resultsSolvent.push_back(eps.bornN);}
        if (it->second == "bornU")          {resultsSolvent.push_back(eps.bornU);}
        if (it->second == "bornX")          {resultsSolvent.push_back(eps.bornX);}
    }
    return resultsSolvent;
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

auto Interface::calculateSolventProp( int j ) -> void
{
    if (pimpl->outSettings.outSolventProp)
    {
        auto solProp = selectPropertiesSolvent(pimpl->thermo.propertiesSolvent(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->thermo.solventSymbol()),
                                               pimpl->thermo.electroPropertiesSolvent(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->thermo.solventSymbol()));
        pimpl->solventProp.insert(pimpl->solventProp.end(), solProp.begin(), solProp.end());
    }
}

auto Interface::calculateResultsSubst( ) -> void
{
    pimpl->substResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->substResults.resize(pimpl->substSymbols.size() * pimpl->tpPairs.size());
    pimpl->solventProp.resize(2*pimpl->tpPairs.size());
    pimpl->solventSymbol = pimpl->thermo.solventSymbol(); pimpl->solventProp.clear();
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->substSymbols.size(); i++)
        {
            pimpl->substResults[(tp*i)+(j)] = selectResultsSubst(pimpl->thermo.thermoPropertiesSubstance(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->substSymbols[i]));
        }
        calculateSolventProp(j);
    }
}

auto Interface::calculateResultsReac( ) -> void
{
    pimpl->reacResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->reacResults.resize(pimpl->reacSymbols.size() * pimpl->tpPairs.size());
    pimpl->solventProp.resize(2*pimpl->tpPairs.size());
    pimpl->solventSymbol = pimpl->thermo.solventSymbol(); pimpl->solventProp.clear();
    auto fromSubst = pimpl->outSettings.calcReactFromSubst;
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->reacSymbols.size(); i++)
        {
            if (fromSubst)
                pimpl->reacResults[(tp*i)+(j)] = selectResultsReac(pimpl->thermo.thermoPropertiesReactionFromReactants(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->reacSymbols[i]));
            else
                pimpl->reacResults[(tp*i)+(j)] = selectResultsReac(pimpl->thermo.thermoPropertiesReaction(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->reacSymbols[i]));
        }
        calculateSolventProp(j);
    }
}

auto Interface::selectResultsSubst_vTpSym(std::vector<std::vector<ThermoPropertiesSubstance>> vTps ) -> void
{
    pimpl->substResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->substResults.resize(pimpl->substSymbols.size() * pimpl->tpPairs.size());
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->substSymbols.size(); i++)
        {
            pimpl->substResults[(tp*i)+(j)] = selectResultsSubst(vTps[j][i]);
        }
    }
}

auto Interface::selectResultsReac_vTpSym(std::vector<std::vector<ThermoPropertiesReaction>> vTpr ) -> void
{
    pimpl->reacResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->reacResults.resize(pimpl->reacSymbols.size() * pimpl->tpPairs.size());
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
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
//    if (std::find(pimpl->reacSymbols.begin(), pimpl->reacSymbols.end(), reacSymbol) == pimpl->reacSymbols.end())
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
    std::map<std::string, const std::string>::const_iterator it;
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
    std::map<std::string, const std::string>::const_iterator it;

    for (unsigned i = 0; i<propNames.size(); i++)
    {
        addProperty(propNames[i]);
    }
}

auto Interface::addSolventProperty (const std::string &sovlentPropName) -> void
{
    std::map<std::string, const std::string>::const_iterator it;
    it = defaultSolventPropertyNames.find(sovlentPropName);
    if ( it != defaultSolventPropertyNames.end())
    {
        if (!pimpl->solventPropNames.empty())
        {
            pimpl->solventPropNames.insert(std::pair<int,std::string>(pimpl->solventPropNames.rbegin()->first+1,sovlentPropName));
        } else
            pimpl->solventPropNames.insert(std::pair<int,std::string>(1,sovlentPropName));
    }

    // write exception prop not existing
}

auto Interface::addSolventProperties (const std::vector<string> &solventPropNames) -> void
{
    std::map<std::string, const std::string>::const_iterator it;

    for (unsigned i = 0; i<solventPropNames.size(); i++)
    {
        addSolventProperty(solventPropNames[i]);
    }
}

auto Interface::addTPpair (const double &T, const double &P) -> void
{
    std::vector<double> one_pair = {T, P};
    pimpl->tpPairs.push_back(one_pair);
}

auto Interface::addTPpairs (const double &Tmin, const double &Tmax, const double &Tstep,
                  const double &Pmin, const double &Pmax, const double &Pstep) -> void
{
    double t = Tmin;
    double p = Pmin;
    do
    {
       do
        {
            addTPpair(t,p);
            p = p + Pstep;
        } while (p <= Pmax);
       t = t + Tstep;
    } while (t <= Tmax);
}

auto Interface::addTPpairs (const std::vector<std::vector<double>> &tpPairs) -> void
{
    for (unsigned i=0; i <tpPairs.size(); i++)
    {
        addTPpair(tpPairs[i][0], tpPairs[i][1]);
    }
    pimpl->tpPairs = tpPairs;
}

auto Interface::addDigits (const std::map<std::string, int> &propDigits)-> void
{
    pimpl->propDigits = propDigits;
}

auto Interface::addSolventPropDigits (const std::map<std::string, int> &solventPropDigits)-> void
{
    pimpl->solventPropDigits = solventPropDigits;
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
auto Interface::clearSolventProperties () -> void
{
    pimpl->solventPropNames.clear();
}
auto Interface::clearTPpairs () -> void
{
    pimpl->tpPairs.clear();
}

auto Interface::clearReactions () -> void
{
    pimpl->reacSymbols.clear();
}


// set functions
auto Interface::setOutputSettings(const OutputSettings &value) -> void
{
    pimpl->outSettings = value;
}

auto Interface::setSolventSymbol(const std::string solvent_symbol) -> void
{
    pimpl->thermo.setSolventSymbol(solvent_symbol);
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

auto Interface::TPpairs() -> const std::vector<std::vector<double> >
{
    return pimpl->tpPairs;
}

auto Interface::propNames() -> const map<int, std::string>
{
    return pimpl->propNames;
}

auto Interface::solventPropNames() -> const map<int, std::string>
{
    return pimpl->solventPropNames;
}

auto Interface::propUnits() -> const std::map<std::string, std::string>
{
    return pimpl->propUnits;
}

auto Interface::propDigits() -> const std::map<std::string, int>
{
    return pimpl->propDigits;
}

auto Interface::solventPropUnits() -> const std::map<std::string, std::string>
{
    return pimpl->solventPropUnits;
}

auto Interface::solventPropDigits() -> const std::map<std::string, int>
{
    return pimpl->solventPropDigits;
}

auto Interface::resultsSubst() -> const std::vector<std::vector<Reaktoro_::ThermoScalar> >
{
    return pimpl->substResults;
}

auto Interface::solventProp() -> const std::vector<Reaktoro_::ThermoScalar>
{
    return pimpl->solventProp;
}

auto Interface::solventSymbol() -> const std::string
{
    return pimpl->solventSymbol;
}

auto Interface::resultsReac() -> const std::vector<std::vector<Reaktoro_::ThermoScalar> >
{
    return pimpl->reacResults;
}

}

