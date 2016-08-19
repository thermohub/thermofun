#ifndef TPCALCULATIONSAPI
#define TPCALCULATIONSAPI
#include "Thermo.h"

namespace TCorrPT {

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
    unsigned precision = 8;

    /// csv file separator
    std::string separator = ",";

    /// file name/path
    std::string fileName = "tpresults.csv";
};

/**
 * @brief The TPcalcualationsAPI class handels the interface between the TCorrPT library and coupled code for calculating the thermodynamic porperties
 * of substances at various T and P and exporting them in a csv file or having dirrect acces to a vector of results
 */
class TPcalcualationsAPI
{

public:
    /// Construct a TPcalcualationsAPI instance with given Thermo instance
    explicit TPcalcualationsAPI(const Thermo& thermo);

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
                                    vector<double> tp_pairs[2]) -> void;

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

