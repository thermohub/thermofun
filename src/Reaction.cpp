#include "Reaction.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace TCorrPT {

//namespace {
//using SubstancesMap = std::map<std::string, Substance>;
//using ReactionsMap = std::map<std::string, Reaction>;
//}

struct Reaction::Impl
{
    /// The name of the chemical Reaction
    std::string name;

    /// The chemical formula of the chemical Reaction
    std::string formula;

    std::map<std::string, int> reactants;

    ThermoPropertiesReaction thermo_ref_prop;

    ThermoParametersReaction thermo_parameters;

};

Reaction::Reaction()
: pimpl(new Impl())
{}

Reaction::Reaction(const Reaction& other)
: pimpl(new Impl(*other.pimpl))
{}

Reaction::~Reaction()
{}

auto Reaction::operator=(Reaction other) -> Reaction&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto Reaction::setName(std::string name) -> void
{
    pimpl->name = name;
}

auto Reaction::setReactants(std::map<std::string, int> reactants) -> void
{
    pimpl->reactants = reactants;
}

//auto Reaction::setFormula(std::string formula) -> void
//{
//    pimpl->formula = formula;
//}

auto Reaction::name() const -> std::string
{
    return pimpl->name;
}

auto Reaction::reactants() -> std::map<std::string, int>
{
    return pimpl->reactants;
}

auto Reaction::thermo_ref_prop() -> ThermoPropertiesReaction
{
    return pimpl->thermo_ref_prop;
}

auto Reaction::thermo_parameters() -> ThermoParametersReaction
{
    return pimpl->thermo_parameters;
}

//auto Reaction::formula() const -> std::string
//{
//    return pimpl->formula;
//}

} // namespace TCorrPT

