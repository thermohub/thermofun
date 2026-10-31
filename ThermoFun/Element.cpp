// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include <iostream>
#include "Element.h"
#include "ChemicalFun/FormulaParser/ChemicalData.h"
#include "Common/Exception.h"
#include "IO/FormatDetect.h"
#include "IO/Legacy/LegacyReader.h"
#include "IO/V2/V2Reader.h"
#include "IO/Legacy/LegacyVocabulary.h"

namespace ThermoFun {

struct Element::Impl
{
    /// The record
    std::shared_ptr<const v2::ElementRecord> record;

    /// Changes a copy of the record: a reference to the old record stays valid.
    template <class Change>
    void edit(Change change)
    {
        auto copy = std::make_shared<v2::ElementRecord>(*record);
        change(*copy);
        copy->original_text.clear();
        record = copy;
    }

    /// Record in JSON string
    std::string jString;
};

Element::Element()
: pimpl(new Impl())
{
    pimpl->record = std::make_shared<const v2::ElementRecord>();
}

Element::Element(std::string jsonElement)
: pimpl(new Impl())
{
    Diagnostics diagnostics;
    if (v2::detectRecordFormat(jsonElement) == v2::DataFormat::V2)
    {
        pimpl->record = std::make_shared<const v2::ElementRecord>(v2::readV2Element(jsonElement, diagnostics));
        if (!diagnostics.ok())
            funError("The record cannot be read", diagnostics.toString(), __LINE__, __FILE__);
    }
    else
        pimpl->record = std::make_shared<const v2::ElementRecord>(v2::readLegacyElement(jsonElement, {}, &diagnostics));
    v2::checkReading(diagnostics);
    pimpl->jString = jsonElement;
}

Element::Element(std::shared_ptr<const v2::ElementRecord> record)
: pimpl(new Impl())
{
    pimpl->record = std::move(record);
    pimpl->jString = pimpl->record->original_text;
}

Element::Element(const Element& other)
: pimpl(new Impl(*other.pimpl))
{}

Element::Element(const ChemicalFun::ElementKey &elKey,
                 const ChemicalFun::ElementValues &elValues)
: pimpl(new Impl())
{
    pimpl->record = std::make_shared<const v2::ElementRecord>();
    setClass(elKey.Class());
    setIsotopeMass(elKey.Isotope());
    setSymbol(elKey.Symbol());
    setName(elValues.name);
    setMolarMass(elValues.atomic_mass);
    setEntropy(elValues.entropy);
    setHeatCapacity(elValues.heat_capacity);
    setVolume(elValues.volume);
    setValence(elValues.valence);
    setNumber(elValues.number);
}

Element::~Element()
{}

auto Element::operator=(Element other) -> Element&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto Element::setName(std::string name) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) { r.name = name; });
}

auto Element::setSymbol(std::string symbol) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) { r.symbol = symbol; });
}

auto Element::setMolarMass(double value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) {
        auto q = r.molar_mass.value_or(v2::Quantity{});
        q.value = value * 1e-3;
        q.value_as_written.clear();
        r.molar_mass = q;
    });
}

auto Element::setEntropy(double value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) {
        auto q = r.standard_entropy.value_or(v2::Quantity{});
        q.value = value;
        q.value_as_written.clear();
        r.standard_entropy = q;
    });
}

auto Element::setHeatCapacity(double value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) {
        auto q = r.heat_capacity.value_or(v2::Quantity{});
        q.value = value;
        q.value_as_written.clear();
        r.heat_capacity = q;
    });
}

auto Element::setVolume(double value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) {
        auto q = r.volume.value_or(v2::Quantity{});
        q.value = value * 1e-5;
        q.value_as_written.clear();
        r.volume = q;
    });
}

auto Element::setValence(int value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) { r.valence = value; });
}

auto Element::setClass(int value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) {
        r.class_ = elementClassFromLegacy(value);
        if (r.class_ == v2::ElementClass::Other)
            r.legacy_class = value;
        else
            r.legacy_class.reset();
    });
}

auto Element::setIsotopeMass(int value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) { r.isotope_mass = value; });
}

auto Element::setNumber(int value) -> void
{
    pimpl->edit([&](v2::ElementRecord& r) { r.number = value; });
}

auto Element::setJsonString(const std::string &jString) ->void
{
    pimpl->jString = jString;
}

auto Element::name() const -> std::string
{
    return pimpl->record->name;
}

auto Element::symbol() const -> std::string
{
    return pimpl->record->symbol;
}

auto Element::molarMass() const -> double
{
    return pimpl->record->molar_mass ? pimpl->record->molar_mass->value * 1e3 : 0.0;
}

auto Element::entropy() const -> double
{
    return pimpl->record->standard_entropy ? pimpl->record->standard_entropy->value : 0.0;
}

auto Element::heatCapacity() const -> double
{
    return pimpl->record->heat_capacity ? pimpl->record->heat_capacity->value : 0.0;
}

auto Element::volume() const -> double
{
    return pimpl->record->volume ? pimpl->record->volume->value * 1e5 : 0.0;
}

auto Element::valence() const -> int
{
    return pimpl->record->valence.value_or(777);
}

auto Element::class_() const -> int
{
    return pimpl->record->legacy_class.value_or(toLegacy(pimpl->record->class_));
}

auto Element::isotopeMass() const -> int
{
    return pimpl->record->isotope_mass.value_or(0);
}

auto Element::number() const -> int
{
    return pimpl->record->number.value_or(0);
}

auto Element::record() const -> std::shared_ptr<const v2::ElementRecord>
{
    return pimpl->record;
}

auto Element::jsonString() const -> std::string
{
    return pimpl->jString;
}

ChemicalFun::ElementKey Element::toElementKey(ChemicalFun::ElementValues &eldata)
{
    //    eldata.recid =;
    eldata.atomic_mass = molarMass();
    eldata.entropy = entropy();
    eldata.heat_capacity = heatCapacity();
    eldata.volume = volume();
    if(valence()==777)
    {
        setValence(ChemicalFun::DBElements::defaultValence(symbol()));
    }
    eldata.valence = valence();
    eldata.number = number();
    eldata.name = symbol(); // was e.name();

    return ChemicalFun::ElementKey(symbol(), class_(), isotopeMass());
}

auto operator<(const Element& lhs, const Element& rhs) -> bool
{
    if (lhs.symbol() < rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() < rhs.name())
            return true;
        if (lhs.name() == rhs.name())
        {
            if (lhs.class_() < rhs.class_())
                return true;
            if (lhs.class_() == rhs.class_())
            {
                if (lhs.isotopeMass() < rhs.isotopeMass())
                    return true;
                else
                    return (lhs.valence() < rhs.valence());
            }
        }
    }
    return false;
}

auto operator>(const Element& lhs, const Element& rhs) -> bool
{
    if (lhs.symbol() > rhs.symbol())
        return true;
    if (lhs.symbol() == rhs.symbol())
    {
        if (lhs.name() > rhs.name())
            return true;
        if (lhs.name() == rhs.name())
        {
            if (lhs.class_() > rhs.class_())
                return true;
            if (lhs.class_() == rhs.class_())
            {
                if (lhs.isotopeMass() > rhs.isotopeMass())
                    return true;
                else
                    return (lhs.valence() > rhs.valence());
            }
        }
    }
    return false;
}

auto operator==(const Element& lhs, const Element& rhs) -> bool
{
    return (lhs.symbol()        == rhs.symbol()) &&
           (lhs.name()          == rhs.name()) &&
           (lhs.class_()        == rhs.class_()) &&
           (lhs.isotopeMass()   == rhs.isotopeMass());
}


auto operator<<(std::ostream& stream, const Element& element) -> std::ostream&
{
    stream << "Element(\n";
    stream << "    name: " << element.name() << "\n";
    stream << "    symbol: " << element.symbol() << "\n";
    stream << "    molarMass: " << element.molarMass() << "\n";
    stream << "    entropy: " << element.entropy() << "\n";
    stream << "    heatCapacity: " << element.heatCapacity() << "\n";
    stream << "    volume: " << element.volume() << "\n";
    stream << "    valence: " << element.valence() << "\n";
    stream << "    class: " << element.class_() << "\n";
    stream << "    isotopeMass: " << element.isotopeMass() << "\n";
    stream << "    number: " << element.number() << "\n)" << std::endl;
    return stream;
}


} // namespace ThermoFun
