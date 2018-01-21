#include "Output.h"
#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include "../GlobalVariables.h"
#include "../Common/Units.hpp"

namespace ThermoFun {

struct Output::Impl
{
    /// Interface instance
    Interface api;

    std::string                         header = "";
    std::ofstream                       fProperties;

    Impl(const Interface& interface)
    : api(Interface(interface))
    {}

};

Output::Output(const Interface& interface)
: pimpl(new Impl(interface))
{}

auto find_and_replace(std::string source, std::string const& find, std::string const& replace) -> std::string
{
    for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
    {
        source.replace(i, find.length(), replace);
        i += replace.length();
    }
    return source;
}

auto Output::toCSV(std::string filename) -> void
{
    pimpl->fProperties.open( filename, std::ios::trunc );

    pimpl->fProperties << CSVHeader() << std::endl;

    foutResults();

    pimpl->fProperties.close();
}

auto Output::toCSVTransposed(std::string filename) -> void
{
    pimpl->fProperties.open( filename, std::ios::trunc );

    pimpl->fProperties << CSVHeaderTransposed() << std::endl;

    foutResultsTransposed();

    pimpl->fProperties.close();
}


auto Output::toDouble() -> double
{
    return pimpl->api.results()[0][0].val;
}

auto Output::to2DVectorDouble() -> std::vector<std::vector<double>>
{
    std::vector<std::vector<Reaktoro_::ThermoScalar>> vectorTS;
    std::vector<std::vector<double>> vectorD;

    vectorTS = pimpl->api.results();

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
    auto properties = pimpl->api.properties();
    auto units      = pimpl->api.units();
    auto s          = pimpl->api.outputSettings().separator;
    std::string header = "";

    header = header + "Symbol" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;

    for(auto prop : properties)
    { header = header + s + prop + "(" + units.at(prop) + ")"; }

    return header;
}

auto Output::CSVHeaderTransposed( ) -> std::string
{
    auto symbols    = pimpl->api.symbols();
    auto units      = pimpl->api.units();
    auto s          = pimpl->api.outputSettings().separator;
    std::string header = "";

    header = header + "Property" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")";

    for (auto symbol : symbols)
    { header = header + s + find_and_replace(symbol, s, "_"); }

    return header;
}

auto Output::foutResults()-> void
{
    unsigned j_size, i_size;
    auto outSettings    = pimpl->api.outputSettings();
    auto s              = outSettings.separator;
    auto symbols        = pimpl->api.symbols();
    auto properties     = pimpl->api.properties();
    auto digits         = pimpl->api.digits();
    auto fromUnits      = defaultPropertyUnits;
    auto toUnits        = pimpl->api.units();
    auto tpPairs        = pimpl->api.TPpairs();
    auto results        = pimpl->api.results();

    if (outSettings.loopOverTPpairsFirst)
    {j_size = tpPairs.size(); i_size = symbols.size();}
    else
    {j_size = symbols.size(); i_size = tpPairs.size();}

    if (pimpl->api.outputSettings().isFixed) pimpl->fProperties << std::fixed;
    if (pimpl->api.outputSettings().isScientific) pimpl->fProperties << std::scientific;

    for (unsigned i=0; i<i_size; i++)
    {
        for (unsigned j=0; j<j_size; j++)
        {
            double T, P; string symbol;
            if (outSettings.loopOverTPpairsFirst)
            {
                T = tpPairs[j][0];
                P = tpPairs[j][1];
                symbol = symbols[i];
            } else
            {
                T = tpPairs[i][0];
                P = tpPairs[i][1];
                symbol = symbols[j];
            }

            pimpl->fProperties << std::setprecision(digits.at("temperature"));
            pimpl->fProperties << find_and_replace(symbol, s, "_") << s << T;
            pimpl->fProperties << std::setprecision(digits.at("pressure"));
            pimpl->fProperties << s << P;

            for (unsigned k=0; k<results[(j_size*i)+(j)].size(); k++)
            {
                auto prop = properties[k];
                pimpl->fProperties << std::setprecision(digits.at(prop));
                pimpl->fProperties << s << units::convert(results[(j_size*i)+(j)][k].val,
                                                          fromUnits.at(prop),
                                                            toUnits.at(prop));
            }
            pimpl->fProperties << std::endl;
        }
    }
}

auto Output::foutResultsTransposed( )-> void
{
    unsigned j_size, i_size;
    auto outSettings    = pimpl->api.outputSettings();
    auto s              = outSettings.separator;
    auto properties     = pimpl->api.properties();
    auto digits         = pimpl->api.digits();
    auto fromUnits      = defaultPropertyUnits;
    auto toUnits        = pimpl->api.units();
    auto tpPairs        = pimpl->api.TPpairs();
    auto results        = pimpl->api.results();
    auto symbols        = pimpl->api.symbols();

    if (outSettings.loopOverTPpairsFirst)
    {j_size = tpPairs.size(); i_size = properties.size();}
    else
    {j_size = properties.size(); i_size = tpPairs.size();}

    if (pimpl->api.outputSettings().isFixed) pimpl->fProperties << std::fixed;
    if (pimpl->api.outputSettings().isScientific) pimpl->fProperties << std::scientific;


    for (unsigned i=0; i<i_size; i++)
    {
        for (unsigned j=0; j<j_size; j++)
        {
            double T, P; string property;
            if (outSettings.loopOverTPpairsFirst)
            {
                T = tpPairs[j][0];
                P = tpPairs[j][1];
                property = properties[i];
            } else
            {
                T = tpPairs[i][0];
                P = tpPairs[i][1];
                property = properties[j];
            }

            pimpl->fProperties << std::setprecision(digits.at("temperature"));
            pimpl->fProperties << find_and_replace(property, s, "_") + "(" + toUnits.at(property) + ")" << s << T;
            pimpl->fProperties << std::setprecision(digits.at("pressure"));
            pimpl->fProperties << s << P;

            for (unsigned k=0; k<symbols.size(); k++)  // loops thorugh symbols
            {
                pimpl->fProperties << std::setprecision(digits.at(property));
                double result;
                if (outSettings.loopOverTPpairsFirst)
                    result = results[(tpPairs.size()*k)+j][i].val;
                else
                    result = results[(symbols.size()*i)+k][j].val;

                pimpl->fProperties << s << units::convert(result, fromUnits.at(property), toUnits.at(property));
            }
            pimpl->fProperties << std::endl;
        }
    }
}

}
