#include "Output.h"
#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace TCorrPT {

struct Output::Impl
{
    /// Interface instance
    Interface api;

    std::string                         header = "";
    std::ofstream                       fThermoPropertiesSubstance;

    Impl(const Interface& interface)
    : api(Interface(interface))
    {}

};

Output::Output(const Interface& interface)
: pimpl(new Impl(interface))
{}

auto Output::toCSV( ) -> void
{
    pimpl->fThermoPropertiesSubstance.open( pimpl->api.outputSettings().fileNameSubst, ios::trunc );

    pimpl->fThermoPropertiesSubstance << CSVHeader() << endl;

    foutResultsSubst();

    pimpl->fThermoPropertiesSubstance.close();

}

//auto Output::toThermoScalar() -> Reaktoro_::ThermoScalar
//{
//    return pimpl->api.resultsSubst()[0][0];
//}

//auto Output::to2DVectorThermoScalar() -> std::vector<std::vector<Reaktoro_::ThermoScalar>>
//{
//    return pimpl->api.resultsSubst();
//}

auto Output::toDouble() -> double
{
    return pimpl->api.resultsSubst()[0][0].val;
}

auto Output::to2DVectorDouble() -> std::vector<std::vector<double>>
{
    std::vector<std::vector<Reaktoro_::ThermoScalar>> vectorTS = pimpl->api.resultsSubst();
    std::vector<std::vector<double>> vectorD;
    vectorD.resize(vectorTS.size());
    for (unsigned i=0; i<vectorTS.size(); i++)
    {
        vectorD[i].resize(vectorTS[i].size());
        for (unsigned j=0; j<vectorTS[i].size(); j++)
            vectorD[i][j] = vectorTS[i][j].val;
    }
    return vectorD;
}

auto Output::CSVHeader( ) -> std::string
{
    typedef std::map<int, std::string>::iterator it_t;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<const std::string, std::string> units  = pimpl->api.propUnits();

    const auto s = pimpl->api.outputSettings().separator;
    std::string header = "";

    if (substanceSymbols.size() > 0)
    {
        header = header + "Substance" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;
        for(it_t it = properties.begin(); it != properties.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }
    } else
    {
        header = header + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")"/* + s*/;
        for(it_t it = properties.begin(); it != properties.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }

    }

    return header;

}

auto Output::foutResultsSubst()-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<const std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TP_pairs       = pimpl->api.TP_pairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsSubst   = pimpl->api.resultsSubst();

    if (pimpl->api.outputSettings().isFixed) pimpl->fThermoPropertiesSubstance << std::fixed;

    for (unsigned i=0; i<substanceSymbols .size(); i++)
    {
        for (unsigned j=0; j<TP_pairs.size(); j++)
        {
            pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at("temperature"));
            pimpl->fThermoPropertiesSubstance << substanceSymbols[i] << s << TP_pairs[j][0];
            pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at("pressure"));
            pimpl->fThermoPropertiesSubstance << s << TP_pairs[j][1];

            for (unsigned k=0; k<resultsSubst[i].size(); k++)
            {
                pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at(properties.at(k+1)));
                pimpl->fThermoPropertiesSubstance << s << resultsSubst[c][k].val;
            }
            c++;
            pimpl->fThermoPropertiesSubstance << endl;
        }
    }

}

}
