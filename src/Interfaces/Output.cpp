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
    std::string solventSymbol                       = pimpl->api.solventSymbol();

    const auto s = pimpl->api.outputSettings().separator;
    const auto outSol = pimpl->api.outputSettings().outSolventProp;
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

    if (outSol)
        header = header + s + "ρ_" + solventSymbol + "(" + "g/cm3" + ")" + s +  + "ε_" + solventSymbol + "(relative_permitivity)";

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

auto Output::foutResultsSubst()-> void
{
    const auto s = pimpl->api.outputSettings().separator; unsigned int c = 0;

    std::vector<std::string> substanceSymbols       = pimpl->api.substanceSymbols();
    std::map<int, std::string> properties           = pimpl->api.propNames();
    std::map<std::string, int> digits         = pimpl->api.propDigits();
    std::vector<std::vector<double>> TPpairs       = pimpl->api.TPpairs();
    std::vector<std::vector<Reaktoro_::ThermoScalar>> resultsSubst   = pimpl->api.resultsSubst();
    std::vector<Reaktoro_::ThermoScalar> solventProp   = pimpl->api.solventProp();
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
            if (outSol && i == 0)
            {
                pimpl->fThermoProperties << std::setprecision(0);
                pimpl->fThermoProperties << s << solventProp[j+j].val;
                pimpl->fThermoProperties << s << solventProp[j+j+1].val;
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
    std::vector<Reaktoro_::ThermoScalar> solventProp   = pimpl->api.solventProp();
    const auto outSol = pimpl->api.outputSettings().outSolventProp;

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
            if (outSol && i == 0)
            {
                pimpl->fThermoProperties << std::setprecision(0);
                pimpl->fThermoProperties << s << solventProp[j+j].val;
                pimpl->fThermoProperties << s << solventProp[j+j+1].val;
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
