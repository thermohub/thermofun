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
    pimpl->fThermoPropertiesSubstance.open( pimpl->api.foutputSettings().fileName, ios::trunc );

    pimpl->fThermoPropertiesSubstance << CSVHeader() << endl;

    foutResults();

    pimpl->fThermoPropertiesSubstance.close();

}

auto Output::toDouble() -> double
{
    return pimpl->api.results()[0][0];
}

auto Output::to2DVectorDouble() -> std::vector<std::vector<double>>
{
    return pimpl->api.results();
}

auto Output::CSVHeader( ) -> std::string
{
    typedef std::map<int, std::string>::iterator it_t;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<const std::string, std::string> units  = pimpl->api.propUnits();

    const auto s = pimpl->api.foutputSettings().separator;
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

auto Output::foutResults()-> void
{
    const auto s = pimpl->api.foutputSettings().separator; unsigned int c = 0;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<const std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TP_pairs       = pimpl->api.TP_pairs();
    std::vector<std::vector<double>> results        = pimpl->api.results();

    if (pimpl->api.foutputSettings().isFixed) pimpl->fThermoPropertiesSubstance << std::fixed;

    for (unsigned i=0; i<substanceSymbols .size(); i++)
    {
        for (unsigned j=0; j<TP_pairs.size(); j++)
        {
            pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at("temperature"));
            pimpl->fThermoPropertiesSubstance << substanceSymbols[i] << s << TP_pairs[j][0];
            pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at("pressure"));
            pimpl->fThermoPropertiesSubstance << s << TP_pairs[j][1];

            for (unsigned k=0; k<results[i].size(); k++)
            {
                pimpl->fThermoPropertiesSubstance << std::setprecision(digits.at(properties.at(k+1)));
                pimpl->fThermoPropertiesSubstance << s << results[c][k];
            }
            c++;
            pimpl->fThermoPropertiesSubstance << endl;
        }
    }

}

}
