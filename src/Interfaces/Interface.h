#ifndef INTERFACE_H
#define INTERFACE_H

#include "../Thermo.h"
#include "Output.h"

namespace ThermoFun {

/**
 * @brief The OutputOptions struct holds the options for outputing results
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

///
/// \brief The Interface class provides an interface to C++ codes coupled to ThermoFunk library for performing
/// calculations related to the standard state properties of substances and reactions at different temperatures
/// and pressures, and retriveve the results as CSV output file, 2D vector (of double or ThrmoScalar types), or
/// single value (of double or ThermoScalar types) for one property-substance-T-P calculation.
///
/// The class provides several functions for adding the symbol of the substance or reaction (or lists of symblos)
/// for which the calculations will be performed.
///
/// The class provides several functions for adding the temperature and pressure points at which the calculations
/// will be performed.
///
/// The class provides several functions for setting the unist in which the results are dysplayed and the
/// siginifincat digits (or precision) in which the values will be written in the output CSV file.
///
///
class Interface
{
    friend class Output;
public:
    ///
    /// \brief Interface constructor
    /// \param database
    ///
    Interface(const Database &database);

    // Substances
    auto addSubstances                  (const std::vector<string> &substSymbols) -> void;
    auto addSubstance                   (const std::string &substSymbol) -> void;
    // Reactions
    auto addReactions                   (const std::vector<string> &reacSymbols) -> void;
    auto addReaction                   (const std::string &reacSymbol) -> void;


    auto addProperties                  (const std::vector<string> &propNames) -> void;
    auto addProperty                    (const std::string &propName) -> void;
    auto addUnits                       (const std::map<const std::string, std::string> &propUnits)-> void;
    auto addDigits                      (const std::map<const std::string, int> &propDigits)-> void;
    auto addProperties_and_Units        (const std::vector<string> &propNames, const std::vector<string> &propUnits)-> void;
    auto addProperties_and_Digits       (const std::vector<string> &propNames, const std::vector<int> &propDigits)-> void;
    auto addProperty_and_Unit           (const std::string &propName, const std::string &propUnit)-> void;
    auto addProperty_and_Digit          (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;
    auto addProperty_and_Unit_and_Digit (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;

    auto addTPpair                      (const double &T, const double &P) -> void;
    auto addTPpairs                     (const double &Tmin, const double &Tmax, const double &Tstep,
                                         const double &Pmin, const double &Pmax, const double &Pstep) -> void;
    auto addTPpairs                     (const std::vector<std::vector<double>> &tpPairs) -> void;

    auto setOutputSettings              (const OutputSettings &value) -> void;
    auto setSolventSymbolForAqSubst     (const std::string solvent_symbol) ->void;

    // claculate functions substances
    auto calculateProperties    () -> Output;
    auto calculateProperties    (const std::string substSymbol, const double T, const double P, const std::string propName) -> Output;
    auto calculateProperties    (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double T, double P) -> Output;
    auto calculateProperties    (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                             double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output;
    auto calculateProperties    (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                             std::vector<std::vector<double> > tpPairs) -> Output;

    // claculate functions reactions
    auto calcPropReactions    () -> Output;
    auto calcPropReactions    (const std::string reacSymbol, const double T, const double P, const std::string propName) -> Output;
    auto calcPropReactions    (std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                                    double T, double P) -> Output;
    auto calcPropReactions    (std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                             double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output;
    auto calcPropReactions    (std::vector<string> reactionSymbols, std::vector<string> thermoProperties,
                               std::vector<std::vector<double> > tpPairs) -> Output;

    ///
    /// \brief selectResultsSubst_vTpSym Selects the results from given a vector of calculated properites
    /// \param vTps Vector of calculated properties of the form [properties at T-P x substances]
    ///
    auto selectResultsSubst_vTpSym(std::vector<std::vector<ThermoPropertiesSubstance>> vTps ) -> void;

    ///
    /// \brief selectResultsReac_vTpSym Selects the results from given a vector of calculated properites
    /// \param vTpr Vector of calculated properties of the form [properties at T-P x substances]
    ///
    auto selectResultsReac_vTpSym(std::vector<std::vector<ThermoPropertiesReaction>> vTpr ) -> void;

    auto clearSubstances    () -> void;
    auto clearReactions     () -> void;
    auto clearProperties    () -> void;
    auto clearTPpairs       () -> void;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    // Substances
    auto selectResultsSubst     (ThermoPropertiesSubstance tps) -> std::vector<Reaktoro_::ThermoScalar>;
    auto calculateResultsSubst  () -> void;
    auto substanceSymbols       () -> const std::vector<string>;
    auto resultsSubst           () -> const std::vector<std::vector<Reaktoro_::ThermoScalar>>;

    // Reactions
    auto selectResultsReac     (ThermoPropertiesReaction tpr) -> std::vector<Reaktoro_::ThermoScalar>;
    auto calculateResultsReac  () -> void;
    auto reactionSymbols       () -> const std::vector<string>;
    auto resultsReac           () -> const std::vector<std::vector<Reaktoro_::ThermoScalar>>;

    auto TPpairs                () -> const std::vector<std::vector<double>>;
    auto propNames              () -> const map<int, std::string>;
    auto propUnits              () -> const std::map<const std::string, std::string>;
    auto propDigits             () -> const std::map<const std::string, int>;
    auto outputSettings         () -> const OutputSettings;
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
    {"internal_energy",                "substance"     },

    {"reaction_gibbs_energy",          "reaction"      },
    {"reaction_helmholtz_energy",      "reaction"      },
    {"reaction_internal_energy",       "reaction"      },
    {"reaction_enthalpy",              "reaction"      },
    {"reaction_entropy",               "reaction"      },
    {"reaction_volume",                "reaction"      },
    {"reaction_heat_capacity_cp",      "reaction"      },
    {"reaction_heat_capacity_cv",      "reaction"      },
    {"logKr",                          "reaction"      },
    {"lnK0",                           "reaction"      }

};

const std::map<const std::string, std::string> defaultPropertyUnits =
{

    {"temperature",                    "C"             },
    {"pressure",                       "bar"           },

    {"gibbs_energy",                   "J/mol"         },
    {"enthalpy",                       "J/mol"         },
    {"entropy",                        "J/K/mol"       },
    {"heat_capacity_cp",               "J/K/mol"       },
    {"heat_capacity_cv",               "J/K/mol"       },
    {"volume", "J/bar"},
    {"helmholtz_energy",               "J/mol"         },
    {"internal_energy",                "J/mol"         },

    {"reaction_gibbs_energy",          "J/mol"         },
    {"reaction_helmholtz_energy",      "J/mol"         },
    {"reaction_internal_energy",       "J/mol"         },
    {"reaction_enthalpy",              "J/mol"         },
    {"reaction_entropy",               "J/mol"         },
    {"reaction_volume",                "J/bar"         },
    {"reaction_heat_capacity_cp",      "J/K/mol"       },
    {"reaction_heat_capacity_cv",      "J/K/mol"       },
    {"logKr",                          ""              },
    {"lnK0",                           ""              }

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
    {"logKr",                          0               },
    {"lnK0",                           0               }

};


}

#endif // INTERFACE_H
