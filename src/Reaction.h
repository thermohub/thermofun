#ifndef REACTION_H
#define REACTION_H

// C++ includes
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "GlobalVariables.h"

namespace ThermoFun {

class ThermoPropertiesReaction;
class ThermoParametersReaction;

/**
 * @brief The Reaction class is a type used to describe a reaction
 */
class Reaction
{
public:
    /// Constrcut a default Reaction instance
    Reaction();

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

//    /// Set the formula of the Reaction.
//    auto setFormula(std::string formula) -> void;

//    /// Set the name of the reaction that defines the Reaction properties.
//    auto setReaction(std::string reaction) -> void;

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
    auto thermo_ref_prop() const  -> ThermoPropertiesReaction;

    /// Returns an instance of the defined parameters of the reaction
    auto thermo_parameters() const  -> ThermoParametersReaction;

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

    /// Checks if the method of clauclation is out of provided T and P bounds. If out of bounds sets the corresponding message
    /// inside the property status
    /// @param modelName Given model name
    /// @param T temparature in bar
    /// @param P pressure in C
    /// @param tpr calculated properties of the reaction, their status message is changed is T and P is out of bounds
    auto checkCalcMethodBounds(string modelName, double T, double P, ThermoPropertiesReaction &tpr) -> void;

//    /// Return the formula of the chemical Reaction
//    auto formula() const -> std::string;

//    /// Calculates record parameters based on the defined method and available data
//    auto calcParameters () -> void;

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

} // namespace ThermoFun

#endif // REACTION_H
