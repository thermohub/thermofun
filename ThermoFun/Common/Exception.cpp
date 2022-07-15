#include "Exception.h"

// C++ includes
#include <algorithm>
#include <sstream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "ThermoProperties.h"

#define LOG_PATTERN "[%n] [%^%l%$] %v"

namespace ThermoFun {

// Thread-safe logger to stdout with colors
std::shared_ptr<spdlog::logger> thfun_logger = spdlog::stdout_color_mt("thermofun");

void update_loggers( bool use_cout, const std::string& logfile_name, size_t log_level)
{
    auto thermofun_logger = spdlog::get("thermofun");
    auto chemicalfun_logger = spdlog::get("chemicalfun");

    // change level
    spdlog::level::level_enum log_lev = spdlog::level::info;
    if( log_level<7 ) {
        log_lev = static_cast<spdlog::level::level_enum>(log_level);
    }
    thermofun_logger->set_level(log_lev);
    chemicalfun_logger->set_level(log_lev);

    //change sinks
    thermofun_logger->sinks().clear();
    chemicalfun_logger->sinks().clear();
    if(use_cout) {
        auto console_output = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_output->set_pattern(LOG_PATTERN);
        thermofun_logger->sinks().push_back(console_output);
        chemicalfun_logger->sinks().push_back(console_output);
    } else
    if (!logfile_name.empty())
    {
        auto file_output = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfile_name, 1048576, 3);
        thermofun_logger->sinks().push_back(file_output);
        chemicalfun_logger->sinks().push_back(file_output);
    } 
}

void clear_loggers( const std::string& logfile_name)
{
    auto thermofun_logger = spdlog::get("thermofun");
    auto chemicalfun_logger = spdlog::get("chemicalfun");

    thermofun_logger->sinks().clear();
    chemicalfun_logger->sinks().clear();

    if (!logfile_name.empty())
    {
        auto file_output = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile_name, true);
        thermofun_logger->sinks().push_back(file_output);
        chemicalfun_logger->sinks().push_back(file_output);
    }
}


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
    if (file.size() > 45)
        ss << "..."<< file.substr(file.size() - 45, 45) << ":" << line;
    else
        ss << file << ":" << line;
    return ss.str();
}

std::string message(const Exception& exception, const std::string& /*file*/, int line)
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
    message << "*** Location: " << loc << std::endl;
    message << bar << std::endl;
    message << std::endl;
    thfun_logger->error(" {} - {} {}", line, error, reason);
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

auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesSubstance &tps ) -> void
{
    if (tps.enthalpy.sta.first == sta)
        tps.enthalpy.sta.second += message + "; ";
    if (tps.entropy.sta.first == sta)
        tps.entropy.sta.second += message + "; ";
    if (tps.gibbs_energy.sta.first == sta)
        tps.gibbs_energy.sta.second += message + "; ";
    if (tps.heat_capacity_cp.sta.first == sta)
        tps.heat_capacity_cp.sta.second += message + "; ";
    if (tps.heat_capacity_cv.sta.first == sta)
        tps.heat_capacity_cv.sta.second += message + "; ";
    if (tps.helmholtz_energy.sta.first == sta)
        tps.helmholtz_energy.sta.second += message + "; ";
    if (tps.internal_energy.sta.first == sta)
        tps.internal_energy.sta.second += message + "; ";
    if (tps.volume.sta.first == sta)
        tps.volume.sta.second += message + "; ";
}

auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesReaction &tpr ) -> void
{
    if (tpr.reaction_enthalpy.sta.first == sta)
        tpr.reaction_enthalpy.sta.second += message + "; ";
    if (tpr.reaction_entropy.sta.first == sta)
        tpr.reaction_entropy.sta.second += message + "; ";
    if (tpr.reaction_gibbs_energy.sta.first == sta)
        tpr.reaction_gibbs_energy.sta.second += message + "; ";
    if (tpr.reaction_heat_capacity_cp.sta.first == sta)
        tpr.reaction_heat_capacity_cp.sta.second += message + "; ";
    if (tpr.reaction_heat_capacity_cv.sta.first == sta)
        tpr.reaction_heat_capacity_cv.sta.second += message + "; ";
    if (tpr.reaction_helmholtz_energy.sta.first == sta)
        tpr.reaction_helmholtz_energy.sta.second += message + "; ";
    if (tpr.reaction_internal_energy.sta.first == sta)
        tpr.reaction_internal_energy.sta.second += message + "; ";
    if (tpr.reaction_volume.sta.first == sta)
        tpr.reaction_volume.sta.second += message + "; ";
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

auto errorSolventNotDefined(std::string /*type*/, std::string name, int line, std::string file) -> void
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

auto funError(const std::string& error, const std::string& reason, const int& line, const std::string& file) -> void
{
    Exception exception;
    exception.error << error;
    exception.reason << reason;
    exception.line = line;
    exception.file = file;
    RaiseError(exception);
}

auto funErrorIf(bool condition, const std::string& error, const std::string& reason, const int& line, const std::string& file) -> void
{
    if (condition)
        funError(error, reason, line, file);
}

} // namespace ThermoFun
