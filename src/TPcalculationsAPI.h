#ifndef TPCALCULATIONSAPI
#define TPCALCULATIONSAPI
#include "Thermo.h"

namespace ThermoFun {

//map<std::string,std::string> thermoPropDefaultUnits = {{"temperature", "C"},
//                                          {"pressure", "bar"},
//                                          {"gibbs_energy", "J/mol"},
//                                          {"enthalpy", "J/mol"},
//                                          {"entropy", "J/mol"},
//                                          {"heat_capacity_cp", "J/mol*K"},
//                                          {"heat_capacity_cv", "J/mol*K"},
//                                          {"volume", "J/bar"},
//                                          {"helmholtz_energy", "J/mol"},
//                                          {"internal_energy", "J/mol"}
//                                        };

//map<std::string, int> thermoPropDefaultPrecision = {{"temperature", 0 },
//                                          {"pressure", 0 },
//                                          {"gibbs_energy", 0},
//                                          {"enthalpy", 0},
//                                          {"entropy", 0},
//                                          {"heat_capacity_cp", 0 },
//                                          {"heat_capacity_cv", 0 },
//                                          {"volume", 0 },
//                                          {"helmholtz_energy", 0 },
//                                          {"internal_energy", 0 }
//                                        };

/**
 * @brief The OutputOptions struct holds the options for ouptuting results
 */
struct OutputOptions
{
    /// using fixed-point notation: the value is represented with exactly as many digits in the decimal part as specified by the precision field
    bool fixed = false;

    /// write values in scientific notation.
    bool scientific = false;

    /// the maximum number of digits to be written
    unsigned precision = 0;

    /// csv file separator
    std::string separator = ",";

    /// file name/path
    std::string fileName = "xtpresults.csv";
};

/**
 * @brief The TPcalcualationsAPI class handels the interface between the ThermoFun library and coupled code for calculating the thermodynamic porperties
 * of substances at various T and P and exporting them in a csv file or having dirrect acces to a vector of results
 */
class TPcalcualationsAPI
{

public:
    /// Construct a TPcalcualationsAPI instance with given Database instance
    /*explicit*/ TPcalcualationsAPI(const Database& database);

    /**
     * @brief calculateThermoProperties calculates the selected thermodynamic properties for selected substances at T and P
     * @param substanceSymbols vector of substances
     * @param thermoProperties vector of selected properties
     * @param T temperature in deg C
     * @param P pressure ib bar
     */
    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double T, double P) -> void;

    /**
     * @brief calculateThermoProperties calculates the selected thermodynamic properties for selected substances for T and P interval
     * @param substanceSymbols vector of substances
     * @param thermoProperties vector of selected properties
     * @param Tmin staring temperature in deg C
     * @param Tmax end temperature in deg C
     * @param Tstep tempperature step
     * @param Pmin staring pressure in bar
     * @param Pmax end pressure in bar
     * @param Pstep pressure step
     */
    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> void;

    /**
     * @brief calculateThermoProperties calculateThermoProperties calculates the selected thermodynamic properties for selected substances for T and P list
     * @param substanceSymbols vector of substances
     * @param thermoProperties vector of selected properties
     * @param tp_pairs list of temperatures in C and pressures in bar
     */
    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    std::vector<std::vector<double> > tp_pairs) -> void;

    /**
     * @brief thermoPropSubstNames
     * @return a map with the selected themrodynamic properties of substances to output
     */
    auto thermoPropSubstNames() -> map<std::string, int>;

    /**
     * @brief thermoPropSubstUnits
     * @return a map with the units of the themrodynamic proeprties
     */
    auto thermoPropSubstUnits() -> map<std::string, std::string>;

    /**
     * @brief results
     * @return a vector of results [[substances*TP-pairs]*properties]
     */
    auto results () -> std::vector<std::vector<double>>;

    auto outputOptions() -> OutputOptions;

    auto setOutputOptions(const OutputOptions &value) -> void;

    auto thermoPrecision() -> map<std::string, int>;

    auto setThermoPrecision(const map<std::string, int> &value) -> void;


private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto setThermoPropSubstNames(const map<std::string, int> &value) -> void;
    auto setThermoPropSubstUnits(const map<std::string, std::string> &value) -> void;
    auto setThermoPropSubstNames (std::vector<string> themroPropertiesNames) -> void;
    auto setTP_pairs (double T, double P) -> void;
    auto setTP_pairs (double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> void;
    /// Set the titles of the chemical quantities using a formatted string.
    auto setHeader(std::vector<string> substanceSymbols, map<int, string> thermoProperties, map<string, string> units ) -> void;
    auto calculate( ) -> std::vector<std::vector<double>>;
    auto selectedPropResults ( ThermoPropertiesSubstance tps ) -> std::vector<double>;
    auto outResults( ) -> void;

};

}

#endif // TPCALCULATIONSAPI

