#include "Substance.h"

// ThermoFun includes
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/ParseJsonToData.h"

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

    /// Reference temperature (in K, 298.15 default)
    double reference_T = 298.15;

    /// Reference pressure (in Pa, 1e5 Pa default)
    double reference_P = 1e5;

    /// Lower temperature limit (in K)
    double lower_T;

    /// Lower pressure limit (in Pa)
    double lower_P;

    /// Upper temperature limit (in K)
    double upper_T;

    /// Upper pressure limit (in Pa)
    double upper_P;

    /// list of Temperature and pressure correction methods in the order they will be executed
    listmethods TPMethods;

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

    std::string jString;

//    /// Concentration scale used in defining the properties of the substance
//    ConcentrationScales::type StdConcentration_scale;
};

Substance::Substance()
: pimpl(new Impl())
{}

Substance::Substance(std::string jsonSubstance)
: pimpl(new Impl())
{
    Substance other = parseSubstance(jsonSubstance);
    pimpl = std::move(other.pimpl);
    setJsonString(jsonSubstance);
}

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

auto Substance::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
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

auto Substance::thermoProperties() const -> ThermoPropertiesSubstance
{
    return pimpl->thermo_prop;
}

auto Substance::thermoParameters() const -> ThermoParametersSubstance
{
    return pimpl->thermo_param;
}

auto Substance::thermoReferenceProperties() const -> ThermoPropertiesSubstance
{
    return pimpl->thermo_ref_prop;
}

auto Substance::methodGenEOS() const -> MethodGenEoS_Thrift::type
{
    return pimpl->method_genEoS;
}

auto Substance::method_T() const -> MethodCorrT_Thrift::type
{
    return pimpl->method_T;
}

auto Substance::method_P() const -> MethodCorrP_Thrift::type
{
    return pimpl->method_P;
}

auto Substance::substanceClass() const -> SubstanceClass::type
{
    return pimpl->substance_class;
}

auto Substance::thermoCalculationType() const -> SubstanceThermoCalculationType::type
{
    return pimpl->thermo_calculation_type;
}

auto Substance::aggregateState() const -> AggregateState::type
{
    return pimpl->aggregate_state;
}

auto Substance::charge() const -> int
{
    return pimpl->charge;
}

auto Substance::jsonString() const -> std::string
{
    return pimpl->jString;
}

auto Substance::checkCalcMethodBounds(std::string modelName, double T, double P, ThermoPropertiesSubstance &tps) -> void
{
    if (pimpl->upper_P<(P) || pimpl->upper_T<(T) ||
        pimpl->lower_P>(P) || pimpl->lower_T>(T))
    {
        std::string message = modelName +": out of "
                                    "T(" + std::to_string(pimpl->lower_T) + "-" + std::to_string(pimpl->upper_T) +" K) and "
                                    "P(" + std::to_string(pimpl->lower_P) + "-" + std::to_string(pimpl->upper_P) +" Pa) bounds";

        setMessage(Reaktoro_::Status::calculated, message, tps );
    }
}

auto operator<(const Substance& lhs, const Substance& rhs) -> bool
{
    if (lhs.symbol() < rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() < rhs.name())
            return true;
    }
    return false;
}

auto operator>(const Substance& lhs, const Substance& rhs) -> bool
{
    if (lhs.symbol() > rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() > rhs.name())
            return true;
    }
    return false;
}

auto operator==(const Substance& lhs, const Substance& rhs) -> bool
{
    return (lhs.symbol()        == rhs.symbol()) &&
           (lhs.name()          == rhs.name());
}


auto operator<<(std::ostream& stream, const ThermoPropertiesSubstance& data) -> std::ostream&
{
    stream << "( " << data.gibbs_energy << ", " << data.helmholtz_energy << ", "
           << data.internal_energy << ", "<< data.enthalpy << ", "
           << data.entropy << ", "<< data.volume << ", "
           << data.heat_capacity_cp << ", "<< data.heat_capacity_cv << " )";
    return stream;
}

auto operator<<(std::ostream& stream, const ThermoParametersSubstance& data) -> std::ostream&
{
    stream << "( " << data.isothermal_compresibility << ", " << data.isobaric_expansivity << " )\n";
    if( !data.temperature_intervals.empty() ) {
       stream << "        temperature_intervals: " << data.temperature_intervals << "\n";
    }
    if( !data.pressure_intervals.empty() ) {
       stream << "        pressure_intervals: " << data.pressure_intervals << "\n";
    }
    if( !data.Cp_coeff.empty() ) {
       stream << "        Cp_coeff: " << data.Cp_coeff << "\n";
    }
    if( !data.Cp_nonElectrolyte_coeff.empty() ) {
       stream << "        Cp_nonElectrolyte_coeff: " << data.Cp_nonElectrolyte_coeff << "\n";
    }
    if( !data.phase_transition_prop.empty() ) {
       stream << "        phase_transition_prop: " << data.phase_transition_prop << "\n";
    }
    if( !data.phase_transition_prop_Berman.empty() ) {
       stream << "        phase_transition_prop_Berman: " << data.phase_transition_prop_Berman << "\n";
    }
    if( !data.m_landau_phase_trans_props.empty() ) {
       stream << "        m_landau_phase_trans_props: " << data.m_landau_phase_trans_props << "\n";
    }
    if( !data.HKF_parameters.empty() ) {
       stream << "        HKF_parameters: " << data.HKF_parameters << "\n";
    }
    if( !data.volume_coeff.empty() ) {
       stream << "        volume_coeff: " << data.volume_coeff << "\n";
    }
    if( !data.critical_parameters.empty() ) {
       stream << "        critical_parameters: " << data.critical_parameters << "\n";
    }
    if( !data.volume_BirchM_coeff.empty() ) {
       stream << "        volume_BirchM_coeff: " << data.volume_BirchM_coeff << "\n";
    }
    if( !data.empirical_coeff.empty() ) {
       stream << "        empirical_coeff: " << data.empirical_coeff << "\n";
    }
    if( !data.solute_holland_powell98_coeff.empty() ) {
       stream << "        solute_holland_powell98_coeff: " << data.solute_holland_powell98_coeff << "\n";
    }
    return stream;
}

auto operator<<(std::ostream& stream, const Substance& subst) -> std::ostream&
{
    stream << "Substance(\n";
    stream << "    name: " << subst.name() << "\n";
    stream << "    symbol: " << subst.symbol() << "\n";
    stream << "    formula: " << subst.formula() << "\n";
    stream << "    reactionSymbol: " << subst.reactionSymbol() << "\n";
    stream << "    molarMass: " << subst.molarMass() << "\n";
    stream << "    T: ( " << subst.referenceT() << ", " << subst.lowerT() << ", " << subst.upperT() << " )\n";
    stream << "    P: ( " << subst.referenceP() << ", " << subst.lowerP() << ", " << subst.upperP() << " )\n";
    stream << "    thermoProperties: " << subst.thermoProperties() << "\n";
    stream << "    thermoParameters: " << subst.thermoParameters();
    stream << "    thermoReferenceProperties: " << subst.thermoReferenceProperties() << "\n";
    stream << "    methodGenEOS: " << subst.methodGenEOS() << "\n";
    stream << "    method_T: " << subst.method_T() << "\n";
    stream << "    method_P: " << subst.method_P() << "\n";
    stream << "    substanceClass: " << subst.substanceClass() << "\n";
    stream << "    thermoCalculationType: " << subst.thermoCalculationType() << "\n";
    stream << "    aggregateState: " << subst.aggregateState() << "\n";
    stream << "    charge: " << subst.charge() << "\n)" << std::endl;
    return stream;
}

} // namespace ThermoFun

