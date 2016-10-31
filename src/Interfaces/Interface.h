#ifndef INTERFACE_H
#define INTERFACE_H

#include "Thermo.h"
#include "Output.h"

namespace TCorrPT {

class Interface
{
public:
    ///
    /// \brief Interface constructor
    /// \param database
    ///
    Interface(const Database &database);

    auto setSubstanceNames (const std::vector<string> &substNames) -> void;
    auto setSubstanceName  (const std::string &substName) -> void;
    //    auto setPropertyNames (const map<std::string, std::string> &propNames) -> void;
    auto setPropertyNames (const std::vector<string> &propNames) -> void;
    auto setPropertyName  (const std::string &propName) -> void;
    auto setPropertyUnits (const std::map<const std::string, std::string> &propUnits)-> void;
    auto setPropertyNamesUnits (const std::vector<string> &propNames, const std::vector<string> &propUnits)-> void;
    auto setPropertyDigits (const std::map<const std::string, int> &propDigits)-> void;
    auto setPropertyNamesDigits (const std::vector<string> &propNames, const std::vector<int> &propDigits)-> void;
    auto setPropertyNameUnit (const std::string &propName, const std::string &propUnit)-> void;
    auto setPropertyNameDigit (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;
    auto setPropertyNameUnitDigit (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;

    auto setTP_pairs (const double &T, const double &P) -> void;
    auto setTP_pairs (const double &Tmin, const double &Tmax, const double &Tstep,
                      const double &Pmin, const double &Pmax, const double &Pstep) -> void;
    auto setTP_pairs (const std::vector<std::vector<double>> &TP_pairs) -> void;

    // claculate functions
    auto thermoCalculate( ) -> Output;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto setHeader(std::vector<string> substanceSymbols, map<int, string> thermoProperties, map<string, string> units ) -> void;


};

/**
 * @brief The OutputOptions struct holds the options for ouptuting results
 */
struct OutputOptions
{
    /// using fixed-point notation: the value is represented with exactly as many digits in the decimal part as specified by the precision field
    bool isFixed = false;

    /// write values in scientific notation.
    bool isScientific = false;

    /// csv file separator
    std::string separator = ",";

    /// file name/path
    std::string fileName = "tpresults.csv";
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
