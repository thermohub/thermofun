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
    
    /// Assign an Substance instance to this instance
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

    /// Set
    auto setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void;

    /// Set
    auto setSubstanceClass(SubstanceClass::type substance_class) -> void;

    /// Set
    auto setAggregateState(AggregateState::type aggregate_state) -> void;
    
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

    /// Return the general EOS method of calculating the thermodynamic properties
    auto methodGenEOS() -> MethodGenEoS_Thrift::type;

    /// Return the class of the substance
    auto substanceClass() -> SubstanceClass::type;
    
private:
    struct Impl;

    std::unique_ptr<Impl> pimpl;
};


} // namespace TCorrPT

#endif // SUBSTANCE_H
