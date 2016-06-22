#include "Substance.h"

// TCorrPT includes
//#include "ThermoProperties.h"

namespace TCorrPT {

struct Substance::Impl
{
    /// The name of the chemical Substance
    std::string name;

    /// The name of the chemical Substance
    std::string symbol;

    /// The chemical formula of the chemical Substance
    std::string formula;

    /// The name of the reaction that defines the properties of this substance
    std::string reaction;

    /// The molar mass of the chemical Substance (in units of g/mol)
    double molar_mass;

    /// Reference temperature (in K)
    double reference_T;

    /// Reference pressure (in Pa)
    double reference_P;

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

    string solventSymbol;

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

auto Substance::setReaction(std::string reaction) -> void
{
    pimpl->reaction = reaction;
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

auto Substance::setSolventSymbol(string symbol) -> void
{
    pimpl->solventSymbol = symbol;
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

auto Substance::reaction() const -> std::string
{
    return pimpl->reaction;
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

auto Substance::charge() -> int
{
    return pimpl->charge;
}

auto Substance::SolventSymbol() -> string
{
    return pimpl->solventSymbol;
}

} // namespace TCorrPT

