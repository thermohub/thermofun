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

    /// Reference temperature (in K)
    double reference_T;

    /// Reference pressure (in ___)
    double reference_P;

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

auto Reaction::setReferenceT(double T) -> void
{
    pimpl->reference_T = T;
}

auto Reaction::setReferenceP(double P) -> void
{
    pimpl->reference_P = P;
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

auto Reaction::referenceT() const -> double
{
    return pimpl->reference_T;
}

auto Reaction::referenceP() const -> double
{
    return pimpl->reference_P;
}

//auto Reaction::formula() const -> std::string
//{
//    return pimpl->formula;
//}

auto Reaction::convert_CpfT_to_logKfT() -> void
{
    auto Rln10 = R_CONSTANT * lg_to_ln;
    auto T     = pimpl->reference_T;
    auto Cp    = pimpl->thermo_parameters.reaction_Cp_fT_coeff;
    auto A     = pimpl->thermo_parameters.reaction_logK_fT_coeff;
    auto ref_prop = pimpl->thermo_ref_prop;

    auto Sr = ref_prop.reaction_entropy;
    auto Hr = ref_prop.reaction_enthalpy;

//    // calculation of logK=f(T) coeffs (only first 5 Cp coefficients, conforming to Haas-Fisher function)
//    A[0] = ( Sr - Cp[0] - Cp[0]*lnT - Cp[1]*T + Cp[2]/(2.0*T_2)
//                  + 2.0*Cp[3]/T_05 - Cp[4]*T_2/2.0 ) / Rln10;
//    A[1] = Cp[1]/(2.0*Rln10);
//    A[2] = -( Hr - Cp[0]*T - Cp[1]*T_2/2.0 + Cp[2]/T
//               - 2.0*Cp[3]*T_05 - Cp[4]*T_3/3.0 ) / Rln10;
//    A[3] = Cp[0]/Rln10;
//    A[4] = Cp[2]/(2.0*Rln10);
//    A[5] = Cp[4]/(6.0*Rln10);
//    A[6] = -4.0*Cp[3]/Rln10;

//// Old version - comment out after debugging !
////    A[0] = Sr/Rln10 - ( Cp[1]*T + Cp[0]*(1.0+lnT) - Cp[2]/(2.0*T_2) +
////                        Cp[4]*T_2/2.0 - 2.0*Cp[3]/T_05 ) / Rln10;
////    A[2] = ( Cp[1]*T_2/2.0 + Cp[0]*T -Cp[2]/T + Cp[4]*T_3/3.0 +
////             2.0*Cp[3]*T_05 - Hr ) / Rln10;
///* commented out by KD on 15.07.03
//    switch(CE) // zeroing off unnecessary coeffs for specific cases
//    {
//    case CTM_LGK:
//    case CTM_LGX:
//        break;
//    case CTM_EK1:
//        K[0]=0.0;
//    case CTM_EK2:
//        K[3]=0.0;
//    case CTM_EK3:
//        K[1]=0.0;
//        K[4]=0.0;
//        K[5]=0.0;
//        K[6]=0.0;
//        break;
//    default:
//        return;
//    }
//*/

//    // Calculation of Cpr and lgK at 25 C
//    Cpr = Cp[0] + Cp[1]*T + Cp[2]/T_2 + Cp[3]/T_05 + Cp[4]*T_2;
//    lgK = A[0] + A[1]*T + A[2]/T + A[3]*lnT + A[4]/T_2 +
//          A[5]*T_2 + A[6]/T_05;
//    rcp->Cps[0] = Cpr;
//    rcp->Ks[1] = lgK;
}
auto Reaction::convert_logKfT_toCpfT(MethodCorrT_Thrift::type methodT) -> void
{

}

} // namespace TCorrPT

