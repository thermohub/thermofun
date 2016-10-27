#include "TPcalculationsAPI.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace TCorrPT {

struct TPcalcualationsAPI::Impl
{
    /// The thermo instance
    Thermo thermo;
    OutputOptions outputOptions;

    map<std::string, int> thermoPropSubstNames = {{"gibbs_energy", 0},
                                              {"enthalpy", 0},
                                              {"entropy", 0},
                                              {"heat_capacity_cp", 0},
                                              {"heat_capacity_cv", 0},
                                              {"volume", 0},
                                              {"helmholtz_energy", 0},
                                              {"internal_energy", 0}
                                            };
    map<std::string,std::string> thermoPropSubstUnits = {{"temperature", "C"},
                                              {"pressure", "bar"},
                                              {"gibbs_energy", "J/mol"},
                                              {"enthalpy", "J/mol"},
                                              {"entropy", "J/mol"},
                                              {"heat_capacity_cp", "J/mol*K"},
                                              {"heat_capacity_cv", "J/mol*K"},
                                              {"volume", "J/bar"},
                                              {"helmholtz_energy", "J/mol"},
                                              {"internal_energy", "J/mol"}
                                            };

    map<std::string, int> thermoPropPrecision = {{"temperature", 0 },
                                              {"pressure", 0 },
                                              {"gibbs_energy", 0},
                                              {"enthalpy", 0},
                                              {"entropy", 0},
                                              {"heat_capacity_cp", 0 },
                                              {"heat_capacity_cv", 0 },
                                              {"volume", 0 },
                                              {"helmholtz_energy", 0 },
                                              {"internal_energy", 0 }
                                            };

    map<int, std::string>               propNamesToExport;
    std::vector<std::vector<double>>    TP_pairs;
    std::vector<string>                 substanceSymbols;
    std::string                         header = "";
    std::vector<std::vector<double>>    results;
    std::ofstream                       fThermoPropertiesSubstance;

    Impl(const Database& database)
    : thermo(Thermo(database))
    {}

public:
//    map<std::string, int> getThermoPropSubstNames() const;
//    void setThermoPropSubstNames(const map<std::string, int> &value);
};

auto TPcalcualationsAPI::results() -> std::vector<std::vector<double>>
{
    return pimpl->results;
}

auto TPcalcualationsAPI::thermoPropSubstNames() -> map<std::string, int>
{
    return pimpl->thermoPropSubstNames;
}

auto TPcalcualationsAPI::setThermoPropSubstNames(const map<std::string, int> &value) -> void
{
    pimpl->thermoPropSubstNames = value;
}

auto TPcalcualationsAPI::thermoPrecision() -> map<std::string, int>
{
    return pimpl->thermoPropPrecision;
}

auto TPcalcualationsAPI::setThermoPrecision(const map<std::string, int> &value) -> void
{
    pimpl->thermoPropPrecision = value;
}

auto TPcalcualationsAPI::thermoPropSubstUnits() -> map<std::string, std::string>
{
    return pimpl->thermoPropSubstUnits;
}

auto TPcalcualationsAPI::setThermoPropSubstUnits(const map<std::string, std::string> &value) -> void
{
    pimpl->thermoPropSubstUnits = value;
}

auto TPcalcualationsAPI::outputOptions() -> OutputOptions
{
    return pimpl->outputOptions;
}

auto TPcalcualationsAPI::setOutputOptions(const OutputOptions &value) -> void
{
    pimpl->outputOptions = value;
}



TPcalcualationsAPI::TPcalcualationsAPI(const Database &database)
    : pimpl(new Impl(database))
{}

auto TPcalcualationsAPI::setThermoPropSubstNames(std::vector<string> thermoPropertiesNames) -> void
{
    std::map<std::string, int>::iterator it;

    for (unsigned i = 0; i<thermoPropertiesNames.size(); i++)
    {
        it = pimpl->thermoPropSubstNames.find(thermoPropertiesNames[i]);
        if (it != pimpl->thermoPropSubstNames.end())
        {
            it->second = i+1;
            pimpl->propNamesToExport.insert ( std::pair<int,std::string>(i+1,thermoPropertiesNames[i]) );
        }
    }
}

auto TPcalcualationsAPI::setTP_pairs (double T, double P) -> void
{
    std::vector<double> one_pair = {T, P};
    pimpl->TP_pairs.push_back(one_pair);
}

auto TPcalcualationsAPI::setTP_pairs (double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> void
{
    double t = Tmin;
    double p = Pmin;
    do
    {
       do
        {
            setTP_pairs(t,p);
            p = p + Pstep;
        } while (p <= Pmax);
       t = t + Tstep;
    } while (t <= Tmax);

}

auto TPcalcualationsAPI::setHeader(std::vector<string> substanceSymbols, map<int, string> PropertiesNames, map<std::string,std::string> units ) -> void
{
    typedef std::map<int, std::string>::iterator it_t;
    const auto s = pimpl->outputOptions.separator;
    std::string header = "";

    if (substanceSymbols.size() > 1)
    {
        header = header + "Substance" + s + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")" /*+ s*/;
        for(it_t it = PropertiesNames.begin(); it != PropertiesNames.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }
    } else
    {
        header = header + "T" + "(" + units.at("temperature") + ")" + s + "P" + "(" + units.at("pressure") + ")"/* + s*/;
        for(it_t it = PropertiesNames.begin(); it != PropertiesNames.end(); it++)
        {   header = header + s + it->second + "(" + units.at(it->second) + ")"; }

    }

    pimpl->header = header;
}

auto TPcalcualationsAPI::selectedPropResults ( ThermoPropertiesSubstance tps ) -> std::vector<double>
{
    std::vector<double> properties;
    std::map<int, std::string>::iterator it;

    for(it = pimpl->propNamesToExport.begin(); it != pimpl->propNamesToExport.end(); it++)
    {
        if (it->second == "gibbs_energy")    properties.push_back(tps.gibbs_energy.val);
        if (it->second == "helmholtz_energy") properties.push_back(tps.helmholtz_energy.val);
        if (it->second == "internal_energy") properties.push_back(tps.internal_energy.val);
        if (it->second == "enthalpy")        properties.push_back(tps.enthalpy.val);
        if (it->second == "entropy")     properties.push_back(tps.entropy.val);
        if (it->second == "volume")          properties.push_back(tps.volume.val);
        if (it->second == "heat_capacity_cp") properties.push_back(tps.heat_capacity_cp.val);
        if (it->second == "heat_capacity_cv") properties.push_back(tps.heat_capacity_cv.val);
    }
    return properties;
}

auto TPcalcualationsAPI::calculate( ) -> std::vector<std::vector<double>>
{
    std::vector<std::vector<double>> results; unsigned int c = 0;
    results.resize(pimpl->substanceSymbols.size() * pimpl->TP_pairs.size());
    for (unsigned i=0; i<pimpl->substanceSymbols.size(); i++)
    {
        for (unsigned j=0; j<pimpl->TP_pairs.size(); j++)
        {
            results[c] = selectedPropResults(pimpl->thermo.thermoPropertiesSubstance(pimpl->TP_pairs[j][0], pimpl->TP_pairs[j][1], pimpl->substanceSymbols[i]));
            c++;
        }
    }
    return results;
}

auto TPcalcualationsAPI::outResults( ) -> void
{
    std::string s = pimpl->outputOptions.separator; unsigned int c = 0;
    pimpl->fThermoPropertiesSubstance.open( pimpl->outputOptions.fileName, ios::trunc );

    pimpl->fThermoPropertiesSubstance << pimpl->header << endl;

    for (unsigned i=0; i<pimpl->substanceSymbols.size(); i++)
    {
        for (unsigned j=0; j<pimpl->TP_pairs.size(); j++)
        {
            pimpl->fThermoPropertiesSubstance << std::setprecision(pimpl->outputOptions.precision) << std::fixed;
            pimpl->fThermoPropertiesSubstance << pimpl->substanceSymbols[i] << s << pimpl->TP_pairs[j][0] << s << pimpl->TP_pairs[j][1];

            if (!pimpl->outputOptions.fixed) pimpl->fThermoPropertiesSubstance << std::defaultfloat;
            for (unsigned k=0; k<pimpl->results[i].size(); k++)
            {
                pimpl->fThermoPropertiesSubstance << std::setprecision(pimpl->thermoPropPrecision.at(pimpl->propNamesToExport.at(k+1)));
                pimpl->fThermoPropertiesSubstance << s << pimpl->results[c][k];
            }
            c++;
            pimpl->fThermoPropertiesSubstance << endl;
        }
    }

    pimpl->fThermoPropertiesSubstance.close();
}

auto TPcalcualationsAPI::calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> void
{
    pimpl->substanceSymbols = substanceSymbols;
    setThermoPropSubstNames(thermoProperties);
    setTP_pairs(T, P);
    setHeader( pimpl->substanceSymbols, pimpl->propNamesToExport, pimpl->thermoPropSubstUnits );
    pimpl->results = calculate();
    outResults();
}

auto  TPcalcualationsAPI::calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                std::vector<std::vector<double>> tp_pairs) -> void
{
    pimpl->substanceSymbols = substanceSymbols;
    setThermoPropSubstNames(thermoProperties);
    pimpl->TP_pairs = tp_pairs;
    setHeader( pimpl->substanceSymbols, pimpl->propNamesToExport, pimpl->thermoPropSubstUnits );
    pimpl->results = calculate();
    outResults();
}

}
