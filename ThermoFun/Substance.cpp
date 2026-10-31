// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Substance.h"

#include <optional>

#include "Compat/SubstanceCompat.h"
#include "IO/Legacy/LegacyVocabulary.h"

// ThermoFun includes
#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"
#include "Common/ParseJsonToData.h"
#include "ThermoPropertiesStatus.h"
#include "IO/Legacy/LegacyReader.h"

namespace ThermoFun {

struct Substance::Impl
{
    /// The record. Empty only for the old parser (withoutRecord).
    std::shared_ptr<const v2::SubstanceRecord> record;

    /// The old codes that a method of the record does not give back (for example CPM_NUL)
    compat::MethodCodes codes_without_method;

    /// The calculation type, if an old setter gave it
    std::optional<SubstanceThermoCalculationType::type> calculation_type;

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

    /// Changes a copy of the record: a reference to the old record stays valid.
    template <class Change>
    void edit(Change change)
    {
        if (!record)
            return;
        auto copy = std::make_shared<v2::SubstanceRecord>(*record);
        change(*copy);
        copy->original_text.clear();
        record = copy;
    }

    auto parameters() const -> ThermoParametersSubstance
    {
        auto out = compat::thermoParameters(*record);
        compat::addWaiting(out, thermo_param);
        return out;
    }

    auto codes() const -> compat::MethodCodes
    {
        auto out = compat::methodCodes(*record);
        if (codes_without_method.general)
            out.general = codes_without_method.general;
        if (codes_without_method.temperature)
            out.temperature = codes_without_method.temperature;
        if (codes_without_method.pressure)
            out.pressure = codes_without_method.pressure;
        return out;
    }

    /// Replaces the methods. A code that a method does not give back is kept beside the record, for the getters.
    void setMethods(const compat::MethodCodes& wanted, const ThermoParametersSubstance& parameters)
    {
        edit([&](v2::SubstanceRecord& r) {
            compat::setMethods(r, wanted, parameters);
            if (calculation_type == SubstanceThermoCalculationType::type::REACDC)
                r.preferred_property_source = v2::PropertySource::Reaction;
            const auto held = compat::methodCodes(r);
            codes_without_method = {};
            if (wanted.general != held.general)
                codes_without_method.general = wanted.general;
            if (wanted.temperature != held.temperature)
                codes_without_method.temperature = wanted.temperature;
            if (wanted.pressure != held.pressure)
                codes_without_method.pressure = wanted.pressure;
        });
    }

//    /// Concentration scale used in defining the properties of the substance
//    ConcentrationScales::type StdConcentration_scale;
};

Substance::Substance()
: pimpl(new Impl())
{
    // the old defaults: the codes 0
    v2::SubstanceRecord empty;
    empty.class_ = substanceClassFromLegacy(0);
    empty.aggregate_state = aggregateStateFromLegacy(0);
    pimpl->record = std::make_shared<const v2::SubstanceRecord>(empty);
}

auto Substance::withoutRecord() -> Substance
{
    Substance s;
    s.pimpl->record.reset();
    return s;
}

Substance::Substance(std::string jsonSubstance)
: pimpl(new Impl())
{
    Substance other = parseSubstance(jsonSubstance);
    pimpl = std::move(other.pimpl);
    setJsonString(jsonSubstance);
    pimpl->thermo_param = {};   // the record holds what a method reads
    pimpl->record = std::make_shared<const v2::SubstanceRecord>(v2::readLegacySubstance(jsonSubstance));
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
    pimpl->edit([&](v2::SubstanceRecord& r) { r.name = name; });
    pimpl->name = name;
}

auto Substance::setSymbol(std::string symbol) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.symbol = symbol; });
    pimpl->symbol = symbol;
}

auto Substance::setFormula(std::string formula) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.formula = formula; });
    pimpl->formula = formula;
}

auto Substance::setReactionSymbol(std::string reaction) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.defining_reaction = reaction; });
    pimpl->reactionSymbol = reaction;
}

auto Substance::setThermoReferenceProperties(ThermoPropertiesSubstance thermo_ref_prop) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { compat::setThermoReferenceProperties(r, thermo_ref_prop); });
    pimpl->thermo_ref_prop = thermo_ref_prop;
}

auto Substance::setThermoParameters(ThermoParametersSubstance thermo_param) -> void
{
    if (pimpl->record)
        pimpl->setMethods(pimpl->codes(), thermo_param);
    pimpl->thermo_param = thermo_param;
}

auto Substance::setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void
{
    if (pimpl->record)
    {
        auto codes = pimpl->codes();
        codes.general = method_genEoS;
        pimpl->setMethods(codes, pimpl->parameters());
    }
    pimpl->method_genEoS = method_genEoS;
}

auto Substance::setMethod_T(MethodCorrT_Thrift::type method_T) -> void
{
    if (pimpl->record)
    {
        auto codes = pimpl->codes();
        codes.temperature = method_T;
        pimpl->setMethods(codes, pimpl->parameters());
    }
    pimpl->method_T = method_T;
}

auto Substance::setMethod_P(MethodCorrP_Thrift::type method_P) -> void
{
    if (pimpl->record)
    {
        auto codes = pimpl->codes();
        codes.pressure = method_P;
        pimpl->setMethods(codes, pimpl->parameters());
    }
    pimpl->method_P = method_P;
}

auto Substance::setSubstanceClass(SubstanceClass::type substance_class) -> void
{
    if (pimpl->record)
    {
        // the rules of the methods read the class
        const auto codes = pimpl->codes();
        const auto parameters = pimpl->parameters();
        pimpl->edit([&](v2::SubstanceRecord& r) { r.class_ = substanceClassFromLegacy(substance_class); });
        if (!pimpl->record->methods.empty())
            pimpl->setMethods(codes, parameters);
    }
    pimpl->substance_class = substance_class;
}

auto Substance::setAggregateState(AggregateState::type aggregate_state) -> void
{
    if (pimpl->record)
    {
        // the rules of the methods read the state
        const auto codes = pimpl->codes();
        const auto parameters = pimpl->parameters();
        pimpl->edit([&](v2::SubstanceRecord& r) { r.aggregate_state = aggregateStateFromLegacy(aggregate_state); });
        if (!pimpl->record->methods.empty())
            pimpl->setMethods(codes, parameters);
    }
    pimpl->aggregate_state = aggregate_state;
}

auto Substance::setThermoCalculationType(SubstanceThermoCalculationType::type calculation_type) -> void
{
    if (pimpl->record)
    {
        pimpl->calculation_type = calculation_type;
        if (calculation_type == SubstanceThermoCalculationType::type::REACDC)
            pimpl->edit([&](v2::SubstanceRecord& r) { r.preferred_property_source = v2::PropertySource::Reaction; });
    }
    pimpl->thermo_calculation_type = calculation_type;
}

auto Substance::setCharge(int charge) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.charge = charge; });
    pimpl->charge = charge;
}

auto Substance::setReferenceT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.reference_conditions.T = T; });
    pimpl->reference_T = T;
}

auto Substance::setReferenceP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.reference_conditions.P = P; });
    pimpl->reference_P = P;
}

auto Substance::setLowerT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.t_min = T; });
    pimpl->lower_T = T;
}

auto Substance::setLowerP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.p_min = P; });
    pimpl->lower_P = P;
}

auto Substance::setUpperT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.t_max = T; });
    pimpl->upper_T = T;
}

auto Substance::setUpperP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.p_max = P; });
    pimpl->upper_P = P;
}

auto Substance::setMolarMass(double molar_mass) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) {
        auto q = r.molar_mass.value_or(v2::Quantity{});
        q.value = molar_mass * 1e-3;
        q.value_as_written.clear();
        r.molar_mass = q;
    });
    pimpl->molar_mass = molar_mass;
}

auto Substance::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
}

auto Substance::name() const -> std::string
{
    if (pimpl->record)
        return pimpl->record->name;
    return pimpl->name;
}

auto Substance::symbol() const -> std::string
{
    if (pimpl->record)
        return pimpl->record->symbol;
    return pimpl->symbol;
}

auto Substance::formula() const -> std::string
{
    if (pimpl->record)
        return pimpl->record->formula;
    return pimpl->formula;
}

auto Substance::reactionSymbol() const -> std::string
{
    if (pimpl->record)
        return pimpl->record->defining_reaction;
    return pimpl->reactionSymbol;
}

auto Substance::molarMass() const -> double
{
    if (pimpl->record)
        return compat::molarMass(*pimpl->record);
    return pimpl->molar_mass;
}

auto Substance::referenceT() const -> double
{
    if (pimpl->record)
        return pimpl->record->reference_conditions.T;
    return pimpl->reference_T;
}

auto Substance::referenceP() const -> double
{
    if (pimpl->record)
        return pimpl->record->reference_conditions.P;
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
    if (pimpl->record)
        return pimpl->parameters();
    return pimpl->thermo_param;
}

auto Substance::thermoReferenceProperties() const -> ThermoPropertiesSubstance
{
    if (pimpl->record)
        return compat::thermoReferenceProperties(*pimpl->record);
    return pimpl->thermo_ref_prop;
}

auto Substance::methodGenEOS() const -> MethodGenEoS_Thrift::type
{
    if (pimpl->record)
        return pimpl->codes().general;
    return pimpl->method_genEoS;
}

auto Substance::method_T() const -> MethodCorrT_Thrift::type
{
    if (pimpl->record)
        return pimpl->codes().temperature;
    return pimpl->method_T;
}

auto Substance::method_P() const -> MethodCorrP_Thrift::type
{
    if (pimpl->record)
        return pimpl->codes().pressure;
    return pimpl->method_P;
}

auto Substance::substanceClass() const -> SubstanceClass::type
{
    if (pimpl->record)
        return toLegacy(pimpl->record->class_);
    return pimpl->substance_class;
}

auto Substance::thermoCalculationType() const -> SubstanceThermoCalculationType::type
{
    if (pimpl->record)
        return pimpl->calculation_type.value_or(compat::thermoCalculationType(*pimpl->record));
    return pimpl->thermo_calculation_type;
}

auto Substance::aggregateState() const -> AggregateState::type
{
    if (pimpl->record)
        return toLegacy(pimpl->record->aggregate_state);
    return pimpl->aggregate_state;
}

auto Substance::charge() const -> int
{
    return pimpl->charge;
}

auto Substance::record() const -> std::shared_ptr<const v2::SubstanceRecord>
{
    return pimpl->record;
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

auto lowerTemperatureBound(const Substance& subst, const std::string& model) -> double
{
    const auto& intervals = subst.thermoParameters().temperature_intervals;
    // thermoParamSubst() pushes one entry per cp_ft_equation TPMethod, in TPMethods JSON order,
    // and pushes an empty entry for one with no limitsTP - so an empty entry can be followed by
    // a populated one from a later cp_ft_equation. Scan all of them rather than trusting front().
    for (const auto& interval : intervals)
        if (!interval.empty())
            return interval.front();

    // No cp_ft_equation method on this substance, so no T interval was ever recorded. Fall back
    // to the substance's own reference temperature (Tst) rather than an arbitrary constant: the
    // record's S298/CP298/etc (and so its whole standard-state description) are only actually
    // valid AT that temperature, so it is the correct "no better information" choice - referenceT()
    // already defaults to 298.15 K even when the source record left Tst unset.
    double fallback = subst.referenceT();
    thfun_logger->warn("{}: substance \"{}\" has no temperature intervals (they come only from a "
                       "cp_ft_equation method); using its reference temperature {} K as the lower "
                       "temperature bound.", model, subst.symbol(), fallback);
    return fallback;
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

