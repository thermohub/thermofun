#include "Element.h"

namespace ThermoFun {

struct Element::Impl
{
    /// The name of the chemical element
    std::string name;

    /// The symbol of the chemical element
    std::string symbol;

    /// The entropy of the element (in units of J/mol/K)
    double entropy;

    /// The heat capacity of the element (in units of J/mol/K)
    double heatCapacity;

    /// The volume of the element (in units of cm3/mol)
    double volume;

    /// The valence of the element
    int valence;

    /// The molar mass of the chemical element (in units of g/mol)
    double molarMass;

    /// Element class as in element.thrift
    int class_;

    /// Element isotope (isotope mass, rounded)
    int isotopeMass;

    /// Element Mendeleev table number
    int number;

    std::string jString;
};

Element::Element()
: pimpl(new Impl())
{}

auto Element::setName(std::string name) -> void
{
    pimpl->name = name;
}

auto Element::setSymbol(std::string symbol) -> void
{
    pimpl->symbol = symbol;
}

auto Element::setMolarMass(double value) -> void
{
    pimpl->molarMass = value;
}

auto Element::setEntropy(double value) -> void
{
    pimpl->entropy = value;
}

auto Element::setHeatCapacity(double value) -> void
{
    pimpl->heatCapacity = value;
}

auto Element::setVolume(double value) -> void
{
    pimpl->volume = value;
}

auto Element::setValence(int value) -> void
{
    pimpl->valence = value;
}

auto Element::setClass(int value) -> void
{
    pimpl->class_ = value;
}

auto Element::setIsotopeMass(int value) -> void
{
    pimpl->isotopeMass = value;
}

auto Element::setNumber(int value) -> void
{
    pimpl->number = value;
}

auto Element::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
}

auto Element::name() const -> std::string
{
    return pimpl->name;
}

auto Element::symbol() const -> std::string
{
    return pimpl->symbol;
}

auto Element::molarMass() const -> double
{
    return pimpl->molarMass;
}

auto Element::entropy() const -> double
{
    return pimpl->entropy;
}

auto Element::heatCapacity() const -> double
{
    return pimpl->heatCapacity;
}

auto Element::volume() const -> double
{
    return pimpl->volume;
}

auto Element::valence() const -> int
{
    return pimpl->valence;
}

auto Element::class_() const -> int
{
    return pimpl->class_;
}

auto Element::isotopeMass() const -> int
{
    return pimpl->isotopeMass;
}

auto Element::number() const -> int
{
    return pimpl->number;
}

auto Element::jonString() const -> std::string
{
    return pimpl->jString;
}

auto operator<(const Element& lhs, const Element& rhs) -> bool
{
    if (lhs.symbol() < rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() < rhs.name())
            return true;
        if (lhs.name() == rhs.name())
        {
            if (lhs.class_() < rhs.class_())
                return true;
            if (lhs.class_() == rhs.class_())
            {
                if (lhs.isotopeMass() < rhs.isotopeMass())
                    return true;
                else
                    return (lhs.valence() < rhs.valence());
            }
        }
    }
    return false;
}

auto operator>(const Element& lhs, const Element& rhs) -> bool
{
    if (lhs.symbol() > rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() > rhs.name())
            return true;
        if (lhs.name() == rhs.name())
        {
            if (lhs.class_() > rhs.class_())
                return true;
            if (lhs.class_() == rhs.class_())
            {
                if (lhs.isotopeMass() > rhs.isotopeMass())
                    return true;
                else
                    return (lhs.valence() > rhs.valence());
            }
        }
    }
    return false;
}

auto operator==(const Element& lhs, const Element& rhs) -> bool
{
    return (lhs.symbol()        == rhs.symbol()) &&
           (lhs.name()          == rhs.name()) &&
           (lhs.class_()        == rhs.class_()) &&
           (lhs.isotopeMass()   == rhs.isotopeMass());
}

} // namespace ThermoFun
