#include "Reaction.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/Exception.h"
#include "Common/ParseJsonToData.h"

namespace ThermoFun {

struct Reaction::Impl
{
    /// The name of the chemical Reaction
    std::string name;

    /// The chemical formula of the chemical Reaction
    std::string equation;

    /// The symbol of the chemical reaction
    std::string symbol;

    std::map<std::string, double> reactants;

    ThermoPropertiesReaction thermo_ref_prop;

    ThermoParametersReaction thermo_parameters;

    /// General method (or equation of state for both T and P correction)
    MethodGenEoS_Thrift::type method_genEoS;

    /// Method of temperature correction of thermodynamic properties
    MethodCorrT_Thrift::type  method_T;

    /// Method of pressure correction of thermodynamic properties
    MethodCorrP_Thrift::type  method_P;

    /// Type of calculation used for determining the reaction properties
    /// using a defined PT correction model or from reactants
    RectionThermoCalculationType::type thermo_calculation_type = RectionThermoCalculationType::type::REACTANTS;

    /// Reference temperature (in K)
    double reference_T = 298.15;

    /// Reference pressure (in Pa)
    double reference_P = 1e5;

    /// Lower temperature limit (in K)
    double lower_T;

    /// Lower pressure limit (in Pa)
    double lower_P;

    /// Upper temperature limit (in K)
    double upper_T;

    /// Upper pressure limit (in Pa)
    double upper_P;

    std::string jString;

    void strip_all(std::string& str, const std::string& valof)
    {
      if( str.empty())
       return;
      string::size_type pos1 = str.find_first_not_of(valof);
      std::string::size_type pos2 = str.find_last_not_of(valof);
      str = str.substr( (pos1 == std::string::npos ? 0 : pos1),
        (pos2 == std::string::npos ? str.length() - 1 : pos2 - pos1 + 1));
      str.erase(remove(str.begin(), str.end(), ' '), str.end());
      str.erase(remove(str.begin(), str.end(), '\n'), str.end());
      str.erase(remove(str.begin(), str.end(), '\t'), str.end());
    }

    //  Function that can be used to split text using regexp
    std::vector<std::string> regexp_split(const std::string& str, std::string rgx_str)
    {
      std::vector<std::string> lst;

      std::regex rgx(rgx_str);

      std::sregex_token_iterator iter(str.begin(), str.end(), rgx, -1);
      std::sregex_token_iterator end;

      while (iter != end)
      {
        lst.push_back(*iter);
        strip_all(lst.back(), "\u0020\t\n");
        ++iter;
      }

      return lst;
    }

    // Extract coefficient from strings like +10.7H2O
    std::string extractCoef( const std::string& data, double& coef )
    {
      //cout << "data " << data << endl;
      coef = 1.;
      if( data.empty() || isalpha( data[0] ) || data[0] == '('  )
       return data;
      if( data[0] == '+' && ( isalpha( data[1] ) || data[1] == '(')  )
       return data.substr(1);
      if( data[0] == '-' && ( isalpha( data[1] ) || data[1] == '(')  )
      {  coef = -1.; return data.substr(1); }

      std::string::size_type sz;
      coef = stod(data,&sz);
      return data.substr(sz);
    }

    auto fromEquation(const std::string &reactionEquation) -> void
    {
        equation = reactionEquation;

        if (symbol.empty())
            symbol = reactionEquation;

        std::size_t eqpos = equation.find("=");
        std::string  str_reactants = equation.substr(0, eqpos);
        std::string  str_products = equation.substr( eqpos+1);
        double coef = 1.;
        std::string reactant;

        std::vector<std::string> parts = regexp_split(str_reactants,  "\\s+\\+\\s+" /*"\\s+"*/ );
        for( auto el: parts)
        {
            reactant = extractCoef( el, coef );
            //cout << reactant << " " << coef << " ; ";
            if(!reactant.empty())
                reactants[reactant] = coef*(-1.);
        }

        parts = regexp_split(str_products, "\\s+\\+\\s+" /*"\\s+"*/ );
        for( auto el: parts)
        {
            reactant = extractCoef( el, coef );
            //cout << reactant << " " << coef << " ; ";
            if(!reactant.empty())
                reactants[reactant] = coef;
        }
    }
};

Reaction::Reaction()
: pimpl(new Impl())
{}

Reaction::Reaction(std::string jsonReaction)
: pimpl(new Impl())
{
    Reaction other = parseReaction(jsonReaction);
    pimpl = std::move(other.pimpl);
    setJsonString(jsonReaction);
}

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

auto Reaction::setSymbol(std::string symbol) -> void
{
    pimpl->symbol = symbol;
}

auto Reaction::setReactants(std::map<std::string, double> reactants) -> void
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

auto Reaction::setLowerT(double T) -> void
{
    pimpl->lower_T = T;
}

auto Reaction::setLowerP(double P) -> void
{
    pimpl->lower_P = P;
}

auto Reaction::setUpperT(double T) -> void
{
    pimpl->upper_T = T;
}

auto Reaction::setUpperP(double P) -> void
{
    pimpl->upper_P = P;
}

auto Reaction::setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void
{
    pimpl->method_genEoS = method_genEoS;
}

auto Reaction::setMethod_T(MethodCorrT_Thrift::type method_T) -> void
{
    pimpl->method_T = method_T;
}

auto Reaction::setMethod_P(MethodCorrP_Thrift::type method_P) -> void
{
    pimpl->method_P = method_P;
}

auto Reaction::setThermoReferenceProperties(ThermoPropertiesReaction refprop) -> void
{
    pimpl->thermo_ref_prop = refprop;
}

auto Reaction::setThermoParameters(ThermoParametersReaction param) -> void
{
    pimpl->thermo_parameters = param;
}

auto Reaction::setEquation(std::string equation) -> void
{
    pimpl->equation = equation;
}

auto Reaction::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
}

auto Reaction::name() const -> std::string
{
    return pimpl->name;
}

auto Reaction::symbol() const -> std::string
{
    return pimpl->symbol;
}

auto Reaction::reactants() const -> std::map<std::string, double>
{
    return pimpl->reactants;
}

auto Reaction::thermo_ref_prop() const -> ThermoPropertiesReaction
{
    return pimpl->thermo_ref_prop;
}


auto Reaction::thermoReferenceProperties() const -> ThermoPropertiesReaction
{
    return pimpl->thermo_ref_prop;
}

auto Reaction::thermoParameters() const -> ThermoParametersReaction
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

auto Reaction::lowerT( ) const -> double
{
    return pimpl->lower_T;
}

auto Reaction::lowerP( ) const -> double
{
    return pimpl->lower_P;
}

auto Reaction::upperT( ) const -> double
{
    return pimpl->upper_T;
}

auto Reaction::upperP() const -> double
{
    return pimpl->upper_P;
}

auto Reaction::methodGenEOS() const -> MethodGenEoS_Thrift::type
{
    return pimpl->method_genEoS;
}

auto Reaction::method_T() const -> MethodCorrT_Thrift::type
{
    return pimpl->method_T;
}

auto Reaction::method_P() const -> MethodCorrP_Thrift::type
{
    return pimpl->method_P;
}

auto Reaction::equation() const -> std::string
{
    return pimpl->equation;
}

auto Reaction::jsonString() const -> std::string
{
    return pimpl->jString;
}

auto Reaction::fromEquation(const std::string &reactionEquation) -> void
{
    pimpl->fromEquation(reactionEquation);
}

auto Reaction::checkCalcMethodBounds(string modelName, double T, double P, ThermoPropertiesReaction &tpr) -> void
{
    if (pimpl->upper_P<(P) || pimpl->upper_T<(T) ||
        pimpl->lower_P>(P) || pimpl->lower_T>(T))
    {
        string message = modelName +": out of "
                                    "T(" + to_string(pimpl->lower_T) + "-" + to_string(pimpl->upper_T) +" K) and "
                                    "P(" + to_string(pimpl->lower_P) + "-" + to_string(pimpl->upper_P) +" Pa) bounds";

        setMessage(Reaktoro_::Status::calculated, message, tpr );
    }
}

auto Reaction::convert_CpfT_to_logKfT() -> ThermoPropertiesReaction
{
    auto Rln10      = R_CONSTANT * lg_to_ln;
    auto TK          = Reaktoro_::Temperature(pimpl->reference_T);
    auto CpCoeff    = pimpl->thermo_parameters.reaction_Cp_fT_coeff;
    auto K_fT_Coeff = pimpl->thermo_parameters.reaction_logK_fT_coeff;
    auto ref_prop   = pimpl->thermo_ref_prop;

    if (CpCoeff.size() < 5)
    {
        errorModelParameters("CpfT", "convert CpfT to logKfT", __LINE__, __FILE__);
    }

    K_fT_Coeff.resize(7);

    auto Hr = ref_prop.reaction_enthalpy;
    auto Sr = ref_prop.reaction_entropy;

    // calculation of logK=f(T) coeffs (only first 5 Cp coefficients, conforming to Haas-Fisher function)
    K_fT_Coeff[0] = (( Sr - CpCoeff[0] - CpCoeff[0]*log(TK) - CpCoeff[1]*TK + CpCoeff[2]/(2.0*TK*TK)
                  + 2.0*CpCoeff[3]/pow(TK,0.5) - CpCoeff[4]*TK*TK/2.0 ) / Rln10).val;
    K_fT_Coeff[1] = CpCoeff[1]/(2.0*Rln10);
    K_fT_Coeff[2] = (-( Hr - CpCoeff[0]*TK - CpCoeff[1]*TK*TK/2.0 + CpCoeff[2]/TK
               - 2.0*CpCoeff[3]*pow(TK,0.5) - CpCoeff[4]*TK*TK*TK/3.0 ) / Rln10).val;
    K_fT_Coeff[3] = CpCoeff[0]/Rln10;
    K_fT_Coeff[4] = CpCoeff[2]/(2.0*Rln10);
    K_fT_Coeff[5] = CpCoeff[4]/(6.0*Rln10);
    K_fT_Coeff[6] = -4.0*CpCoeff[3]/Rln10;

    // Calculation of Cpr and lgK at 25 C
    auto Cpr  = CpCoeff[0] + CpCoeff[1]*TK + CpCoeff[2]/TK*TK + CpCoeff[3]/pow(TK,0.5) + CpCoeff[4]*TK*TK;
    auto lgKr = K_fT_Coeff[0] + K_fT_Coeff[1]*TK + K_fT_Coeff[2]/TK + K_fT_Coeff[3]*log(TK) + K_fT_Coeff[4]/TK*TK +
                K_fT_Coeff[5]*TK*TK + K_fT_Coeff[6]/pow(TK,0.5);
         Hr = Rln10 * ( K_fT_Coeff[1]*TK*TK - K_fT_Coeff[2] + K_fT_Coeff[3]*TK -
            2.0*K_fT_Coeff[4]/TK + 2.0*K_fT_Coeff[5]*TK*TK*TK - 0.5*K_fT_Coeff[6]*pow(TK,0.5) );
         Sr = Rln10 * ( K_fT_Coeff[0] + 2.0*K_fT_Coeff[1]*TK + K_fT_Coeff[3]*(1.0 + log(TK))
            - K_fT_Coeff[4]/TK*TK + 3.0*K_fT_Coeff[5]*TK*TK + 0.5*K_fT_Coeff[6]/pow(TK,0.5) );

    auto th_param = thermoParameters();
    th_param.reaction_Cp_fT_coeff   = CpCoeff;
    th_param.reaction_logK_fT_coeff = K_fT_Coeff;
    setThermoParameters(th_param);

    ref_prop.reaction_entropy = Sr;  // In this case, everything will be inserted
    ref_prop.reaction_enthalpy = Hr;
    ref_prop.reaction_heat_capacity_cp = Cpr;
    ref_prop.ln_equilibrium_constant = lgKr * lg_to_ln;
    ref_prop.log_equilibrium_constant = lgKr;
    ref_prop.reaction_gibbs_energy = -Rln10*TK*lgKr;

    return ref_prop;
}

auto Reaction::convert_logKfT_toCpfT(/*MethodCorrT_Thrift::type methodT*/) -> ThermoPropertiesReaction
{

    auto Rln10      = R_CONSTANT * lg_to_ln;
    auto TK         = Reaktoro_::Temperature(pimpl->reference_T);
    auto CpCoeff    = pimpl->thermo_parameters.reaction_Cp_fT_coeff;
    auto K_fT_Coeff = pimpl->thermo_parameters.reaction_logK_fT_coeff;
    auto ref_prop   = pimpl->thermo_ref_prop;
    auto method_T   = pimpl->method_T;

    auto Sr = ref_prop.reaction_entropy;
//    auto Gr = ref_prop.reaction_gibbs_energy;
    auto Hr = ref_prop.reaction_enthalpy;
    auto Cpr = ref_prop.reaction_heat_capacity_cp;
    auto lgKr = ref_prop.ln_equilibrium_constant / lg_to_ln;

    switch( method_T )
    {
    case MethodCorrT_Thrift::type::CTM_LGK:
       /* Calc lgK, Hr and Sr */
        CpCoeff.resize(5);
        lgKr = K_fT_Coeff[0] + K_fT_Coeff[1]*TK + K_fT_Coeff[2]/TK + K_fT_Coeff[3]*log(TK) + K_fT_Coeff[4]/TK*TK +
                K_fT_Coeff[5]*TK*TK + K_fT_Coeff[6]/pow(TK,0.5);
        Hr = Rln10 * ( K_fT_Coeff[1]*TK*TK - K_fT_Coeff[2] + K_fT_Coeff[3]*TK -
                2.0*K_fT_Coeff[4]/TK + 2.0*K_fT_Coeff[5]*TK*TK*TK - 0.5*K_fT_Coeff[6]*pow(TK,0.5) );
        Sr = Rln10 * ( K_fT_Coeff[0] + 2.0*K_fT_Coeff[1]*TK + K_fT_Coeff[3]*(1.0 + log(TK))
                - K_fT_Coeff[4]/TK*TK + 3.0*K_fT_Coeff[5]*TK*TK + 0.5*K_fT_Coeff[6]/pow(TK,0.5) );

        CpCoeff[0] = Rln10 * K_fT_Coeff[3];
        CpCoeff[1] = Rln10 * 2.0 * K_fT_Coeff[1];
        CpCoeff[2] = Rln10 * 2.0 * K_fT_Coeff[4];
        CpCoeff[3] = -Rln10 * 0.25 * K_fT_Coeff[6];
        CpCoeff[4] = Rln10 * 6.0 * K_fT_Coeff[5];
        Cpr   = CpCoeff[0] + CpCoeff[1]*TK + CpCoeff[2]/TK*TK + CpCoeff[4]*TK*TK + CpCoeff[3]/pow(TK,0.5);

        ref_prop.reaction_entropy = Sr;  // In this case, everything will be inserted
        ref_prop.reaction_enthalpy = Hr;
        ref_prop.reaction_heat_capacity_cp = Cpr;
        ref_prop.ln_equilibrium_constant = lgKr * lg_to_ln;
        ref_prop.log_equilibrium_constant = lgKr;
        ref_prop.reaction_gibbs_energy = -Rln10*TK*lgKr;

    }

    auto th_param = thermoParameters();
    th_param.reaction_Cp_fT_coeff   = CpCoeff;
    th_param.reaction_logK_fT_coeff = K_fT_Coeff;
    setThermoParameters(th_param);

    return ref_prop;

}

auto Reaction::calc_logK_fT_coefficients() -> vd
{
    auto Rln10      = R_CONSTANT * lg_to_ln;
    auto TK         = Reaktoro_::Temperature(pimpl->reference_T);
    vd K_fT_Coeff   = {0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    auto ref_prop   = pimpl->thermo_ref_prop;
    auto method_T   = pimpl->method_T;

    auto Sr = ref_prop.reaction_entropy;
    auto Hr = ref_prop.reaction_enthalpy;
    auto Cpr = ref_prop.reaction_heat_capacity_cp;

    K_fT_Coeff[0] = ref_prop.log_equilibrium_constant.val;

    switch( method_T )
    { // calculation 2- and 3-term param approximation
    case MethodCorrT_Thrift::type::CTM_DKR: // 3-term extrap. from Franck-Marshall density model
    case MethodCorrT_Thrift::type::CTM_MRB: // 3-term extrap. from MRB at 25 C (provisional DK 06.08.07)
    case MethodCorrT_Thrift::type::CTM_LGK:  // Here, all logK-f(T) coeffs are given
    case MethodCorrT_Thrift::type::CTM_LGX:  // uncommented, 19.06.2008
        break;
    case MethodCorrT_Thrift::type::CTM_IKZ:  // Isotopic forms
        return K_fT_Coeff;
    case MethodCorrT_Thrift::type::CTM_EK0: // Generating 1-term extrapolation at logK = const
        K_fT_Coeff[0]=(Sr/Rln10).val;
        break;
    case MethodCorrT_Thrift::type::CTM_EK1: // Generating 1-term extrapolation at dGr = const
        K_fT_Coeff[0]=0.0;
        K_fT_Coeff[1]=0.0;
        K_fT_Coeff[2]=(-Hr/Rln10).val;
        break;
    case MethodCorrT_Thrift::type::CTM_EK2: // Generating 2-term (Vant Hoff) extrapolation
        K_fT_Coeff[0]=(Sr/Rln10).val;
        K_fT_Coeff[1]=0.0;
        K_fT_Coeff[2]=(-Hr/Rln10).val;
        break;
    case MethodCorrT_Thrift::type::CTM_PPE:
    case MethodCorrT_Thrift::type::CTM_EK3: // Generating 3-term extrapolation at constant dCpr
        K_fT_Coeff[0]=(( Sr - Cpr*(1.+log(TK)) ) / Rln10).val;
        K_fT_Coeff[1]=0.0;
        K_fT_Coeff[2]=(( Cpr*TK - Hr ) / Rln10).val;
//        if (lg10) // in phreeqc // lgK = A[0] + A[2]/T + A[3] * logT;
//            K_fT_Coeff[3]=(Cpr / R_CONSTANT).val;
//        else
            K_fT_Coeff[3]=(Cpr / Rln10).val;
        break;
//    default:
//        errorMethodNotFound("convert","logKfT to CpfT", __LINE__, __FILE__);
    }

    return K_fT_Coeff;

//    auto th_param = thermoParameters();
//    th_param.reaction_logK_fT_coeff = K_fT_Coeff;
//    setThermoParameters(th_param);
}



} // namespace ThermoFun

