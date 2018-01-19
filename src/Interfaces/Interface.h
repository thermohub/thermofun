#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../Common/ThermoScalar.hpp"

namespace ThermoFun {

struct Database;
struct Output;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;

/**
 * @brief The OutputOptions struct holds the options for outputing results
 */
struct OutputSettings
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
    bool outSolventProp = false;

    bool calcSubstFromReact = false;

    bool calcReactFromSubst = false;

    bool loopOverTPpairsFirst = false;
};

struct CalculationSettings
{
    bool dependentSubstancesFromReactions = false;

    bool reactionsFromSubstances = false;
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
class Interface
{
    friend class Output;
public:
    ///
    /// \brief Interface constructor
    /// \param database
    ///
    Interface(const Database &database);

    auto setUnits                   (const std::map<std::string, std::string> &propUnits)-> void;
    auto setDigits                  (const std::map<std::string, int> &propDigits)-> void;
    auto setPropertiesUnits         (const std::vector<std::string> &propNames, const std::vector<std::string> &propUnits)-> void;
    auto setPropertiesDigits        (const std::vector<std::string> &propNames, const std::vector<int> &propDigits)-> void;
    auto setPropertyUnit            (const std::string &propName, const std::string &propUnit)-> void;
    auto setPropertyDigit           (const std::string &propName, const int &propDigit)-> void;
    auto setPropertyUnitDigit       (const std::string &propName, const std::string &propUnit, const int &propDigit)-> void;

    // claculate functions substances
    auto thermoPropertiesSubstance   (const std::string substSymbol, const double TC, const double Pbar, const std::string propName) -> Output;
    auto thermoPropertiesSubstance   (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                                      double TC, double Pbar) -> Output;
    auto thermoPropertiesSubstance   (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                                      std::array<double, 3> aT, std::array<double, 3> aP) -> Output;
    auto thermoPropertiesSubstance   (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                                      std::vector<std::vector<double> > tpPairs) -> Output;

    // claculate functions reactions
    auto thermoPropertiesReaction    (const std::string reacSymbol, const double TC, const double Pbar, const std::string propName) -> Output;
    auto thermoPropertiesReaction    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                                      double TC, double Pbar) -> Output;
    auto thermoPropertiesReaction    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                                      std::array<double, 3> aT, std::array<double, 3> aP) -> Output;
    auto thermoPropertiesReaction    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                                      std::vector<std::vector<double> > tpPairs) -> Output;

    auto setOutputSettings      (const OutputSettings &value) -> void;
    auto setSolventSymbol       (const std::string solventSymbol) ->void;

    // claculate functions substances
    auto calculateProperties    (const std::string substSymbol, const double TC, const double Pbar, const std::string propName) -> Output;
    auto calculateProperties    (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                                    double TC, double Pbar) -> Output;
    auto calculateProperties    (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                             double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output;
    auto calculateProperties    (std::vector<std::string> substanceSymbols, std::vector<std::string> thermoProperties,
                             std::vector<std::vector<double> > tpPairs) -> Output;

    // claculate functions reactions
    auto calcPropReactions    (const std::string reacSymbol, const double TC, const double Pbar, const std::string propName) -> Output;
    auto calcPropReactions    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                                    double TC, double Pbar) -> Output;
    auto calcPropReactions    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                             double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> Output;
    auto calcPropReactions    (std::vector<std::string> reactionSymbols, std::vector<std::string> thermoProperties,
                               std::vector<std::vector<double> > tpPairs) -> Output;

    // output order
    auto providedResultsSubstance_colTProwSym(std::vector<std::vector<ThermoPropertiesSubstance>> vTps ) -> void;

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

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    // Substances
    auto addSubstances                  (const std::vector<std::string> &substSymbols) -> void;
    auto addSubstance                   (const std::string &substSymbol) -> void;
    // Reactions
    auto addReactions                   (const std::vector<std::string> &reacSymbols) -> void;
    auto addReaction                    (const std::string &reacSymbol) -> void;


    auto addProperties                  (const std::vector<std::string> &propNames) -> void;
    auto addProperty                    (const std::string &propName) -> void;


    // Substances
    auto selectResultsSubst     (ThermoPropertiesSubstance tps) -> std::vector<Reaktoro_::ThermoScalar>;
    auto calculateResultsSubst  () -> void;
    auto substanceSymbols       () -> const std::vector<std::string>;
    auto resultsSubst           () -> const std::vector<std::vector<Reaktoro_::ThermoScalar>>;
    auto calculateSolventProp   (int tpNdx) -> void;

    auto selectPropertiesSolvent (PropertiesSolvent ps, ElectroPropertiesSolvent eps) -> std::vector<Reaktoro_::ThermoScalar>;

    // Reactions
    auto selectResultsReac      (ThermoPropertiesReaction tpr) -> std::vector<Reaktoro_::ThermoScalar>;
    auto calculateResultsReac   () -> void;
    auto reactionSymbols        () -> const std::vector<std::string>;
    auto resultsReac            () -> const std::vector<std::vector<Reaktoro_::ThermoScalar>>;

    auto TPpairs                () -> const std::vector<std::vector<double>>;
    auto propNames              () -> const std::vector<std::string>;
    auto propUnits              () -> const std::map<std::string, std::string>;
    auto propDigits             () -> const std::map<std::string, int>;
    auto outputSettings         () -> const OutputSettings;

    auto solventProp            () -> const std::vector<Reaktoro_::ThermoScalar>;
    auto solventSymbol          () -> const std::string;
    auto solventPropUnits       () -> const std::map<std::string, std::string>;
    auto solventPropDigits      () -> const std::map<std::string, int>;
    auto solventPropNames       () -> const std::vector<std::string>;

    auto clearSubstances        () -> void;
    auto clearReactions         () -> void;
    auto clearProperties        () -> void;
    auto clearTPpairs           () -> void;
    auto clearSolventProperties () -> void;

//    auto clearAll() -> void;
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
    {"logKr",                          ""              },
    {"lnK0",                           ""              }
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
    {"lnK0",                           0               }
};


}

#endif // INTERFACE_H
