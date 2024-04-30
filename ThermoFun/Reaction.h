#ifndef REACTION_H
#define REACTION_H

// C++ includes
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "GlobalVariables.h"

namespace ThermoFun {

struct ThermoPropertiesReaction;
struct ThermoParametersReaction;

/**
 * @brief The Reaction class is a type used to describe a reaction
 */
class Reaction
{
public:
    /// Constrcut a default Reaction instance
    Reaction();

    /// Constrcut a Reaction instance from a JSON string
    Reaction(std::string jsonReaction);

    /// Construct a copy of an Reaction instance
    Reaction(const Reaction& other);

    /// Assign an Reaction instance to this instance
    auto operator=(Reaction other) -> Reaction&;

    /// Destroy this instance
    virtual ~Reaction();

    // Set functions
    /// Set the name of the Reaction.
    auto setName(std::string name) -> void;

    /// Set the symbol of the Reaction.
    auto setSymbol(std::string symbol) -> void;

    /// Set the equation of the Reaction
    auto setEquation(std::string equation) -> void;

    /// Set map of reactant symbols
    auto setReactants(std::map<std::string, double> reactants) -> void;

    /// Set reference temperature fo the standard properties of reaction (K)
    auto setReferenceT(double T) -> void;

    /// Set references pressure for the standard proeprties of reaction (Pa)
    auto setReferenceP(double P) -> void;

    /// Set upper temperature limit of the correction method (K)
    auto setLowerT(double T) -> void;

    /// Set upper presure limit of the correction method (Pa)
    auto setLowerP(double P) -> void;

    /// Set lower temperature limit of the correction method (K)
    auto setUpperT(double T) -> void;

    /// Set lower pressure limit for the correction method (Pa)
    auto setUpperP(double P) -> void;

    /// Set the code for general EOS calculation method
    auto setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void;

    /// Set the code for the temperature correction method
    auto setMethod_T(MethodCorrT_Thrift::type method_T) -> void;

    /// Set the code for the pressure correction method
    auto setMethod_P(MethodCorrP_Thrift::type method_P) -> void;

    /// Set the refereneces proeprties of reaction
    auto setThermoReferenceProperties(ThermoPropertiesReaction refprop) -> void;

    /// Set the parameters used for calculating the proeprties of reaction at T and P
    auto setThermoParameters(ThermoParametersReaction param) -> void;

    /**
     * @brief setJsonString stores the json object of the record
     * @param jString
     */
    auto setJsonString(const std::string &jString) ->void;

    /**
     * @brief fromEquation initialize the reaction using a reaction equation
     * @param reactionEquation e.g. Calcite = Ca+2 + CO3-2 (use the symbols of substances!)
     */
    auto fromEquation(const std::string &reactionEquation) ->void;

    // Get functions
    /// Returns the name of the chemical Reaction
    auto name() const -> std::string;

    /// Returns the symbol of the Reaction
    auto symbol() const -> std::string;

    /// Returns the equation of the Reaction
    auto equation() const -> std::string;

    /// Returns the map of reactants symbols and their coeffiicents
    auto reactants() const -> std::map<std::string, double>;

    /// Returns the references proeprties of the reaction
    auto thermoReferenceProperties() const  -> ThermoPropertiesReaction;
    auto thermo_ref_prop() const -> ThermoPropertiesReaction;

    /// Returns an instance of the defined parameters of the reaction
    auto thermoParameters() const  -> ThermoParametersReaction;

    /// Returns the references temperature (K)
    auto referenceT() const -> double;

    /// Returns the references pressure (Pa)
    auto referenceP() const -> double;

    /// Return upper temperature limit of the correction method (K)
    auto lowerT( ) const -> double;

    /// Return upper presure limit of the correction method (Pa)
    auto lowerP( ) const -> double;

    /// Return lower temperature limit of the correction method (K)
    auto upperT( ) const -> double;

    /// Return lower pressure limit for the correction method (Pa)
    auto upperP( ) const -> double;

    /// Return the general EOS method code
    auto methodGenEOS() const  -> MethodGenEoS_Thrift::type;

    /// Return the temperature correction method code
    auto method_T() const  -> MethodCorrT_Thrift::type;

    /// Return the pressure correction method code
    auto method_P() const  -> MethodCorrP_Thrift::type;

    /**
     * @brief jonString
     * @return record in json string
     */
    auto jsonString() const -> std::string;

    /// Checks if the method of clauclation is out of provided T and P bounds. If out of bounds sets the corresponding message
    /// inside the property status
    /// @param modelName Given model name
    /// @param T temparature in bar
    /// @param P pressure in C
    /// @param tpr calculated properties of the reaction, their status message is changed is T and P is out of bounds
    auto checkCalcMethodBounds(std::string modelName, double T, double P, ThermoPropertiesReaction &tpr) -> void;

    /// Returns the vector of logK as a function of T coefficients calculated based on the defined method and the available data
    auto calc_logK_fT_coefficients() -> vd;

private:

    /// Converts the Cp as a function of T coefficients into logK as a function of T coefficients
    auto convert_CpfT_to_logKfT() -> ThermoPropertiesReaction;

    /// Converst the logK as a function of T coefficients into Cp as a function of T coefficients
    auto convert_logKfT_toCpfT() -> ThermoPropertiesReaction;

    auto logK_params_from_123TermExtrapolations() -> ThermoPropertiesReaction;

    struct Impl;

    std::unique_ptr<Impl> pimpl;

};

auto operator<<(std::ostream& stream, const Reaction& react) -> std::ostream&;

} // namespace ThermoFun

#endif // REACTION_H
