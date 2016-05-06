#ifndef SUBSTANCESOLVENT_H
#define SUBSTANCESOLVENT_H

// TCorrPT includes
#include "Substance.h"
//#include "ThermoProperties.h"

// Forward declarations
//class Substance;

namespace TCorrPT {

class SubstanceSolvent : public Substance
{
public:
    SubstanceSolvent();

    /// Construct an SubstanceSolvent instance from a Species instance
    SubstanceSolvent(const Substance& solvent);

    /// Construct a copy of an SubstanceSolvent instance
    SubstanceSolvent(const SubstanceSolvent& other);

    /// Destroy this instance
    virtual ~SubstanceSolvent();

    /// Assign an SubstanceSolvent instance to this instance
    auto operator=(SubstanceSolvent other) -> SubstanceSolvent&;

    /// Set the thermodynamic data of the aqueous species.
    auto setThermoPropertiesSolvent(const ThermoPropertiesSolvent& thermo) -> void;

private:
    struct Impl;

    std::unique_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // SUBSTANCESOLVENT_H
