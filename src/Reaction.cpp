#include "Reaction.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

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

//  std::map<std::string, double>  substances_in_reaction_map

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

//auto Reaction::setFormula(std::string formula) -> void
//{
//    pimpl->formula = formula;
//}

auto Reaction::name() const -> std::string
{
    return pimpl->name;
}

//auto Reaction::formula() const -> std::string
//{
//    return pimpl->formula;
//}

} // namespace TCorrPT

