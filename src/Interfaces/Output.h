#ifndef OUTPUT
#define OUTPUT

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace ThermoFun {

class Interface;
class ThermoScalar;

class Output
{
public:
    ///
    /// \brief Output constructor
    /// \param interface
    ///
    Output(const Interface & interface);

    auto toCSV              (std::string filename) -> void;
    auto toCSVtransposed    (std::string filename, std::string propertyname) -> void;
    auto toDouble           () -> double;
    auto toThermoScalar     () -> ThermoScalar;
    auto to2DVectorDouble   () -> std::vector<std::vector<double>>;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto CSVHeader            () -> std::string;
    auto CSVSolventHeader     () -> std::string;
    auto CSVHeaderTransposed  () -> std::string;
    auto foutResultsSubst     () -> void;
    auto foutResultsReac      () -> void;
    auto foutResultsSolv      () -> void;
    auto foutResultsReacTrans (std::string property) -> void;
    auto foutResultsSubstTrans(std::string property) -> void;
};

}

#endif // OUTPUT

