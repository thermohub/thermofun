#ifndef OUTPUT
#define OUTPUT

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "../Common/ThermoScalar.hpp"

namespace ThermoFun {

class ThermoBatch;

/// outputs temperature in C and pressure in bar
class Output
{
public:
    ///
    /// \brief Output constructor
    /// \param batch object of ThermoBatch calculation used for output
    ///
    Output(const ThermoBatch& batch);

    /**
     * @brief toCSV write results to CSV file
     * with substances/reactions on rows and proeprties on columns
     * @param filename
     */
    auto toCSV                  (std::string filename) -> void;

    /**
     * @brief toCSVTransposed write results to CSV file in a special transposed format.
     * with substances/reactions on columns and proeprties on rows
     * @param filename
     */
    auto toCSVTransposed        (std::string filename) -> void;

    /**
     * @brief toCSVPropertyGrid write results to CSV file in a property grid format
     * one file will be created for each property
     * if loopTemperatureThenPressure in BatchPreferences is True the grid will have T in horizontal and P in vertical
     * if loopTemperatureThenPressure in BatchPreferences is False the grid will have P in horizontal and T in vertical
     * @param filename will be modified to include the property name
     */
    auto toCSVPropertyGrid      (std::string filename) -> void;

    /**
     * @brief toDouble
     * @return property value
     */
    auto toDouble               () -> double;

    /**
     * @brief toThermoScalar
     * @return property in ThermoScalar type
     */
    auto toThermoScalar         () -> Reaktoro_::ThermoScalar;

    /**
     * @brief to2DVectorDouble
     * @return vector of calculated properties
     */
    auto to2DVectorDouble       () -> std::vector<std::vector<double>>;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto CSVHeader            () -> std::string;
    auto CSVHeaderTransposed  () -> std::string;
    auto CSVHeaderPropertyGrid() -> std::string;
    auto foutResults          () -> void;
    auto foutResultsTransposed() -> void;
    auto foutPropertyGrid     (const std::string &property, const size_t &index_property) -> void;
};

}

#endif // OUTPUT

