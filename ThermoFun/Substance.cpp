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
#include "ThermoPropertiesStatus.h"
#include "IO/FormatDetect.h"
#include "IO/Legacy/LegacyReader.h"
#include "IO/V2/V2Reader.h"

namespace ThermoFun {

struct Substance::Impl
{
    /// The record
    std::shared_ptr<const v2::SubstanceRecord> record;

    /// The old codes that a method of the record does not give back (for example CPM_NUL)
    compat::MethodCodes codes_without_method;

    /// The calculation type, if an old setter gave it
    std::optional<SubstanceThermoCalculationType::type> calculation_type;

    /// Arrays that an old setter gave and no method of the record reads yet: it can set them before the method
    ThermoParametersSubstance waiting_parameters;

    /// The text of the record. An edit does not change it, until the legacy writer exists (R4.4).
    std::string jString;

    /// Changes a copy of the record: a reference to the old record stays valid.
    template <class Change>
    void edit(Change change)
    {
        auto copy = std::make_shared<v2::SubstanceRecord>(*record);
        change(*copy);
        copy->original_text.clear();
        record = copy;
    }

    auto parameters() const -> ThermoParametersSubstance
    {
        auto out = compat::thermoParameters(*record);
        compat::addWaiting(out, waiting_parameters);
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

Substance::Substance(std::string jsonSubstance)
: pimpl(new Impl())
{
    Diagnostics diagnostics;
    if (v2::detectRecordFormat(jsonSubstance) == v2::DataFormat::V2)
    {
        pimpl->record = std::make_shared<const v2::SubstanceRecord>(v2::readV2Substance(jsonSubstance, diagnostics));
        if (!diagnostics.ok())
            funError("The record cannot be read", diagnostics.toString(), __LINE__, __FILE__);
    }
    else
        pimpl->record = std::make_shared<const v2::SubstanceRecord>(v2::readLegacySubstance(jsonSubstance, {}, &diagnostics));
    v2::checkReading(diagnostics);
    pimpl->jString = jsonSubstance;
}

Substance::Substance(std::shared_ptr<const v2::SubstanceRecord> record)
: pimpl(new Impl())
{
    pimpl->record = std::move(record);
    pimpl->jString = pimpl->record->original_text;
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
}

auto Substance::setSymbol(std::string symbol) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.symbol = symbol; });
}

auto Substance::setFormula(std::string formula) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.formula = formula; });
}

auto Substance::setReactionSymbol(std::string reaction) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.defining_reaction = reaction; });
}

auto Substance::setThermoReferenceProperties(ThermoPropertiesSubstance thermo_ref_prop) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { compat::setThermoReferenceProperties(r, thermo_ref_prop); });
}

auto Substance::setThermoParameters(ThermoParametersSubstance thermo_param) -> void
{
    pimpl->setMethods(pimpl->codes(), thermo_param);
    pimpl->waiting_parameters = thermo_param;
}

auto Substance::setMethodGenEoS(MethodGenEoS_Thrift::type method_genEoS) -> void
{
    auto codes = pimpl->codes();
    codes.general = method_genEoS;
    pimpl->setMethods(codes, pimpl->parameters());
}

auto Substance::setMethod_T(MethodCorrT_Thrift::type method_T) -> void
{
    auto codes = pimpl->codes();
    codes.temperature = method_T;
    pimpl->setMethods(codes, pimpl->parameters());
}

auto Substance::setMethod_P(MethodCorrP_Thrift::type method_P) -> void
{
    auto codes = pimpl->codes();
    codes.pressure = method_P;
    pimpl->setMethods(codes, pimpl->parameters());
}

auto Substance::setSubstanceClass(SubstanceClass::type substance_class) -> void
{
    // the rules of the methods read the class
    const auto codes = pimpl->codes();
    const auto parameters = pimpl->parameters();
    pimpl->edit([&](v2::SubstanceRecord& r) { r.class_ = substanceClassFromLegacy(substance_class); });
    if (!pimpl->record->methods.empty())
        pimpl->setMethods(codes, parameters);
}

auto Substance::setAggregateState(AggregateState::type aggregate_state) -> void
{
    // the rules of the methods read the state
    const auto codes = pimpl->codes();
    const auto parameters = pimpl->parameters();
    pimpl->edit([&](v2::SubstanceRecord& r) { r.aggregate_state = aggregateStateFromLegacy(aggregate_state); });
    if (!pimpl->record->methods.empty())
        pimpl->setMethods(codes, parameters);
}

auto Substance::setThermoCalculationType(SubstanceThermoCalculationType::type calculation_type) -> void
{
    pimpl->calculation_type = calculation_type;
    if (calculation_type == SubstanceThermoCalculationType::type::REACDC)
        pimpl->edit([&](v2::SubstanceRecord& r) { r.preferred_property_source = v2::PropertySource::Reaction; });
}

auto Substance::setCharge(int charge) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.charge = charge; });
}

auto Substance::setReferenceT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.reference_conditions.T = T; });
}

auto Substance::setReferenceP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.reference_conditions.P = P; });
}

auto Substance::setLowerT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.t_min = T; });
}

auto Substance::setLowerP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.p_min = P; });
}

auto Substance::setUpperT(double T) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.t_max = T; });
}

auto Substance::setUpperP(double P) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) { r.applicability.p_max = P; });
}

auto Substance::setMolarMass(double molar_mass) -> void
{
    pimpl->edit([&](v2::SubstanceRecord& r) {
        auto q = r.molar_mass.value_or(v2::Quantity{});
        q.value = molar_mass * 1e-3;
        q.value_as_written.clear();
        r.molar_mass = q;
    });
}

auto Substance::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
}

auto Substance::name() const -> std::string
{
    return pimpl->record->name;
}

auto Substance::symbol() const -> std::string
{
    return pimpl->record->symbol;
}

auto Substance::formula() const -> std::string
{
    return pimpl->record->formula;
}

auto Substance::reactionSymbol() const -> std::string
{
    return pimpl->record->defining_reaction;
}

auto Substance::molarMass() const -> double
{
    return compat::molarMass(*pimpl->record);
}

auto Substance::referenceT() const -> double
{
    return pimpl->record->reference_conditions.T;
}

auto Substance::referenceP() const -> double
{
    return pimpl->record->reference_conditions.P;
}

auto Substance::lowerT( ) const -> double
{
    return pimpl->record->applicability.t_min.value_or(0.0);
}

auto Substance::lowerP( ) const -> double
{
    return pimpl->record->applicability.p_min.value_or(0.0);
}

auto Substance::upperT( ) const -> double
{
    return pimpl->record->applicability.t_max.value_or(0.0);
}

auto Substance::upperP() const -> double
{
    return pimpl->record->applicability.p_max.value_or(0.0);
}

auto Substance::thermoProperties() const -> ThermoPropertiesSubstance
{
    return {};
}

auto Substance::thermoParameters() const -> ThermoParametersSubstance
{
    return pimpl->parameters();
}

auto Substance::thermoReferenceProperties() const -> ThermoPropertiesSubstance
{
    return compat::thermoReferenceProperties(*pimpl->record);
}

auto Substance::methodGenEOS() const -> MethodGenEoS_Thrift::type
{
    return pimpl->codes().general;
}

auto Substance::method_T() const -> MethodCorrT_Thrift::type
{
    return pimpl->codes().temperature;
}

auto Substance::method_P() const -> MethodCorrP_Thrift::type
{
    return pimpl->codes().pressure;
}

auto Substance::substanceClass() const -> SubstanceClass::type
{
    return toLegacy(pimpl->record->class_);
}

auto Substance::thermoCalculationType() const -> SubstanceThermoCalculationType::type
{
    return pimpl->calculation_type.value_or(compat::thermoCalculationType(*pimpl->record));
}

auto Substance::aggregateState() const -> AggregateState::type
{
    return toLegacy(pimpl->record->aggregate_state);
}

auto Substance::charge() const -> int
{
    return pimpl->record->charge.value_or(0);
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
    if (upperP()<(P) || upperT()<(T) ||
        lowerP()>(P) || lowerT()>(T))
    {
        std::string message = modelName +": out of "
                                    "T(" + std::to_string(lowerT()) + "-" + std::to_string(upperT()) +" K) and "
                                    "P(" + std::to_string(lowerP()) + "-" + std::to_string(upperP()) +" Pa) bounds";

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

