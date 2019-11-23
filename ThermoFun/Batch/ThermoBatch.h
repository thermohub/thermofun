#ifndef INTERFACE_H
#define INTERFACE_H

// C++ includes
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../Common/ThermoScalar.hpp"
#include "OutputBatch.h"

namespace ThermoFun {

class Database;
class ThermoEngine;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;

typedef std::vector<std::string> vstr;
typedef std::vector<std::vector<double>> vvd;
typedef std::vector<Reaktoro_::ThermoScalar> vThScalar;
typedef std::vector<vThScalar> vvThScalar;
typedef std::vector<ThermoPropertiesSubstance> vtps;
typedef std::vector<ThermoPropertiesReaction>  vtpr;

/**
 * @brief The OutputOptions struct holds the options for outputing results
 */
struct BatchPreferences
{
    /// using fixed-point notation: the value is represented with exactly as many digits in the decimal part as specified by the precision field
    bool isFixed = true;
    bool isFloating = false;

    /// write values in scientific notation.
    bool isScientific = false;

    /// csv file separator
    std::string separator = ",";

    /// file name/path
    std::string fileName = "tpresults.csv";

    /// solvent prop file name/path
    std::string solventFileName = "tpSolventResults.csv";

    /// write solvent properties
    bool outputSolventProperties = false;

    bool substancePropertiesFromReaction = false;

    bool reactionPropertiesFromReactants = false;

    bool loopOverTPpairsFirst = true;

    /**
     * @brief loopTemperatureThenPressure
     * @value True generate T-P pairs by first incrementing temperature
     * @value False generate T-P pairs by first incrementing pressure
     */
    bool loopTemperatureThenPressure = true;
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
/// The input temperature and pressure should be in celsius and bar
///
class ThermoBatch
{
    friend class Output;
public:
    ///
    /// \brief Interface constructor
    /// \param database
    ///
    ThermoBatch(const Database &database);

    ///
    /// \brief Interface
    /// \param filename
    ///
    ThermoBatch(std::string filename);

    ///
    /// \brief ThermoBatch
    /// \param engine
    ///
    ThermoBatch(const ThermoEngine &engine);

    auto setUnits                   (const std::map<std::string, std::string> &units)-> void;
    auto setDigits                  (const std::map<std::string, int> &digits)-> void;
    auto setPropertiesUnits         (const vstr &properties, const vstr &units)-> void;
    auto setPropertiesDigits        (const vstr &properties, const std::vector<int> &digits)-> void;
    auto setPropertyUnit            (const std::string &property, const std::string &unit)-> void;
    auto setPropertyDigit           (const std::string &property, const int &digit)-> void;
    auto setPropertyUnitDigit       (const std::string &property, const std::string &unit, const int &digit)-> void;
    auto setTemperatureIncrement    (const double& Tmin, const double& Tmax, const double& Tstep) -> void;
    auto setPressureIncrement       (const double& Pmin, const double& Pmax, const double& Pstep) -> void;

    // claculate functions substances
    auto thermoPropertiesSubstance  (double T, double P, std::string symbol,  std::string property) -> Output;
    auto thermoPropertiesSubstance  (double T, double P, vstr        symbols, vstr        properties) -> Output;
    auto thermoPropertiesSubstance  (vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesSubstance  (vvd tpPairs, vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesSubstance  (vvd tpPairs, vstr symbols, vstr properties, vtps vTps) -> Output;
    auto thermoPropertiesSubstance  (std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesSubstance  (std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties, vtps vTps) -> Output;

    // claculate functions reactions
    auto thermoPropertiesReaction   (double T, double P, std::string symbol,  std::string property) -> Output;
    auto thermoPropertiesReaction   (double T, double P, vstr        symbols, vstr        properties) -> Output;
    auto thermoPropertiesReaction   (vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesReaction   (vvd tpPairs, vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesReaction   (vvd tpPairs, vstr symbols, vstr properties, vtpr vTpr) -> Output;
    auto thermoPropertiesReaction   (std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties) -> Output;
    auto thermoPropertiesReaction   (std::vector<double> temperatures, std::vector<double> pressures, vstr symbols, vstr properties, vtpr vTpr) -> Output;

    auto setBatchPreferences        (const BatchPreferences &value) -> void;
    auto setSolventSymbol           (const std::string solventSymbol) ->void;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    // Substances
    auto symbols        () -> const vstr;
    auto results        () -> const vvThScalar;

    auto TPpairs        () -> const vvd;
    auto Temperatures   () -> const std::vector <double>;
    auto Pressures      () -> const std::vector <double>;
    auto properties     () -> const vstr;
    auto units          () -> const std::map<std::string, std::string>;
    auto digits         () -> const std::map<std::string, int>;
    auto outputSettings () -> const BatchPreferences;
};

const std::map<std::string, const std::string> defaultPropertyNames =
{
        // Substance propeties
    {"gibbs_energy",                   "substance"     },
    {"enthalpy",                       "substance"     },
    {"entropy",                        "substance"     },
    {"heat_capacity_cp",               "substance"     },
    {"heat_capacity_cv",               "substance"     },
    {"volume",                         "substance"     },
    {"helmholtz_energy",               "substance"     },
    {"internal_energy",                "substance"     },
        // Reaction propeties
    {"reaction_gibbs_energy",          "reaction"      },
    {"reaction_helmholtz_energy",      "reaction"      },
    {"reaction_internal_energy",       "reaction"      },
    {"reaction_enthalpy",              "reaction"      },
    {"reaction_entropy",               "reaction"      },
    {"reaction_volume",                "reaction"      },
    {"reaction_heat_capacity_cp",      "reaction"      },
    {"reaction_heat_capacity_cv",      "reaction"      },
    {"logKr",                          "reaction"      },
    {"lnKr",                           "reaction"      },
        // Solvent propeties
    {"density",                        "solvent"      },
    {"densityT",                       "solvent"      },
    {"densityP",                       "solvent"      },
    {"densityTT",                      "solvent"      },
    {"densityTP",                      "solvent"      },
    {"densityPP",                      "solvent"      },
    {"alpha",                          "solvent"      },
    {"beta",                           "solvent"      },
    {"alphaT",                         "solvent"      },

    {"epsilon",                        "solvent"      },
    {"epsilonT",                       "solvent"      },
    {"epsilonP",                       "solvent"      },
    {"epsilonTT",                      "solvent"      },
    {"epsilonTP",                      "solvent"      },
    {"epsilonPP",                      "solvent"      },
    {"bornZ",                          "solvent"      },
    {"bornY",                          "solvent"      },
    {"bornQ",                          "solvent"      },
    {"bornN",                          "solvent"      },
    {"bornU",                          "solvent"      },
    {"bornX",                          "solvent"      }
};

const std::map<std::string, std::string> defaultSolventPropertyUnits =
{
    {"density",                        "kg/m3"          },
    {"densityT",                       "(kg/m3)/K"      },
    {"densityP",                       "(kg/m3)/Pa"     },
    {"densityTT",                      "(kg/m3)/(K*K)"  },
    {"densityTP",                      "(kg/m3)/(K*Pa)" },
    {"densityPP",                      "(kg/m3)/(Pa*Pa)"},
    {"alpha",                          "1/K"            },
    {"beta",                           "1/Pa"           },
    {"alphaT",                         ""               },

    {"epsilon",                        ""               },
    {"epsilonT",                       ""               },
    {"epsilonP",                       ""               },
    {"epsilonTT",                      ""               },
    {"epsilonTP",                      ""               },
    {"epsilonPP",                      ""               },
    {"bornZ",                          ""               },
    {"bornY",                          "1/K"            },
    {"bornQ",                          "1/Pa"           },
    {"bornN",                          "1/Pa*Pa"        },
    {"bornU",                          "1/Pa*K"         },
    {"bornX",                          "1/K*K"          }
};

const std::map<std::string, int> defaultSolventPropertyDigits =
{
    {"density",                        0            },
    {"densityT",                       0            },
    {"densityP",                       0            },
    {"densityTT",                      0            },
    {"densityTP",                      0            },
    {"densityPP",                      0            },
    {"alpha",                          0            },
    {"beta",                           0            },
    {"alphaT",                         0            },

    {"epsilon",                        0            },
    {"epsilonT",                       0            },
    {"epsilonP",                       0            },
    {"epsilonTT",                      0            },
    {"epsilonTP",                      0            },
    {"epsilonPP",                      0            },
    {"bornZ",                          0            },
    {"bornY",                          0            },
    {"bornQ",                          0            },
    {"bornN",                          0            },
    {"bornU",                          0            },
    {"bornX",                          0            }
};

const std::map<std::string, std::string> defaultPropertyUnits =
{

    {"temperature",                    "K"             },
    {"pressure",                       "Pa"            },

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
    {"logKr",                          "1"             },
    {"lnKr",                           "1"             }
};

const std::map<std::string, int> defaultPropertyDigits =
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
    {"lnKr",                           0               }
};

const std::map<std::string, double> defaultTemperatureIncrement =
{
    {"Tmin", 25.},
    {"Tmax", 300.},
    {"Tstep", 25.}
};

const std::map<std::string, double> defaultPressureIncrement =
{
    {"Pmin", 0.},
    {"Pmax", 0.},
    {"Pstep", 0.}
};


}

#endif // INTERFACE_H
