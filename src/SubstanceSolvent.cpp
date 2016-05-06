#include "SubstanceSolvent.h"

namespace TCorrPT {

struct SubstanceSolvent::Impl
{
    /// The thermodynamic data specific to the solvent.
    ThermoPropertiesSolvent thermo;
};


SubstanceSolvent::SubstanceSolvent()
: pimpl(new Impl())
{}

SubstanceSolvent::SubstanceSolvent(const Substance& solvent)
: Substance(solvent), pimpl(new Impl())
{}

SubstanceSolvent::SubstanceSolvent(const SubstanceSolvent& other)
: Substance(other), pimpl(new Impl(*other.pimpl))
{}

SubstanceSolvent::~SubstanceSolvent()
{}

auto SubstanceSolvent::operator=(SubstanceSolvent other) -> SubstanceSolvent&
{
    Substance::operator=(other);
    pimpl = std::move(other.pimpl);
    return *this;
}

auto SubstanceSolvent::setThermoPropertiesSolvent(const ThermoPropertiesSolvent& thermo) -> void
{
    pimpl->thermo = thermo;
}

} // namespace TCorrPT

