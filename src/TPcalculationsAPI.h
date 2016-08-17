#ifndef TPCALCULATIONSAPI
#define TPCALCULATIONSAPI
#include "Thermo.h"

namespace TCorrPT {

struct OutputOptions
{
    bool fixed = false;

    bool scientific = false;

    unsigned precision = 8;

    unsigned width = 15;

    std::string separator = ",";

    std::string fileName = "tpresults.csv";
};

//typedef std::pair<int, std::string> pair_t;
//const pair_t map_start_values[] = {
//    pair_t(-1, "sm_gibbs_energy"),
//    pair_t(-1, "sm_enthalpy"),
//    pair_t(-1, "sm_entropy_f"),
//    pair_t(-1, "sm_heat_capacity_p"),
//    pair_t(-1, "sm_heat_capacity_v"),
//    pair_t(-1, "sm_volume"),
//    pair_t(-1, "sm_helmoltz_energy"),
//    pair_t(-1, "sm_internal_energy")
//};


class TPcalcualationsAPI
{

public:
    /// Construct a Thermo instance with given Database instance
    explicit TPcalcualationsAPI(const Thermo& thermo);

    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double T, double P) -> void;
    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    double Tmin, double Tmax, double Tstep, double Pmin, double Pmax, double Pstep) -> void;
    auto calculateThermoProperties (std::vector<string> substanceSymbols, std::vector<string> thermoProperties,
                                    vector<double> tp_pairs[2]) -> void;

    auto thermoPropSubstNames() -> map<std::string, int>;

    auto thermoPropSubstUnits() -> map<std::string, std::string>;

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

