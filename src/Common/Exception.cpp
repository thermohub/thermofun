#include "Exception.h"

// C++ includes
#include <algorithm>
#include <sstream>

namespace ThermoFun {
namespace internal {
/// Creates the location string from the file name and line number.
/// The result of this function on the file `/home/user/gitThermoFun/ThermoFun/src/Substance.cpp`
/// will be `ThermoFun/src/Substance.cpp`.
/// @param file The full path to the file
/// @param line The line number
std::string location(const std::string& file, int line)
{
    //    std::string str = "ThermoFun/";
    //    auto pos = std::find_end(file.begin(), file.end(), str.begin(), str.end()) - file.begin();
    std::stringstream ss;
    ss << "..."<< file.substr(file.size() - 15) << ":" << line;
    return ss.str();
}

std::string message(const Exception& exception, const std::string& file, int line)
{
    std::string error = exception.error.str();
    std::string reason = exception.reason.str();
    std::string loc = location(exception.file, exception.line);
    unsigned length = std::max(error.size(), std::max(reason.size(), loc.size())) + 16;
    std::string bar(length, '*');
    std::stringstream message;
    message << std::endl;
    message << bar << std::endl;
    message << "*** Error: " << error << std::endl;
    message << "*** Reason: " << reason << std::endl;
    message << "*** Location: This error was encountered in " << loc << "." << std::endl;
    message << bar << std::endl;
    message << std::endl;
    return message.str();
}
}

auto setMessage(Reaktoro_::Status sta, std::string propertyOfsymbol, std::string calcMessage, std::string &message_ ) -> void
{
    if (sta == Reaktoro_::Status::notdefined)
        message_ += propertyOfsymbol + " not defined; ";
    else
        message_ = calcMessage;
}

auto errorMethodNotFound(std::string type, std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "The calculation method was not found.";
    exception.reason << "The calculation method defined for the " << type << " " << name << " is not implemented.";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto errorReactionNotDefined(std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "The reaction for the dependent substance is not defined.";
    exception.reason << "The reaction symbol for the dependent substance " << name << " is not defined.";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto errorSolventNotDefined(std::string type, std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "Solvent symbol not defiend";
    exception.reason << "The solvent symbol for " << name <<  " was not defined.";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto errorModelParameters(std::string type, std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "The parameters of the model are not correctly defiend";
    exception.reason << "The " << type <<  " parameters for " << name << " were not correctly defined. ";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto errorSameSymbol(std::string type, std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "Initializing Thermufun database.";
    exception.reason << "Duplicate symbol for " << type <<  " " << name << " was encountered! ";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto errorNotMatchingLevel(std::string type, std::string name, int line, std::string file) -> void
{
    Exception exception;
    exception.error << "Not matching level in the graph traversal";
    exception.reason << "For " << type <<  " " << name << " ";
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

} // namespace ThermoFun
