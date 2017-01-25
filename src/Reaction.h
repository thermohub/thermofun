#ifndef REACTION_H
#define REACTION_H

// C++ includes
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ThermoFun_global.h"

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

    auto setReactants(std::map<std::string, int> reactants) -> void;

    auto setReferenceT(double T) -> void;

    auto setReferenceP(double P) -> void;

    /// Set the code for general EOS calculation method
    auto setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void;

    /// Set the code for the temperature correction method
    auto setMethod_T(MethodCorrT_Thrift::type method_T) -> void;

    /// Set the code for the pressure correction method
    auto setMethod_P(MethodCorrP_Thrift::type method_P) -> void;

    auto setThermoReferenceProperties(ThermoPropertiesReaction refprop) -> void;

    auto setThermoParameters(ThermoParametersReaction param) -> void;

//    /// Set the formula of the Reaction.
//    auto setFormula(std::string formula) -> void;

//    /// Set the name of the reaction that defines the Reaction properties.
//    auto setReaction(std::string reaction) -> void;

    // Get functions
    /// Return the name of the chemical Reaction
    auto name() const -> std::string;

    auto symbol() const -> std::string;

    auto reactants() -> std::map<std::string, int>;

    auto thermo_ref_prop() -> ThermoPropertiesReaction;

    auto thermo_parameters() -> ThermoParametersReaction;

    auto referenceT() const -> double;

    auto referenceP() const -> double;

    /// Return the general EOS method code
    auto methodGenEOS() -> MethodGenEoS_Thrift::type;

    /// Return the temperature correction method code
    auto method_T() -> MethodCorrT_Thrift::type;

    /// Return the pressure correction method code
    auto method_P() -> MethodCorrP_Thrift::type;

//    /// Return the formula of the chemical Reaction
//    auto formula() const -> std::string;


private:

    auto convert_CpfT_to_logKfT() -> void;
    auto convert_logKfT_toCpfT(MethodCorrT_Thrift::type methodT) -> void;

    struct Impl;

    std::unique_ptr<Impl> pimpl;

};

} // namespace ThermoFun

#endif // REACTION_H
