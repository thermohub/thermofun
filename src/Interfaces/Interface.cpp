#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "ThermoProperties.h"
#include "Database.h"
#include "Interfaces/Output.h"
#include "Thermo.h"
#include "Common/Units.hpp"

namespace ThermoFun {

struct Interface::Impl
{
    /// The thermo instance
    Thermo                                              thermo;

    OutputSettings                                      outSettings;

    std::vector<std::vector<double>>                    tpPairs;

    std::vector<std::string>                            propNames;

    std::vector<std::string>                            solventPropNames;

    std::map<std::string, std::string>                  propUnits  = defaultPropertyUnits;

    std::map<std::string, int>                          propDigits = defaultPropertyDigits;

    std::map<std::string, std::string>                  solventPropUnits  = defaultSolventPropertyUnits;

    std::map<std::string, int>                          solventPropDigits = defaultSolventPropertyDigits;

    std::vector<string>                                 substSymbols;

    std::vector<std::vector<Reaktoro_::ThermoScalar>>   substResults;

    std::vector<Reaktoro_::ThermoScalar>                solventProp;

    std::string                                         solventSymbol;

    std::vector<string>                                 reactSymbols;

    std::vector<std::vector<Reaktoro_::ThermoScalar>>   reactResults;

    unsigned int                                        count = 0;

    Impl(const Database& database)
    : thermo(Thermo(database))
    {}

    // clear functions
    auto clearSubstances () -> void
    {
        substSymbols.clear();
    }

    auto clearReactions () -> void
    {
        reactSymbols.clear();
    }

    auto clearProperties () -> void
    {
        propNames.clear();
    }

    auto clearSolventProperties () -> void
    {
        solventPropNames.clear();
    }

    auto clearTPpairs () -> void
    {
        tpPairs.clear();
    }

    auto clearAll() -> void
    {
        clearProperties();
        clearReactions();
        clearSubstances();
        clearTPpairs();
        clearSolventProperties();
    }

    // Add functions
    auto addSubstances(const std::vector<string> &symbols) -> void
    {
        substSymbols = symbols;
    }

    auto addReactions (const std::vector<string> &symbols) -> void
    {
        reactSymbols = symbols;
    }

    auto addProperties (const std::vector<string> &names) -> void
    {
        propNames = names;
    }

    auto addTPpair (const double &T, const double &P) -> void
    {
        std::vector<double> one_pair = {T, P};
        tpPairs.push_back(one_pair);
    }

    auto addTPpairs (const double &Tmin, const double &Tmax, const double &Tstep,
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

    auto addTPpairs (const std::vector<std::vector<double>> &tpPairs) -> void
    {
        for (unsigned i=0; i <tpPairs.size(); i++)
        {
            addTPpair(tpPairs[i][0], tpPairs[i][1]);
        }
    }

    // select functions
    auto selectResultsSubst ( ThermoPropertiesSubstance tps ) -> std::vector<Reaktoro_::ThermoScalar>
    {
        std::vector<Reaktoro_::ThermoScalar> resultsSubst;
        for( auto name : propNames)
        {
            if (name == "gibbs_energy")       {resultsSubst.push_back(tps.gibbs_energy);}
            if (name == "helmholtz_energy")   {resultsSubst.push_back(tps.helmholtz_energy);}
            if (name == "internal_energy")    {resultsSubst.push_back(tps.internal_energy);}
            if (name == "enthalpy")           {resultsSubst.push_back(tps.enthalpy);}
            if (name == "entropy")            {resultsSubst.push_back(tps.entropy);}
            if (name == "volume")             {resultsSubst.push_back(tps.volume);}
            if (name == "heat_capacity_cp")   {resultsSubst.push_back(tps.heat_capacity_cp);}
            if (name == "heat_capacity_cv")   {resultsSubst.push_back(tps.heat_capacity_cv);}
        }
        return resultsSubst;
    }

    auto selectPropertiesSolvent ( PropertiesSolvent ps, ElectroPropertiesSolvent eps ) -> std::vector<Reaktoro_::ThermoScalar>
    {
        std::vector<Reaktoro_::ThermoScalar> resultsSolvent;
        for( auto name : solventPropNames)
        {
            if (name == "density")        {resultsSolvent.push_back(ps.density);}
            if (name == "densityT")       {resultsSolvent.push_back(ps.densityT);}
            if (name == "densityP")       {resultsSolvent.push_back(ps.densityP);}
            if (name == "densityTT")      {resultsSolvent.push_back(ps.densityTT);}
            if (name == "densityTP")      {resultsSolvent.push_back(ps.densityTP);}
            if (name == "densityPP")      {resultsSolvent.push_back(ps.densityPP);}
            if (name == "alpha")          {resultsSolvent.push_back(ps.Alpha);}
            if (name == "beta")           {resultsSolvent.push_back(ps.Beta);}
            if (name == "alphaT")         {resultsSolvent.push_back(ps.dAldT);}

            if (name == "epsilon")        {resultsSolvent.push_back(eps.epsilon);}
            if (name == "epsilonT")       {resultsSolvent.push_back(eps.epsilonT);}
            if (name == "epsilonP")       {resultsSolvent.push_back(eps.epsilonP);}
            if (name == "epsilonTT")      {resultsSolvent.push_back(eps.epsilonTT);}
            if (name == "epsilonTP")      {resultsSolvent.push_back(eps.epsilonTP);}
            if (name == "epsilonPP")      {resultsSolvent.push_back(eps.epsilonPP);}
            if (name == "bornZ")          {resultsSolvent.push_back(eps.bornZ);}
            if (name == "bornY")          {resultsSolvent.push_back(eps.bornY);}
            if (name == "bornQ")          {resultsSolvent.push_back(eps.bornQ);}
            if (name == "bornN")          {resultsSolvent.push_back(eps.bornN);}
            if (name == "bornU")          {resultsSolvent.push_back(eps.bornU);}
            if (name == "bornX")          {resultsSolvent.push_back(eps.bornX);}
        }
        return resultsSolvent;
    }

    auto selectResultsReac ( ThermoPropertiesReaction tpr ) -> std::vector<Reaktoro_::ThermoScalar>
    {
        std::vector<Reaktoro_::ThermoScalar> resultsReac;
        for( auto name : propNames)
        {
            if (name == "reaction_gibbs_energy")       {resultsReac.push_back(tpr.reaction_gibbs_energy);}
            if (name == "reaction_helmholtz_energy")   {resultsReac.push_back(tpr.reaction_helmholtz_energy);}
            if (name == "reaction_internal_energy")    {resultsReac.push_back(tpr.reaction_internal_energy);}
            if (name == "reaction_enthalpy")           {resultsReac.push_back(tpr.reaction_enthalpy);}
            if (name == "reaction_entropy")            {resultsReac.push_back(tpr.reaction_entropy);}
            if (name == "reaction_volume")             {resultsReac.push_back(tpr.reaction_volume);}
            if (name == "reaction_heat_capacity_cp")   {resultsReac.push_back(tpr.reaction_heat_capacity_cp);}
            if (name == "reaction_heat_capacity_cv")   {resultsReac.push_back(tpr.reaction_heat_capacity_cv);}
            if (name == "logKr")                       {resultsReac.push_back(tpr.log_equilibrium_constant);}
            if (name == "lnK0")                        {resultsReac.push_back(tpr.ln_equilibrium_constant);}
        }
        return resultsReac;
    }

};

Interface::Interface(const Database& database)
    : pimpl(new Impl(database))
{}


auto Interface::thermoPropertiesSubstance(const std::string substSymbol, const double TC, const double Pbar, const std::string propName) -> Output
{
    pimpl->addSubstances({substSymbol});

    pimpl->addTPpair(TC,Pbar);

    pimpl->addProperties({propName});

    calculateSubstProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::thermoPropertiesSubstance(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double TC, double Pbar) -> Output
{
    pimpl->addSubstances(substanceSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpair(TC, Pbar);

    calculateSubstProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::thermoPropertiesSubstance(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     array<double, 3> aT, array<double, 3> aP) -> Output
{
    pimpl->addSubstances(substanceSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpairs(aT[1], aT[2], aT[3], aP[1], aP[2], aP[3]);

    calculateSubstProp();

    pimpl->clearAll();

    return Output (*this);
}

// Reaction

auto Interface::thermoPropertiesSubstance(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tpPairs) -> Output
{
    pimpl->addSubstances(substanceSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpairs(tpPairs);

    calculateSubstProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::thermoPropertiesReaction(const std::string reacSymbol, const double TC, const double Pbar, const std::string propName) -> Output
{
    pimpl->addReactions({reacSymbol});

    pimpl->addTPpair(TC,Pbar);

    pimpl->addProperties({propName});

    calculateReactProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::thermoPropertiesReaction(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                                double TC, double Pbar) -> Output
{
    pimpl->addReactions(reactionSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpair(TC, Pbar);

    calculateReactProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::thermoPropertiesReaction(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     array<double, 3> aT, array<double, 3> aP) -> Output
{
    pimpl->addReactions(reactionSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpairs(aT[1], aT[2], aT[3], aP[1], aP[2], aP[3]);

    calculateReactProp();

    pimpl->clearAll();

    return Output (*this);
}
auto Interface::thermoPropertiesReaction(std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tpPairs) -> Output
{
    pimpl->addReactions(reactionSymbols);

    pimpl->addProperties(thermoProperties);

    pimpl->addTPpairs(tpPairs);

    calculateReactProp();

    pimpl->clearAll();

    return Output (*this);
}

auto Interface::calculateSolventProp( int j ) -> void
{
    pimpl->solventProp.empty();
    unsigned tp_size = pimpl->tpPairs.size();
    pimpl->solventProp.resize(2*tp_size);
    pimpl->solventSymbol = pimpl->thermo.solventSymbol();
    if (pimpl->outSettings.outSolventProp)
    {
        auto solProp = pimpl->selectPropertiesSolvent(pimpl->thermo.propertiesSolvent(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->thermo.solventSymbol()),
                                               pimpl->thermo.electroPropertiesSolvent(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->thermo.solventSymbol()));
        pimpl->solventProp.insert(pimpl->solventProp.end(), solProp.begin(), solProp.end());
    }
}

auto Interface::calculateSubstProp( ) -> void
{
    pimpl->substResults.empty();
    unsigned j_size, i_size;
    if (pimpl->outSettings.loopOverTPpairsFirst)
        {j_size = pimpl->tpPairs.size(); i_size = pimpl->substSymbols.size();}
    else
        {j_size = pimpl->substSymbols.size(); i_size = pimpl->tpPairs.size();}

    pimpl->substResults.resize(j_size * i_size);

    for (unsigned j=0; j<j_size; j++)
    {
        for (unsigned i=0; i<i_size; i++)
        {
            if (pimpl->outSettings.loopOverTPpairsFirst)
                pimpl->substResults[(j_size*i)+(j)] = pimpl->selectResultsSubst(
                            pimpl->thermo.thermoPropertiesSubstance(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->substSymbols[i]));
            else
                pimpl->substResults[(j_size*i)+(j)] = pimpl->selectResultsSubst(
                            pimpl->thermo.thermoPropertiesSubstance(pimpl->tpPairs[i][0], pimpl->tpPairs[i][1], pimpl->substSymbols[j]));
        }
    }
}

auto Interface::calculateReactProp( ) -> void
{
    pimpl->reactResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->reactResults.resize(pimpl->reactSymbols.size() * pimpl->tpPairs.size());
    auto fromSubst = pimpl->outSettings.calcReactFromSubst;
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->reactSymbols.size(); i++)
        {
            if (fromSubst)
                pimpl->reactResults[(tp*i)+(j)] = pimpl->selectResultsReac(pimpl->thermo.thermoPropertiesReactionFromReactants(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->reactSymbols[i]));
            else
                pimpl->reactResults[(tp*i)+(j)] = pimpl->selectResultsReac(pimpl->thermo.thermoPropertiesReaction(pimpl->tpPairs[j][0], pimpl->tpPairs[j][1], pimpl->reactSymbols[i]));
        }
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
            pimpl->substResults[(tp*i)+(j)] = pimpl->selectResultsSubst(vTps[j][i]);
        }
    }
}

auto Interface::selectResultsReac_vTpSym(std::vector<std::vector<ThermoPropertiesReaction>> vTpr ) -> void
{
    pimpl->reactResults.empty(); unsigned tp = pimpl->tpPairs.size();
    pimpl->reactResults.resize(pimpl->reactSymbols.size() * pimpl->tpPairs.size());
    for (unsigned j=0; j<pimpl->tpPairs.size(); j++)
    {
        for (unsigned i=0; i<pimpl->reactSymbols.size(); i++)
        {
            pimpl->reactResults[(tp*i)+(j)] = pimpl->selectResultsReac(vTpr[j][i]);
        }
    }
}

// Set functions
auto Interface::setDigits(const std::map<std::string, int> &propDigits)-> void
{
    pimpl->propDigits = propDigits;
}

auto Interface::setUnits(const std::map<std::string, std::string> &propUnits)-> void
{

}

auto Interface::setPropertiesUnits(const std::vector<std::string> &propNames, const std::vector<std::string> &propUnits)-> void
{

}

auto Interface::setPropertiesDigits(const std::vector<std::string> &propNames, const std::vector<int> &propDigits)-> void
{

}

auto Interface::setPropertyUnit(const std::string &propName, const std::string &propUnit)-> void
{

}

auto Interface::setPropertyDigit(const std::string &propName, const int &propDigit)-> void
{

}

auto Interface::setPropertyUnitDigit(const std::string &propName, const std::string &propUnit, const int &propDigit)-> void
{

}

auto Interface::setOutputSettings(const OutputSettings &value) -> void
{
    pimpl->outSettings = value;
}

auto Interface::setSolventSymbol(const std::string solventSymbol) -> void
{
    pimpl->thermo.setSolventSymbol(solventSymbol);
}

// Private

// get functions
auto Interface::substanceSymbols() -> const std::vector<string>
{
    return pimpl->substSymbols;
}

auto Interface::reactionSymbols() -> const std::vector<string>
{
    return pimpl->reactSymbols;
}

auto Interface::outputSettings() -> const OutputSettings
{
    return pimpl->outSettings;
}

auto Interface::TPpairs() -> const std::vector<std::vector<double> >
{
    return pimpl->tpPairs;
}

auto Interface::propNames() -> const std::vector<std::string>
{
    return pimpl->propNames;
}

auto Interface::solventPropNames() -> const std::vector<std::string>
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
    return pimpl->reactResults;
}

}

