#include "ThermoBatch.h"

// C++ includes
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

// ThermoFun includes
#include "ThermoProperties.h"
#include "Database.h"
#include "ThermoEngine.h"
#include "Common/Units.hpp"

#ifdef _WIN32
#include <array>
#else
#endif

namespace ThermoFun {

enum Calculation {forSUBSTANCE, forREACTION, forSOLVENT};

struct ThermoBatch::Impl
{
    /// The thermo instance
    ThermoEngine                        thermo;

    BatchPreferences                    outSettings;

//    BatchCalculationSettings            calcSettings;

    vvd                                 tpPairs;

    std::vector <double>                Ts;

    std::vector <double>                Ps;

    vstr                                properties;

    std::map<std::string, std::string>  givenPropertyUnits  = defaultPropertyUnits;

    std::map<std::string, int>          givenPropertyDigits = defaultPropertyDigits;

    vstr                                symbols;

    vvThScalar                          results;

    Impl(const Database& database)
    : thermo(ThermoEngine(database))
    {}

    Impl(std::string filename)
    : thermo(filename)
    {}

    Impl(const ThermoEngine& engine)
    : thermo(engine)
    {}

    // clear functions
    auto clearSymbols () -> void
    {
        symbols.clear();
    }

    auto clearProperties () -> void
    {
        properties.clear();
    }

    auto clearTemperatures () -> void
    {
        Ts.clear();
    }

    auto clearPressures () -> void
    {
        Ps.clear();
    }

    auto clearTPpairs () -> void
    {
        tpPairs.clear();
        clearTemperatures();
        clearPressures();
    }

    auto clearAll() -> void
    {
        clearProperties();
        clearSymbols();
        clearTPpairs();
        clearTemperatures();
        clearPressures();
    }

    // Add functions
    auto addSymbols(const vstr &symbols_) -> void
    {
        symbols = symbols_;
    }

    auto addProperties (const vstr &names) -> void
    {
        properties = names;
    }

    auto addSymbolsProperties(const vstr &symbols_, const vstr &properties) -> void
    {
        clearAll();
        addSymbols(symbols_);
        addProperties(properties);
    }

    auto addTPpair (const double &T, const double &P) -> void
    {
        std::vector<double> one_pair = {T, P};
        if ( std::find(Ts.begin(), Ts.end(), T) == Ts.end() )
           Ts.push_back(T);
        if ( std::find(Ps.begin(), Ps.end(), P) == Ps.end() )
           Ps.push_back(P);

        tpPairs.push_back(one_pair);
    }

    auto addTPpairs (const std::vector<double> &temperatures, const std::vector<double> &pressures) -> void
    {
        clearTPpairs();
        std::vector<double> rows, columns;
        if (outSettings.loopTemperatureThenPressure)
        { rows = temperatures; columns = pressures; }
        else
        { rows = pressures; columns = temperatures; }

        for (auto r : rows)
            for (auto c : columns)
                if (outSettings.loopTemperatureThenPressure)
                    addTPpair(r,c);
                else
                    addTPpair(c,r);
    }

    auto addTPpairs (const double &Tmin, const double &Tmax, const double &Tstep,
                     const double &Pmin, const double &Pmax, const double &Pstep) -> void
    {
        clearTPpairs();
        double t = Tmin-Tstep;
        double p = Pmin-Pstep;
        std::vector<double> temperatures;
        std::vector<double> pressures;

        do
        {
            p = p + Pstep;
            pressures.push_back(p);
        } while (p < Pmax);

        do
        {
            t = t + Tstep;
            temperatures.push_back(t);
        } while (t < Tmax);

        addTPpairs(temperatures, pressures);
    }

    auto addTPpairs (const vvd &tpPairs) -> void
    {
        clearTPpairs();
        for (unsigned i=0; i <tpPairs.size(); i++)
        {
            addTPpair(tpPairs[i][0], tpPairs[i][1]);
        }
    }

    // select functions
    auto selectResultsSubst ( ThermoPropertiesSubstance tps ) -> vThScalar
    {
        vThScalar resultsSubst;
        for( auto name : properties)
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

    auto selectResultsSolvent ( PropertiesSolvent ps, ElectroPropertiesSolvent eps ) -> vThScalar
    {
        vThScalar resultsSolvent;
        for( auto name : properties)
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

    auto selectResultsReact ( ThermoPropertiesReaction tpr ) -> vThScalar
    {
        vThScalar resultsReac;
        for( auto name : properties)
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

    auto calculate(Calculation calculation) -> void
    {
        double T, P; string symbol; size_t j_size, i_size;
        auto defUnitT = defaultPropertyUnits.at("temperature");
        auto unitT    =   givenPropertyUnits.at("temperature");
        auto defUnitP = defaultPropertyUnits.at("pressure");
        auto unitP    =   givenPropertyUnits.at("pressure");
        results.clear();

        if (outSettings.loopOverTPpairsFirst)
        {j_size = tpPairs.size(); i_size = symbols.size();}
        else
        {j_size = symbols.size(); i_size = tpPairs.size();}

        results.resize(j_size * i_size);

        for (unsigned j=0; j<j_size; j++)
        {
            for (unsigned i=0; i<i_size; i++)
            {
                if (outSettings.loopOverTPpairsFirst)
                {
                    T = units::convert(tpPairs[j][0], unitT, defUnitT);
                    P = units::convert(tpPairs[j][1], unitP, defUnitP);
                    symbol = symbols[i];
                } else
                {
                    T = units::convert(tpPairs[i][0], unitT, defUnitT);
                    P = units::convert(tpPairs[i][1], unitP, defUnitP);
                    symbol = symbols[j];
                }

                switch(calculation) {
                case forSUBSTANCE: calculateSubstProp(T, P, symbol, (j_size*i)+(j)); break;
                case forREACTION: calculateReactProp (T, P, symbol, (j_size*i)+(j)); break;
                case forSOLVENT: calculateSolventProp(T, P, symbol, (j_size*i)+(j)); break;
                }

                // in case of Psat = 0 updating with real value
                if (outSettings.loopOverTPpairsFirst)
                {
                    tpPairs[j][1] = units::convert(P, defUnitP, unitP);
                    symbol = symbols[i];
                } else
                {
                    tpPairs[i][1] = units::convert(P, defUnitP, unitP);
                    symbol = symbols[j];
                }
            }
        }
        auto tpp = tpPairs;
        addTPpairs(tpp);
    }

    auto selectProvidedSubstancesProperties(vtps vTps) -> void
    {
        results.clear();
        for (auto result : vTps)
        {
            results.push_back(selectResultsSubst(result));
        }
    }

    auto selectProvidedReactionsProperties(vtpr vTpr) -> void
    {
        results.clear();
        for (auto result : vTpr)
        {
            results.push_back(selectResultsReact(result));
        }
    }

    // Calculate functions
    auto calculateSubstProp( double T, double &P, string symbol, unsigned index ) -> void
    {
            results[index] = selectResultsSubst(thermo.thermoPropertiesSubstance(T, P, symbol));
    }

    auto calculateReactProp( double T, double &P, string symbol, unsigned index ) -> void
    {
        if (outSettings.reactionPropertiesFromReactants)
            results[index] = selectResultsReact(thermo.thermoPropertiesReactionFromReactants(T, P, symbol));
        else
            results[index] = selectResultsReact(thermo.thermoPropertiesReaction(T, P, symbol));
    }

    auto calculateSolventProp( double T, double &P, string symbol, unsigned index ) -> void
    {
        results[index] = selectResultsSolvent(thermo.propertiesSolvent(T, P, symbol),
                                              thermo.electroPropertiesSolvent(T, P, symbol));
    }
};

ThermoBatch::ThermoBatch(const Database& database)
    : pimpl(new Impl(database))
{}

ThermoBatch::ThermoBatch(const ThermoEngine& engine)
    : pimpl(new Impl(engine))
{}

ThermoBatch::ThermoBatch(std::string filename)
    : pimpl(new Impl(filename))
{}

auto ThermoBatch::thermoPropertiesSubstance(double T, double P, std::string symbol, std::string property) -> Output
{
    pimpl->addSymbolsProperties({symbol}, {property});

    pimpl->addTPpair(T,P);

    pimpl->calculate(forSUBSTANCE);

    return Output(*this);
}

auto ThermoBatch::thermoPropertiesSubstance(double T, double P, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpair(T, P);

    pimpl->calculate(forSUBSTANCE);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesSubstance(std::map<std::string, double> Tincrement, std::map<std::string, double> Pincrement, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(Tincrement["Tmin"], Tincrement["Tmax"], Tincrement["Tstep"], Pincrement["Pmin"], Pincrement["Pmax"], Pincrement["Pstep"]);

    pimpl->calculate(forSUBSTANCE);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesSubstance(std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(temperatures, pressures);

    pimpl->calculate(forSUBSTANCE);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesSubstance(std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties, vtps vTps) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(temperatures, pressures);

    if (vTps.empty())
        pimpl->calculate(forSUBSTANCE);
    else
        pimpl->selectProvidedSubstancesProperties(vTps);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesSubstance(vvd tpPairs, vstr symbols, vstr properties) -> Output
{
    return thermoPropertiesSubstance(tpPairs,symbols,properties,{});
}

auto ThermoBatch::thermoPropertiesSubstance(vvd tpPairs, vstr symbols, vstr properties, vtps vTps) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(tpPairs);

    if (vTps.empty())
        pimpl->calculate(forSUBSTANCE);
    else
        pimpl->selectProvidedSubstancesProperties(vTps);

    return Output (*this);
}

// Reaction
auto ThermoBatch::thermoPropertiesReaction(double T, double P, std::string symbol, std::string property) -> Output
{
    pimpl->addSymbolsProperties({symbol}, {property});

    pimpl->addTPpair(T,P);

    pimpl->calculate(forREACTION);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesReaction(double T, double P, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpair(T, P);

    pimpl->calculate(forREACTION);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesReaction(std::map<std::string, double> Tincrement, std::map<std::string, double> Pincrement, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(Tincrement["Tmin"], Tincrement["Tmax"], Tincrement["Tstep"], Pincrement["Pmin"], Pincrement["Pmax"], Pincrement["Pstep"]);

    pimpl->calculate(forREACTION);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesReaction(std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(temperatures, pressures);

    pimpl->calculate(forREACTION);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesReaction(std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties, vtpr vTpr) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(temperatures, pressures);

    if (vTpr.empty())
        pimpl->calculate(forREACTION);
    else
        pimpl->selectProvidedReactionsProperties(vTpr);

    return Output (*this);
}

auto ThermoBatch::thermoPropertiesReaction(vvd tpPairs, vstr symbols, vstr properties) -> Output
{
    return thermoPropertiesReaction(tpPairs, symbols, properties, {});
}

auto ThermoBatch::thermoPropertiesReaction(vvd tpPairs, vstr symbols, vstr properties, vtpr vTpr) -> Output
{
    pimpl->addSymbolsProperties(symbols, properties);

    pimpl->addTPpairs(tpPairs);

    if (vTpr.empty())
        pimpl->calculate(forREACTION);
    else
        pimpl->selectProvidedReactionsProperties(vTpr);

    return Output (*this);
}

// Set functions
auto ThermoBatch::setDigits(const std::map<std::string, int> &propDigits)-> void
{
    pimpl->givenPropertyDigits = propDigits;
}

auto ThermoBatch::setUnits(const std::map<std::string, std::string> &units)-> void
{
    pimpl->givenPropertyUnits = units;
}

auto ThermoBatch::setPropertiesUnits(const vstr &properties, const vstr &units)-> void
{
    for (std::size_t i=0; i<properties.size(); i++)
        setPropertyUnit(properties[i], units[i]);
}

auto ThermoBatch::setPropertiesDigits(const vstr &properties, const std::vector<int> &propDigits)-> void
{
    for (std::size_t i=0; i<properties.size(); i++)
        setPropertyDigit(properties[i], propDigits[i]);
}

auto ThermoBatch::setPropertyUnit(const std::string &property, const std::string &unit)-> void
{
    pimpl->givenPropertyUnits.at(property) = unit;
}

auto ThermoBatch::setPropertyDigit(const std::string &property, const int &digit)-> void
{
    pimpl->givenPropertyDigits.at(property) = digit;
}

auto ThermoBatch::setPropertyUnitDigit(const std::string &property, const std::string &unit, const int &digit)-> void
{
    setPropertyUnit(property, unit);
    setPropertyDigit(property, digit);
}

auto ThermoBatch::setBatchPreferences(const BatchPreferences &value) -> void
{
    pimpl->outSettings = value;
}

auto ThermoBatch::setSolventSymbol(const std::string solventSymbol) -> void
{
    pimpl->thermo.setSolventSymbol(solventSymbol);
}

// Private
// get functions
auto ThermoBatch::symbols() -> const vstr
{
    return pimpl->symbols;
}

auto ThermoBatch::outputSettings() -> const BatchPreferences
{
    return pimpl->outSettings;
}

auto ThermoBatch::TPpairs() -> const vvd
{
    return pimpl->tpPairs;
}

auto ThermoBatch::Temperatures() -> const std::vector <double>
{
    return pimpl->Ts;
}

auto ThermoBatch::Pressures() -> const std::vector <double>
{
    return pimpl->Ps;
}

auto ThermoBatch::properties() -> const vstr
{
    return pimpl->properties;
}

auto ThermoBatch::units() -> const std::map<std::string, std::string>
{
    return pimpl->givenPropertyUnits;
}

auto ThermoBatch::digits() -> const std::map<std::string, int>
{
    return pimpl->givenPropertyDigits;
}

auto ThermoBatch::results() -> const vvThScalar
{
    return pimpl->results;
}

}

