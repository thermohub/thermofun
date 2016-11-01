#ifndef INTERFACE_H
#define INTERFACE_H

#include "Thermo.h"
#include "Output.h"

namespace TCorrPT {

/**
 * @brief The OutputOptions struct holds the options for ouptuting results
 */
struct OutputSettings
{
    /// using fixed-point notation: the value is represented with exactly as many digits in the decimal part as specified by the precision field
    bool isFixed = true;

    /// write values in scientific notation.
    bool isScientific = false;

    /// csv file separator
    std::string separator = ",";

    /// file name/path
    std::string fileName = "tpresults.csv";
};

class Interface
{
    friend class Output;
public:
    ///
    /// \brief Interface constructor
    /// \param database
    ///
    Interface(const Database &database);

    auto addSubstanceSymbols      (const std::vector<string> &substSymbols) -> void;
    auto addSubstanceSymbol       (const std::string &substSymbol) -> void;
    auto addPropertyNames         (const std::vector<string> &propNames) -> void;
    auto addPropertyName          (const std::string &propName) -> void;
    auto addPropertyUnits         (const std::map<const std::string, std::string> &propUnits)-> void;
    auto addPropertyNamesUnits    (const std::vector<string> &propNames, const std::vector<string> &propUnits)-> void;
    auto addPropertyDigits        (const std::map<const std::string, int> &propDigits)-> void;
    auto addPropertyNamesDigits   (const std::vector<string> &propNames, const std::vector<int> &propDigits)-> void;
    auto addPropertyNameUnit      (const std::string &propName, const std::string &propUnit)-> void;
    auto addPropertyNameDigit     (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;
    auto addPropertyNameUnitDigit (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;

    auto addTP_pairs (const double &T, const double &P) -> void;
    auto addTP_pairs (const double &Tmin, const double &Tmax, const double &Tstep,
                      const double &Pmin, const double &Pmax, const double &Pstep) -> void;
    auto addTP_pairs (const std::vector<std::vector<double>> &TP_pairs) -> void;

    auto setFoutputSettings(const OutputSettings &value) -> void;

    // claculate functions
    auto thermoCalculate() -> Output;
    auto thermoCalculate(const std::string substSymbol, const double T, const double P, const std::string propName) -> Output;
    auto thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double T, double P) -> Output;
    auto thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                         double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output;
    auto thermoCalculate(std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                         std::vector<std::vector<double> > tp_pairs) -> Output;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto calculateResults() -> void;

    auto selectResults(ThermoPropertiesSubstance tps) -> std::vector<double>;

    auto substanceSymbols() -> const std::vector<string>;
    auto TP_pairs()         -> const std::vector<std::vector<double>>;
    auto propNames()        -> const map<int, std::string>;
    auto propUnits()        -> const std::map<const std::string, std::string>;
    auto propDigits()       -> const std::map<const std::string, int>;
    auto results()          -> const std::vector<std::vector<double>>;

    auto foutputSettings()  -> const OutputSettings;

};

const std::map<const std::string, const std::string> defaultPropertyNames =
{

    {"gibbs_energy",                   "substance"     },
    {"enthalpy",                       "substance"     },
    {"entropy",                        "substance"     },
    {"heat_capacity_cp",               "substance"     },
    {"heat_capacity_cv",               "substance"     },
    {"volume",                         "substance"     },
    {"helmholtz_energy",               "substance"     },
    {"internal_energy",                "substance"     }

};

const std::map<const std::string, std::string> defaultPropertyUnits =
{

    {"temperature",                    "C"             },
    {"pressure",                       "bar"           },

    {"gibbs_energy",                   "J/mol"         },
    {"enthalpy",                       "J/mol"         },
    {"entropy",                        "J/mol"         },
    {"heat_capacity_cp",               "J/mol*K"       },
    {"heat_capacity_cv",               "J/mol*K"       },
    {"volume", "J/bar"},
    {"helmholtz_energy",               "J/mol"         },
    {"internal_energy",                "J/mol"         },

    {"reaction_gibbs_energy",          "J/mol"         },
    {"reaction_helmholtz_energy",      "J/mol"         },
    {"reaction_internal_energy",       "J/mol"         },
    {"reaction_enthalpy",              "J/mol"         },
    {"reaction_entropy",               "J/mol"         },
    {"reaction_volume",                "J/bar"         },
    {"reaction_heat_capacity_cp",      "J/mol*K"       },
    {"reaction_heat_capacity_cv",      "J/mol*K"       }

};

const std::map<const std::string, int> defaultPropertyDigits =
{

    {"temperature",                    0               },
    {"pressure",                       0               },

    {"gibbs_energy",                   0               },
    {"enthalpy",                       0               },
    {"entropy",                        0               },
    {"heat_capacity_cp",               0               },
    {"heat_capacity_cv",               0               },
    {"volume",                         0               },
    {"helmholtz_energy",               0               },
    {"internal_energy",                0               },

    {"reaction_gibbs_energy",          0               },
    {"reaction_helmholtz_energy",      0               },
    {"reaction_internal_energy",       0               },
    {"reaction_enthalpy",              0               },
    {"reaction_entropy",               0               },
    {"reaction_volume",                0               },
    {"reaction_heat_capacity_cp",      0               },
    {"reaction_heat_capacity_cv",      0               },

};


}

#endif // INTERFACE_H
