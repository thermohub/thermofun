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
    auto T     = Reaktoro_::Temperature(pimpl->reference_T);
    auto Cp    = pimpl->thermo_parameters.reaction_Cp_fT_coeff;
    auto A     = pimpl->thermo_parameters.reaction_logK_fT_coeff; A.resize(7);
    auto ref_prop = pimpl->thermo_ref_prop;

    auto Sr = ref_prop.reaction_entropy;
    auto Hr = ref_prop.reaction_enthalpy;

    // calculation of logK=f(T) coeffs (only first 5 Cp coefficients, conforming to Haas-Fisher function)
    A[0] = (( Sr - Cp[0] - Cp[0]*log(T) - Cp[1]*T + Cp[2]/(2.0*T*T)
                  + 2.0*Cp[3]/pow(T,0.5) - Cp[4]*T*T/2.0 ) / Rln10).val;
    A[1] = Cp[1]/(2.0*Rln10);
    A[2] = (-( Hr - Cp[0]*T - Cp[1]*T*T/2.0 + Cp[2]/T
               - 2.0*Cp[3]*pow(T,0.5) - Cp[4]*T*T*T/3.0 ) / Rln10).val;
    A[3] = Cp[0]/Rln10;
    A[4] = Cp[2]/(2.0*Rln10);
    A[5] = Cp[4]/(6.0*Rln10);
    A[6] = -4.0*Cp[3]/Rln10;

    // Calculation of Cpr and lgK at 25 C
    auto Cpr  = Cp[0] + Cp[1]*T + Cp[2]/T*T + Cp[3]/pow(T,0.5) + Cp[4]*T*T;
    auto lgKr = A[0] + A[1]*T + A[2]/T + A[3]*log(T) + A[4]/T*T +
                A[5]*T*T + A[6]/pow(T,0.5);
    ref_prop.reaction_heat_capacity_cp = Cpr;
    ref_prop.ln_equilibrium_constant = lgKr*lg_to_ln;
}
auto Reaction::convert_logKfT_toCpfT(MethodCorrT_Thrift::type methodT) -> void
{

    auto Rln10 = R_CONSTANT * lg_to_ln;
    auto T     = Reaktoro_::Temperature(pimpl->reference_T);
    auto Cp    = pimpl->thermo_parameters.reaction_Cp_fT_coeff;
    auto A     = pimpl->thermo_parameters.reaction_logK_fT_coeff;
    auto ref_prop = pimpl->thermo_ref_prop;

    auto Sr = ref_prop.reaction_entropy;
//    auto Gr = ref_prop.reaction_gibbs_energy;
    auto Hr = ref_prop.reaction_enthalpy;
    auto Cpr = ref_prop.reaction_heat_capacity_cp;
    auto lgKr = ref_prop.ln_equilibrium_constant / lg_to_ln;

    switch( methodT )
    { // calculation 2- and 3-term�param approximation
    case MethodCorrT_Thrift::type::CTM_DKR: // 3-term extrap. from Franck-Marshall density model
    case MethodCorrT_Thrift::type::CTM_MRB: // 3-term extrap. from MRB at 25 C (provisional DK 06.08.07)
    case MethodCorrT_Thrift::type::CTM_LGK:  // Here, all logK-f(T) coeffs are given
    case MethodCorrT_Thrift::type::CTM_LGX:  // uncommented, 19.06.2008
        break;
    case MethodCorrT_Thrift::type::CTM_IKZ:  // Isotopic forms
        return;
    case MethodCorrT_Thrift::type::CTM_EK0: // Generating 1-term extrapolation at logK = const
        A[0]=(Sr/Rln10).val;
        A[1]=0.0;
        A[2]=0.0;
        A[3]=0.0;
        A[4]=0.0;
        A[5]=0.0;
        A[6]=0.0;
        break;
    case MethodCorrT_Thrift::type::CTM_EK1: // Generating 1-term extrapolation at dGr = const
        A[0]=0.0;
        A[1]=0.0;
        A[2]=(-Hr/Rln10).val;
        A[3]=0.0;
        A[4]=0.0;
        A[5]=0.0;
        A[6]=0.0;
        break;
    case MethodCorrT_Thrift::type::CTM_EK2: // Generating 2-term (Vant Hoff) extrapolation
        A[0]=(Sr/Rln10).val;
        A[1]=0.0;
        A[2]=(-Hr/Rln10).val;
        A[3]=0.0;
        A[4]=0.0;
        A[5]=0.0;
        A[6]=0.0;
        break;
    case MethodCorrT_Thrift::type::CTM_PPE:
    case MethodCorrT_Thrift::type::CTM_EK3: // Generating 3-term extrapolation at constant dCpr
//        if( Cp )
//        {
        Cp[1] = Cp[2] = Cp[3] = Cp[4] = 0.0;
//        ref_prop.reaction_heat_capacity_cp = Cp[0];
//        }

        A[0]=(( Sr - Cpr*(1.+log(T)) ) / Rln10).val;
        A[1]=0.0;
        A[2]=(( Cpr*T - Hr ) / Rln10).val;
        A[3]=(Cpr / Rln10).val;
        A[4]=0.0;
        A[5]=0.0;
        A[6]=0.0;
        break;
    default:
        cout << "error";
//        Error( GetName(),"E23RErun: Invalid method code in Convert_KT_to_Cp()");
    }

    switch( methodT )
    {
    case MethodCorrT_Thrift::type::CTM_LGK:
       /* Calc lgK, Hr and Sr */
       lgKr = A[0] + A[1]*T + A[2]/T + A[3]*log(T) + A[4]/T*T +
             A[5]*T*T + A[6]/pow(T,0.5);
       Hr = Rln10 * ( A[1]*T*T - A[2] + A[3]*T -
                         2.0*A[4]/T + 2.0*A[5]*T*T*T - 0.5*A[6]*pow(T,0.5) );
       Sr = Rln10 * ( A[0] + 2.0*A[1]*T + A[3]*(1.0 + log(T))
                   - A[4]/T*T + 3.0*A[5]*T*T + 0.5*A[6]/pow(T,0.5) );

       Cp[0] = Rln10 * A[3];
       Cp[1] = Rln10 * 2.0 * A[1];
       Cp[2] = Rln10 * 2.0 * A[4];
       Cp[3] = -Rln10 * 0.25 * A[6];
       Cp[4] = Rln10 * 6.0 * A[5];
       Cpr   = Cp[0] + Cp[1]*T + Cp[2]/T*T + Cp[4]*T*T + Cp[3]/pow(T,0.5);

       ref_prop.reaction_entropy = Sr;  // In this case, everything will be inserted
       ref_prop.reaction_enthalpy = Hr;
       ref_prop.reaction_heat_capacity_cp = Cpr;
       ref_prop.ln_equilibrium_constant = lgKr * lg_to_ln;
       ref_prop.reaction_gibbs_energy = -Rln10*T*lgKr;
    }

}

} // namespace TCorrPT

