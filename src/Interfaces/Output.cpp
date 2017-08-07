#include "Output.h"
#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace ThermoFun {

struct Output::Impl
{
    /// Interface instance
    Interface api;

    std::string                         header = "";
    std::ofstream                       fThermoProperties;
    std::ofstream                       fSolventProperties;

    Impl(const Interface& interface)
    : api(Interface(interface))
    {}

};

Output::Output(const Interface& interface)
: pimpl(new Impl(interface))
{}

auto Output::toCSV(std::string filename) -> void
{
    pimpl->fThermoProperties.open( filename, std::ios::trunc );

    pimpl->fThermoProperties << CSVHeader() << std::endl;

    foutResultsSubst();
    foutResultsReac();

    pimpl->fThermoProperties.close();

    if (pimpl->api.outputSettings().outSolventProp)
    {
        pimpl->fSolventProperties.open(pimpl->api.outputSettings().solventFileName, std::ios::trunc);
        pimpl->fSolventProperties << CSVSolventHeader() << std::endl;
        foutResultsSolv();
        pimpl->fSolventProperties.close();
    }
}

auto Output::toCSVtransposed(std::string filename, std::string propertyname) -> void
{
    pimpl->fThermoProperties.open( filename, std::ios::trunc );

    pimpl->fThermoProperties << CSVHeaderTransposed() << std::endl;

    foutResultsSubstTrans(propertyname);
    foutResultsReacTrans(propertyname);

    pimpl->fThermoProperties.close();
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
    if (pimpl->api.substanceSymbols().size() > 0)
    return pimpl->api.resultsSubst()[0][0].val;
    if (pimpl->api.reactionSymbols().size() > 0)
    return pimpl->api.resultsReac()[0][0].val;
}

auto Output::to2DVectorDouble() -> std::vector<std::vector<double>>
{
    std::vector<std::vector<Reaktoro_::ThermoScalar>> vectorTS;
    std::vector<std::vector<double>> vectorD;

    if (pimpl->api.substanceSymbols().size() > 0)
        vectorTS = pimpl->api.resultsSubst();
    if (pimpl->api.reactionSymbols().size() > 0)
        vectorTS = pimpl->api.resultsReac();

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
    std::vector<std::string> reactionSymbols        = pimpl->api.reactionSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, std::string> units        = pimpl->api.propUnits();

    const auto s = pimpl->api.outputSettings().separator;
    std::string header = "";

    if (substanceSymbols.size() > 0)
    {
        header = header + "Substance" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;
        for(it_t it = properties.begin(); it != properties.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }
    }
    //else
//    {
//        header = header + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")"/* + s*/;
//        for(it_t it = properties.begin(); it != properties.end(); it++)
//        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }
//    }

    if (reactionSymbols.size() > 0)
    {
        header = header + "Reaction" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;
        for(it_t it = properties.begin(); it != properties.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }
    }
    return header;
}

auto Output::CSVSolventHeader( ) -> std::string
{
    std::string header = "";
    auto properties = pimpl->api.solventPropNames();
    auto units  = pimpl->api.solventPropUnits();
    const auto s = pimpl->api.outputSettings().separator;

    header = header + "Substance" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;
    for(auto prop : properties)
    {   header = header + s + prop.second + "(" + units.at(prop.second) + ")"; }
    return header;
}

auto Output::CSVHeaderTransposed( ) -> std::string
{
//    typedef std::map<int, std::string>::iterator it_t;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::vector<std::string> reactionSymbols        = pimpl->api.reactionSymbols();
//    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, std::string> units  = pimpl->api.propUnits();

    const auto s = pimpl->api.outputSettings().separator;
    std::string header = "";
    header = header + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")";

    for (auto symbol : substanceSymbols)
    {
        header = header + s + symbol;
    }

    for (auto symbol : reactionSymbols)
    {
        header = header + s + symbol;
    }
    return header;
}

auto Output::foutResultsSolv()-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

//    std::map<int, std::string> properties            = pimpl->api.solventPropNames();
    std::map<std::string, int> digits                = pimpl->api.solventPropDigits();
    std::vector<std::vector<double>> TPpairs         = pimpl->api.TPpairs();
    std::vector<Reaktoro_::ThermoScalar> solventProp = pimpl->api.solventProp();
    auto solventSymbol                               = pimpl->api.solventSymbol();

    for (unsigned j=0; j<TPpairs.size(); j++)
    {
        pimpl->fSolventProperties << std::setprecision(digits.at("temperature"));
        pimpl->fSolventProperties << solventSymbol << s << TPpairs[j][0];
        pimpl->fSolventProperties << std::setprecision(digits.at("pressure"));
        pimpl->fSolventProperties << s << TPpairs[j][1];

        for (unsigned k=0; k<solventProp.size(); k++)
        {
//            pimpl->fSolventProperties << std::setprecision(digits.at(properties.at(k+1)));
            pimpl->fSolventProperties << s << solventProp[c].val;
            c++;
        }
        pimpl->fSolventProperties << std::endl;
    }
}

auto Output::foutResultsSubst()-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> substanceSymbols        = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties            = pimpl->api.propNames();
    std::map<std::string, int> digits                = pimpl->api.propDigits();
    std::vector<std::vector<double>> TPpairs         = pimpl->api.TPpairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsSubst   = pimpl->api.resultsSubst();
    std::vector<Reaktoro_::ThermoScalar> solventProp = pimpl->api.solventProp();
    const auto outSol = pimpl->api.outputSettings().outSolventProp;

    if (pimpl->api.outputSettings().isFixed) pimpl->fThermoProperties << std::fixed;

    for (unsigned i=0; i<substanceSymbols .size(); i++)
    {
        for (unsigned j=0; j<TPpairs.size(); j++)
        {
            pimpl->fThermoProperties << std::setprecision(digits.at("temperature"));
            pimpl->fThermoProperties << substanceSymbols[i] << s << TPpairs[j][0];
            pimpl->fThermoProperties << std::setprecision(digits.at("pressure"));
            pimpl->fThermoProperties << s << TPpairs[j][1];

            for (unsigned k=0; k<resultsSubst[i].size(); k++)
            {
                pimpl->fThermoProperties << std::setprecision(digits.at(properties.at(k+1)));
                pimpl->fThermoProperties << s << resultsSubst[c][k].val;
            }
            c++;
            pimpl->fThermoProperties << std::endl;
        }
    }
}

auto Output::foutResultsReac()-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> reactionSymbols        = pimpl->api.reactionSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TPpairs       = pimpl->api.TPpairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsReac   = pimpl->api.resultsReac();

    if (pimpl->api.outputSettings().isFixed) pimpl->fThermoProperties << std::fixed;

    for (unsigned i=0; i<reactionSymbols .size(); i++)
    {
        for (unsigned j=0; j<TPpairs.size(); j++)
        {
            pimpl->fThermoProperties << std::setprecision(digits.at("temperature"));
            pimpl->fThermoProperties << reactionSymbols[i] << s << TPpairs[j][0];
            pimpl->fThermoProperties << std::setprecision(digits.at("pressure"));
            pimpl->fThermoProperties << s << TPpairs[j][1];

            for (unsigned k=0; k<resultsReac[i].size(); k++)
            {
                pimpl->fThermoProperties << std::setprecision(digits.at(properties.at(k+1)));
                pimpl->fThermoProperties << s << resultsReac[c][k].val;
            }
            c++;
            pimpl->fThermoProperties << std::endl;
        }
    }
}

auto Output::foutResultsSubstTrans(std::string property)-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TPpairs       = pimpl->api.TPpairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsSubst   = pimpl->api.resultsSubst();
    unsigned tp = TPpairs.size();

    if (pimpl->api.outputSettings().isFixed) pimpl->fThermoProperties << std::fixed;

    for (unsigned j=0; j<TPpairs.size(); j++)
    {
        pimpl->fThermoProperties << std::setprecision(digits.at("temperature"));
        pimpl->fThermoProperties << TPpairs[j][0];
        pimpl->fThermoProperties << std::setprecision(digits.at("pressure"));
        pimpl->fThermoProperties << s << TPpairs[j][1];
        for (unsigned i=0; i<substanceSymbols .size(); i++)
        {
            for (unsigned k=0; k<resultsSubst[i].size(); k++)
            {
                if (properties.at(k+1) == property)
                {
                    pimpl->fThermoProperties << std::setprecision(digits.at(properties.at(k+1)));
                    pimpl->fThermoProperties << s << resultsSubst[(tp*i)+(j)][k].val;
                }
            }
        }
        pimpl->fThermoProperties << std::endl;
    }
}

auto Output::foutResultsReacTrans(std::string property)-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> reactionSymbols        = pimpl->api.reactionSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TPpairs       = pimpl->api.TPpairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsReac   = pimpl->api.resultsReac();
    unsigned tp = TPpairs.size();

    if (pimpl->api.outputSettings().isFixed) pimpl->fThermoProperties << std::fixed;


    for (unsigned j=0; j<TPpairs.size(); j++)
    {
        pimpl->fThermoProperties << std::setprecision(digits.at("temperature"));
        pimpl->fThermoProperties << TPpairs[j][0];
        pimpl->fThermoProperties << std::setprecision(digits.at("pressure"));
        pimpl->fThermoProperties << s << TPpairs[j][1];

        for (unsigned i=0; i<reactionSymbols .size(); i++)
        {

            for (unsigned k=0; k<resultsReac[i].size(); k++)
            {
                if (properties.at(k+1) == property)
                {
                    pimpl->fThermoProperties << std::setprecision(digits.at(properties.at(k+1)));
                    pimpl->fThermoProperties << s << resultsReac[(tp*i)+(j)][k].val;
                }
            }
        }
        pimpl->fThermoProperties << std::endl;
    }
}

}
