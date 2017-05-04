#ifndef EXCEPTION_H
#define EXCEPTION_H

// C++ includes
#include <sstream>
#include <stdexcept>
#include <string>
#include "Common/ThermoScalar.hpp"
#include "ThermoProperties.h"

namespace ThermoFun {

/// Provides a convenient way to initialized an exception with helpful error messages.
struct Exception
{
    /// The error message to be displayed when the exception is raised
    std::stringstream error;

    /// The reason message to be displayed when the exception is raised
    std::stringstream reason;

    /// The line in the cpp file
    int line;

    std::string file;
};

namespace internal {

/// Create a message error from an Exception instance
/// @param exception The exception instance to be raised
/// @param file The name of the file where the exception was raised
/// @param line The number of the line where the exception was raised
/// @return The message error of the exception
/// @see Exception
std::string message(const Exception& exception, const std::string& file, int line);

} // namespace internal

/// Define a macro to raise a runtime exception from an Exception instance.
/// @see Exception
/// @ingroup Common
#define RaiseError(exception) \
    throw std::runtime_error(ThermoFun::internal::message(exception, __FILE__, __LINE__));

/// Define a macro to raise a runtime exception from a error string and a reason string.
/// @see Exception
/// @ingroup Common
#define RuntimeError(errorstr, reasonstr) \
    { \
        Reaktoro::Exception exception; \
        exception.error << errorstr; \
        exception.reason << reasonstr; \
        RaiseError(exception); \
    }

/// Define a macro to raise a runtime exception from a error string and a reason string.
/// @see Exception
/// @ingroup Common
#define Assert(condition, errorstr, reasonstr) \
    { \
        if(!(condition)) { \
            Reaktoro::Exception exception; \
            exception.error << errorstr; \
            exception.reason << reasonstr; \
            RaiseError(exception); \
        } \
    }

auto errorMethodNotFound(std::string type, std::string name, int line, std::string file) -> void;

auto errorReactionNotDefined(std::string name, int line, std::string file) -> void;

auto errorSolventNotDefined(std::string type, std::string name, int line, std::string file) -> void;

auto errorModelParameters(std::string type, std::string name, int line, std::string file) -> void;

auto errorSameSymbol(std::string type, std::string name, int line, std::string file) -> void;

auto errorNotMatchingLevel(std::string type, std::string name, int line, std::string file) -> void;

auto setMessage(Reaktoro_::Status sta, std::string propertyOfsymbol, std::string calcMessage, std::string &message_ ) -> void;

auto setMessage(Reaktoro_::Status sta, std::string message , ThermoPropertiesSubstance &tps) -> void;

auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesReaction &tpr ) -> void;

} // namespace ThermoFun

#endif // EXCEPTION_H
