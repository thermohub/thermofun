#ifndef SUBSTANCE_H
#define SUBSTANCE_H

// C++ includes
#include <map>
#include <memory>
#include <string>
#include <vector>

// TCorrPT includes
#include "tcorrpt_global.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace TCorrPT {

/// A type used to describe a chemical substance (species, dependent components) 
class Substance
{
public:
    /// Constrcut a default Substance instance
    Substance();
    
    /// Construct a copy of an Substance instance
    Substance(const Substance& other);
    
    /// Assign a Substance instance to this instance
    auto operator=(Substance other) -> Substance&;

    /// Destroy this instance
    virtual ~Substance();

    // Set functions
    /// Set the name of the Substance.
    auto setName(std::string name) -> void;

    /// Set the formula of the Substance.
    auto setFormula(std::string formula) -> void;

    /// Set the name of the reaction that defines the substance properties.
    auto setReaction(std::string reaction) -> void;

    /// Set the molar mass of the chemical species (in units of kg/mol)
    auto setMolarMass(double molar_mass) -> void;

    /// Set the reference thermodynamic data of the substance.
    auto setThermoReferenceProperties(ThermoPropertiesSubstance thermo_ref_prop) -> void;

    /// Set the reference thermodynamic data of the substance.
    auto setThermoParameters(ThermoParametersSubstance thermo_param) -> void;

    /// Set the code for general EOS calculation method
    auto setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void;

    /// Set the code for the temperature correction method
    auto setMethod_T(MethodCorrT_Thrift::type method_T) -> void;

    /// Set the code for the pressure correction method
    auto setMethod_P(MethodCorrP_Thrift::type method_P) -> void;

    /// Set the code for the substance class
    auto setSubstanceClass(SubstanceClass::type substance_class) -> void;

    /// Set the code for the aggregate state of the substance
    auto setAggregateState(AggregateState::type aggregate_state) -> void;

    /// Set
    auto setCharge(int charge) -> void;

    /// Set
    auto setSolventName(string name) -> void;

    /// Set
    auto setReferenceT(double T) -> void;

    /// Set
    auto setReferenceP(double P) -> void;
    
    // Get functions
    /// Return the name of the chemical Substance
    auto name() const -> std::string;

    /// Return the formula of the chemical Substance
    auto formula() const -> std::string;

    /// Return the reaction name that defines the properties of the chemical Substance
    auto reaction() const -> std::string;

    /// Return the molar mass of the chemical species (in units of kg/mol)
    auto molarMass() const -> double;

    /// Return the reference temperature (celsius)
    auto referenceT() const -> double;

    /// Return the referennce pressure (bar)
    auto referenceP() const -> double;

    /// Return the thermodynamic data of the substance.
    auto thermoProperties() -> ThermoPropertiesSubstance;

    /// Return the thermodynamic parameters for calculating thermodynamic properties of the substance.
    auto thermoParameters() -> ThermoParametersSubstance;

    /// Return the reference thermodynamic data of the substance.
    auto thermoReferenceProperties() -> ThermoPropertiesSubstance;

    /// Return the general EOS method code
    auto methodGenEOS() -> MethodGenEoS_Thrift::type;

    /// Return the temperature correction method code
    auto method_T() -> MethodCorrT_Thrift::type;

    /// Return the pressure correction method code
    auto method_P() -> MethodCorrP_Thrift::type;

    /// Return the class type of the substance
    auto substanceClass() -> SubstanceClass::type;

    /// Return
    auto charge() -> int;

    /// Return
    auto SolventName() -> string;
    
private:
    struct Impl;

    std::unique_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // SUBSTANCE_H
