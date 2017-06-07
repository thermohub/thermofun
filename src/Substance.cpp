#include "Substance.h"

// ThermoFun includes
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

struct Substance::Impl
{
    /// The name of the chemical Substance
    std::string name;

    /// The name of the chemical Substance
    std::string symbol;

    /// The chemical formula of the chemical Substance
    std::string formula;

    /// The symbol of the reaction that defines the properties of this substance
    std::string reactionSymbol;

    /// The molar mass of the chemical Substance (in units of g/mol)
    double molar_mass;

    /// Reference temperature (in K)
    double reference_T;

    /// Reference pressure (in Pa)
    double reference_P;

    /// Lower temperature limit (in K)
    double lower_T;

    /// Lower pressure limit (in Pa)
    double lower_P;

    /// Upper temperature limit (in K)
    double upper_T;

    /// Upper pressure limit (in Pa)
    double upper_P;

    /// General method (or equation of state for both T and P correction)
    MethodGenEoS_Thrift::type method_genEoS;

    /// Method of temperature correction of thermodynamic properties
    MethodCorrT_Thrift::type  method_T;

    /// Method of pressure correction of thermodynamic properties
    MethodCorrP_Thrift::type  method_P;

    /// Class of the substance  COMPONENT, GASFLUID, AQSOLUTE, AQSOLVENT, SURFSPECIES, SITEMOIETY, OTHER_SC
    SubstanceClass::type substance_class;

    /// Agregate state of the substance
    AggregateState::type aggregate_state;

    /// Type of calculation used for determining the substance properties
    /// using a defined PT correction model or from a reaction
    SubstanceThermoCalculationType::type thermo_calculation_type;

    /// Thermodynamic properties of the substance at temperature and pressure
    /// and at reference tempearature and reference presure (usually 298.15 K and 1e05 Pa)
    ThermoPropertiesSubstance thermo_prop, thermo_ref_prop;

    /// Parameters used to calculate the thermodynamic properties of the substance at the desired
    /// temperature and pressure using a certain calculation model
    ThermoParametersSubstance thermo_param;

    int charge;

//    /// Concentration scale used in defining the properties of the substance
//    ConcentrationScales::type StdConcentration_scale;
};

Substance::Substance()
: pimpl(new Impl())
{}

Substance::Substance(const Substance& other)
: pimpl(new Impl(*other.pimpl))
{}

Substance::~Substance()
{}

auto Substance::operator=(Substance other) -> Substance&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto Substance::setName(std::string name) -> void
{
    pimpl->name = name;
}

auto Substance::setSymbol(std::string symbol) -> void
{
    pimpl->symbol = symbol;
}

auto Substance::setFormula(std::string formula) -> void
{
    pimpl->formula = formula;
}

auto Substance::setReactionSymbol(std::string reaction) -> void
{
    pimpl->reactionSymbol = reaction;
}

auto Substance::setThermoReferenceProperties(ThermoPropertiesSubstance thermo_ref_prop) -> void
{
    pimpl->thermo_ref_prop = thermo_ref_prop;
}

auto Substance::setThermoParameters(ThermoParametersSubstance thermo_param) -> void
{
    pimpl->thermo_param = thermo_param;
}

auto Substance::setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void
{
    pimpl->method_genEoS = method_genEoS;
}

auto Substance::setMethod_T(MethodCorrT_Thrift::type method_T) -> void
{
    pimpl->method_T = method_T;
}

auto Substance::setMethod_P(MethodCorrP_Thrift::type method_P) -> void
{
    pimpl->method_P = method_P;
}

auto Substance::setSubstanceClass(SubstanceClass::type substance_class) -> void
{
    pimpl->substance_class = substance_class;
}

auto Substance::setAggregateState(AggregateState::type aggregate_state) -> void
{
    pimpl->aggregate_state = aggregate_state;
}

auto Substance::setThermoCalculationType(SubstanceThermoCalculationType::type calculation_type) -> void
{
    pimpl->thermo_calculation_type = calculation_type;
}

auto Substance::setCharge(int charge) -> void
{
    pimpl->charge = charge;
}

auto Substance::setReferenceT(double T) -> void
{
    pimpl->reference_T = T;
}

auto Substance::setReferenceP(double P) -> void
{
    pimpl->reference_P = P;
}

auto Substance::setLowerT(double T) -> void
{
    pimpl->lower_T = T;
}

auto Substance::setLowerP(double P) -> void
{
    pimpl->lower_P = P;
}

auto Substance::setUpperT(double T) -> void
{
    pimpl->upper_T = T;
}

auto Substance::setUpperP(double P) -> void
{
    pimpl->upper_P = P;
}

auto Substance::setMolarMass(double molar_mass) -> void
{
    pimpl->molar_mass = molar_mass;
}

auto Substance::name() const -> std::string
{
    return pimpl->name;
}

auto Substance::symbol() const -> std::string
{
    return pimpl->symbol;
}

auto Substance::formula() const -> std::string
{
    return pimpl->formula;
}

auto Substance::reactionSymbol() const -> std::string
{
    return pimpl->reactionSymbol;
}

auto Substance::molarMass() const -> double
{
    return pimpl->molar_mass;
}

auto Substance::referenceT() const -> double
{
    return pimpl->reference_T;
}

auto Substance::referenceP() const -> double
{
    return pimpl->reference_P;
}

auto Substance::lowerT( ) const -> double
{
    return pimpl->lower_T;
}

auto Substance::lowerP( ) const -> double
{
    return pimpl->lower_P;
}

auto Substance::upperT( ) const -> double
{
    return pimpl->upper_T;
}

auto Substance::upperP() const -> double
{
    return pimpl->upper_P;
}

auto Substance::thermoProperties() -> ThermoPropertiesSubstance
{
    return pimpl->thermo_prop;
}

auto Substance::thermoParameters() -> ThermoParametersSubstance
{
    return pimpl->thermo_param;
}

auto Substance::thermoReferenceProperties() -> ThermoPropertiesSubstance
{
    return pimpl->thermo_ref_prop;
}

auto Substance::methodGenEOS() -> MethodGenEoS_Thrift::type
{
    return pimpl->method_genEoS;
}

auto Substance::method_T() -> MethodCorrT_Thrift::type
{
    return pimpl->method_T;
}

auto Substance::method_P() -> MethodCorrP_Thrift::type
{
    return pimpl->method_P;
}

auto Substance::substanceClass() -> SubstanceClass::type
{
    return pimpl->substance_class;
}

auto Substance::thermoCalculationType() -> SubstanceThermoCalculationType::type
{
    return pimpl->thermo_calculation_type;
}

auto Substance::aggregateState() -> AggregateState::type
{
    return pimpl->aggregate_state;
}

auto Substance::charge() const -> int
{
    return pimpl->charge;
}

auto Substance::checkCalcMethodBounds(string modelName, double T, double P, ThermoPropertiesSubstance &tps) -> void
{
    if (pimpl->upper_P<(P*bar_to_Pa) || pimpl->upper_T<(T+C_to_K) ||
        pimpl->lower_P>(P*bar_to_Pa) || pimpl->lower_T>(T+C_to_K))
    {
        string message = modelName +": out of "
                                    "T(" + to_string(pimpl->lower_T) + "-" + to_string(pimpl->upper_T) +" K) and "
                                    "P(" + to_string(pimpl->lower_P) + "-" + to_string(pimpl->upper_P) +" Pa) bounds";

        setMessage(Reaktoro_::Status::calculated, message, tps );
    }
}

} // namespace ThermoFun

