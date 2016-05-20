#ifndef ELECTROMODELSSOLVENT_H
#define ELECTROMODELSSOLVENT_H

#include <memory>

namespace TCorrPT {

struct Substance;
struct ElectroPropertiesSolvent;
struct PropertiesSolvent;

class WaterJNreaktoro
{
public:
    /// Construct a default class object instance
    WaterJNreaktoro();

    /// Construct an class object instance from a Species instance
    explicit WaterJNreaktoro(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto electroPropertiesSolvent(double T, double P, PropertiesSolvent ps) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class WaterJNgems
{
public:
    /// Construct a default class object instance
    WaterJNgems();

    /// Construct an class object instance from a Species instance
    explicit WaterJNgems(const Substance& substance);

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto electroPropertiesSolvent(double T, double P) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};



} // namespace TCorrPT

#endif // ELECTROMODELSSOLVENT_H
