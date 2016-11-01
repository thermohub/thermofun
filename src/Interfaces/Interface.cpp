#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace TCorrPT {

struct Interface::Impl
{
    /// The thermo instance
    Thermo thermo;

    OutputSettings                           outputSettings;

    std::vector<string>                      substanceSymbols;
    std::vector<std::vector<double>>         TP_pairs;
    map<int, std::string>                    propNames;
    std::map<const std::string, std::string> propUnits  = defaultPropertyUnits;
    std::map<const std::string, int>         propDigits = defaultPropertyDigits;

    std::vector<std::vector<double>>         results;
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsThermoScalar;

    Impl(const Database& database)
    : thermo(Thermo(database))
    {}


public:
};

Interface::Interface(const Database& database)
    : pimpl(new Impl(database))
{}

auto Interface::thermoCalculate() -> Output
{
    calculateResults();

    return Output (*this);
}

auto Interface::thermoCalculate(const std::string substSymbol, const double T, const double P, const std::string propName) -> Output
{
    addSubstanceSymbol(substSymbol);

    addTP_pairs(T,P);

    addPropertyName(propName);

    calculateResults();

    return Output (*this);
}

auto Interface::thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> Output
{
    addSubstanceSymbols(substanceSymbols);

    addPropertyNames(thermoProperties);

    addTP_pairs(T, P);

    calculateResults();

    return Output (*this);
}

auto Interface::thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output
{
    addSubstanceSymbols(substanceSymbols);

    addPropertyNames(thermoProperties);

    addTP_pairs(Tmin, Tmax, Tstep, Pmin, Pmax, Pstep);

    calculateResults();

    return Output (*this);
}
auto Interface::thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                     std::vector<std::vector<double> > tp_pairs) -> Output
{
   return Output (*this);
}

auto Interface::selectResults ( ThermoPropertiesSubstance tps ) -> std::vector<double>
{
    std::vector<double> results;
    std::map<int, std::string>::iterator it;

    for(it = pimpl->propNames.begin(); it != pimpl->propNames.end(); it++)
    {
        if (it->second == "gibbs_energy")       {results.push_back(tps.gibbs_energy.val);}
        if (it->second == "helmholtz_energy")   {results.push_back(tps.helmholtz_energy.val);}
        if (it->second == "internal_energy")    {results.push_back(tps.internal_energy.val);}
        if (it->second == "enthalpy")           {results.push_back(tps.enthalpy.val);}
        if (it->second == "entropy")            {results.push_back(tps.entropy.val);}
        if (it->second == "volume")             {results.push_back(tps.volume.val);}
        if (it->second == "heat_capacity_cp")   {results.push_back(tps.heat_capacity_cp.val);}
        if (it->second == "heat_capacity_cv")   {results.push_back(tps.heat_capacity_cv.val);}
    }
    return results;
}

auto Interface::calculateResults( ) -> void
{
    pimpl->results.empty(); unsigned int c = 0;
    pimpl->results.resize(pimpl->substanceSymbols.size() * pimpl->TP_pairs.size());
    for (unsigned i=0; i<pimpl->substanceSymbols.size(); i++)
    {
        for (unsigned j=0; j<pimpl->TP_pairs.size(); j++)
        {
            pimpl->results[c] = selectResults(pimpl->thermo.thermoPropertiesSubstance(pimpl->TP_pairs[j][0], pimpl->TP_pairs[j][1], pimpl->substanceSymbols[i]));
            c++;
        }
    }
}


// Add functions

auto Interface::addSubstanceSymbol (const std::string &substSymbol) -> void
{
    if (std::find(pimpl->substanceSymbols.begin(), pimpl->substanceSymbols.end(), substSymbol) == pimpl->substanceSymbols.end())
        pimpl->substanceSymbols.push_back(substSymbol);
    // add exception ??
}

auto Interface::addSubstanceSymbols (const std::vector<string> &substSymbols) -> void
{
    pimpl->substanceSymbols = substSymbols;
    // add exception ??
}

auto Interface::addPropertyName (const std::string &propName) -> void
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

auto Interface::addPropertyNames (const std::vector<string> &propNames) -> void
{
    std::map<const std::string, const std::string>::const_iterator it;

    for (unsigned i = 0; i<propNames.size(); i++)
    {
        addPropertyName(propNames[i]);
    }
}

auto Interface::addTP_pairs (const double &T, const double &P) -> void
{
    std::vector<double> one_pair = {T, P};
    pimpl->TP_pairs.push_back(one_pair);
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
            addTP_pairs(t,p);
            p = p + Pstep;
        } while (p <= Pmax);
       t = t + Tstep;
    } while (t <= Tmax);
}

auto Interface::addTP_pairs (const std::vector<std::vector<double>> &TP_pairs) -> void
{
    pimpl->TP_pairs = TP_pairs;
}

// set functions
auto Interface::setFoutputSettings(const OutputSettings &value) -> void
{
    pimpl->outputSettings = value;
}

// get functions

auto Interface::substanceSymbols() -> const std::vector<string>
{
    return pimpl->substanceSymbols;
}

auto Interface::foutputSettings() -> const OutputSettings
{
    return pimpl->outputSettings;
}

auto Interface::TP_pairs() -> const std::vector<std::vector<double> >
{
    return pimpl->TP_pairs;
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

auto Interface::results() -> const std::vector<std::vector<double> >
{
    return pimpl->results;
}

}

