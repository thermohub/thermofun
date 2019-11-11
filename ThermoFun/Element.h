#ifndef ELEMENT_H
#define ELEMENT_H

// C++ includes
#include <memory>
#include <string>

namespace ThermoFun {

/// A type used to define a chemical element and its attributes
class Element
{
public:
    /// Construct a default Element instance
    Element();

    /// Set the name of the element
    auto setName(std::string name) -> void;

    /// Set the symbol of the element
    auto setSymbol(std::string symbol) -> void;

    /// Set the molar mass of the element (in units of g/mol)
    auto setMolarMass(double value) -> void;

    /// Set the entropy of the element (in units of J/mol/K)
    auto setEntropy(double value) -> void;

    /// Set the heat capacity of the element (in units of J/mol/K)
    auto setHeatCapacity(double value) -> void;

    /// Set the volume of the element (in units of cm3/mol)
    auto setVolume(double value) -> void;

    /// Set the valence of the element
    auto setValence(int value) -> void;

    /// Set the class_ of the element (as in element.thrift)
    auto setClass(int value) -> void;

    /// Set the rounded isotopic mass of the element
    auto setIsotopeMass(int value) -> void;

    /// Set the element Mendeleev table number
    auto setNumber(int value) -> void;

    /**
     * @brief setJsonString stores the json object of the record
     * @param jString
     */
    auto setJsonString(const std::string &jString) ->void;

    /// Return the name of the element
    auto name() const -> std::string;

    /// Return the symbol of the element
    auto symbol() const -> std::string;

    /// Return the molar mass of the element (in units of g/mol)
    auto molarMass() const -> double;

    /// Return the entropy of the element (in units of J/mol/K)
    auto entropy() const -> double;

    /// Return the heat capacity of the element (in units of J/mol/K)
    auto heatCapacity() const -> double;

    /// Return the volume of the element (in units of cm3/mol)
    auto volume() const -> double;

    /// Return the valence of the element
    auto valence() const -> int;

    /// Return the class of the element (as in element.thrift)
    auto class_() const -> int;

    /// Return the rounded isotopic mass of the element
    auto isotopeMass() const -> int;

    /// Return the Mendeleev table number of the element
    auto number() const -> int;

    /**
     * @brief jonString
     * @return record in json string
     */
    auto jsonString() const -> std::string;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/// Compare two Element instances for less than
auto operator<(const Element& lhs, const Element& rhs) -> bool;

/// Compare two Element instances for equality
auto operator==(const Element& lhs, const Element& rhs) -> bool;

} // namespace ThermoFun

#endif // ELEMENT_H
