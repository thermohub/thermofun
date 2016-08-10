#include "TPcalculationsAPI.h"

namespace TCorrPT {

struct TPcalcualationsAPI::Impl
{
    /// The thermo instance
    Thermo thermo;

    OutputOptions outputOptions;

    map<std::string, int> thermoPropSubstNames =  { {"sm_gibbs_energy", 0},
                                              {"sm_enthalpy", 0},
                                              {"sm_entropy_f", 0},
                                              {"sm_heat_capacity_p", 0},
                                              {"sm_heat_capacity_v", 0},
                                              {"sm_volume", 0},
                                              {"sm_helmoltz_energy", 0},
                                              {"sm_internal_energy", 0}
                                            };
    map<std::string,std::string> thermoPropSubstUnits =  { {"temperature", "(C)"},
                                              {"pressure", "(bar)"},
                                              {"sm_gibbs_energy", "(J/mol)"},
                                              {"sm_enthalpy", "(J/mol)"},
                                              {"sm_entropy_f", "(J/mol)"},
                                              {"sm_heat_capacity_p", "(J/mol*K)"},
                                              {"sm_heat_capacity_v", "(J/mol*K)"},
                                              {"sm_volume", "(J/bar)"},
                                              {"sm_helmoltz_energy", "(J/mol)"},
                                              {"sm_internal_energy", "(J/mol)"}
                                            };

     map<int, std::string> propNamesToExport;
    // map for properties solvent

    std::vector<std::vector<double>> TP_pairs;
    std::vector<string> substanceSymbols;
    std::string header = "";


//    Impl()
//    {}

    Impl(const Thermo& thermo)
    : thermo(thermo)
    {}

public:
//    map<std::string, int> getThermoPropSubstNames() const;
//    void setThermoPropSubstNames(const map<std::string, int> &value);
};

auto TPcalcualationsAPI::thermoPropSubstNames() -> map<std::string, int>
{
    return pimpl->thermoPropSubstNames;
}

auto TPcalcualationsAPI::setThermoPropSubstNames(const map<std::string, int> &value) -> void
{
    pimpl->thermoPropSubstNames = value;
}

auto TPcalcualationsAPI::thermoPropSubstUnits() -> map<std::string, std::string>
{
    return pimpl->thermoPropSubstUnits;
}

auto TPcalcualationsAPI::setThermoPropSubstUnits(const map<std::string, std::string> &value) -> void
{
    pimpl->thermoPropSubstUnits = value;
}


TPcalcualationsAPI::TPcalcualationsAPI(const Thermo& thermo)
    : pimpl(new Impl(thermo))
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
        header = header + "Substance" + s + "T" + units.at("temperature") + s + "P" + units.at("pressure") /*+ s*/;
        for(it_t it = PropertiesNames.begin(); it != PropertiesNames.end(); it++)
        {   header = header + s + it->second + units.at(it->second); }
    } else
    {
        header = header + "T" + units.at("temperature") + s + "P" + units.at("pressure")/* + s*/;
        for(it_t it = PropertiesNames.begin(); it != PropertiesNames.end(); it++)
        {   header = header + s + it->second + units.at(it->second); }

    }

    pimpl->header = header;
}

auto TPcalcualationsAPI::calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                double T, double P) -> void
{
    pimpl->substanceSymbols = substanceSymbols;
    setThermoPropSubstNames(thermoProperties);
    setTP_pairs(T, P);
    setHeader(pimpl->substanceSymbols, pimpl->propNamesToExport, pimpl->thermoPropSubstUnits );
}

}
