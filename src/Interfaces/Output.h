#ifndef OUTPUT
#define OUTPUT

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace ThermoFun {

class Interface;
class ThermoScalar;

/// outputs temperature in C and pressure in bar
class Output
{
public:
    ///
    /// \brief Output constructor
    /// \param interface
    ///
    Output(const Interface & interface);

    auto toCSV                  (std::string filename) -> void;
    auto toCSVTransposed        (std::string filename) -> void;
    auto toDouble               () -> double;
    auto toThermoScalar         () -> ThermoScalar;
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

