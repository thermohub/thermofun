#ifndef OUTPUT
#define OUTPUT

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace TCorrPT {

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

    auto toCSV()            -> void;
    auto toDouble()         -> double;
    auto toThermoScalar()   -> ThermoScalar;
    auto to2DVectorDouble() -> std::vector<std::vector<double>>;


private:
    struct Impl;
    std::shared_ptr<Impl> pimpl;

    auto CSVHeader()-> std::string;
    auto foutResults()-> void;


};

}

#endif // OUTPUT

