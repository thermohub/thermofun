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
    /// \param interface
    ///
    Output(const ThermoBatch & batch);

    auto toCSV                  (std::string filename) -> void;
    auto toCSVTransposed        (std::string filename) -> void;
    auto toDouble               () -> double;
    auto toThermoScalar         () -> Reaktoro_::ThermoScalar;
    auto to2DVectorDouble       () -> std::vector<std::vector<double>>;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto CSVHeader            () -> std::string;
    auto CSVHeaderTransposed  () -> std::string;
    auto foutResults          () -> void;
    auto foutResultsTransposed() -> void;
};

}

#endif // OUTPUT

