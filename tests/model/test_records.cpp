// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of the record types of Model/Records.h: default values, copies and equality.

#include "Model/Records.h"
#include "check.h"

using namespace ThermoFun::v2;

namespace {

void testDefaults()
{
    SubstanceRecord s;
    CHECK(s.class_ == SubstanceClass::Other);
    CHECK(s.aggregate_state == AggregateState::Other);
    CHECK(s.reference_conditions.T == 298.15);
    CHECK(s.reference_conditions.P == 1e5);
    CHECK(!s.charge.has_value());
    CHECK(!s.molar_mass.has_value());
    CHECK(s.source_format == DataFormat::Legacy);

    ReactionRecord r;
    CHECK(r.level == 0);
    CHECK(r.stoichiometry.empty());

    ElementRecord e;
    CHECK(e.class_ == ElementClass::Element);

    ThermoDataSetInfo info;
    CHECK(info.status == DatasetStatus::Draft);
    CHECK(info.water_triple_point_reference == "HKF_HGK_1974");

    MethodDefinition m;
    CHECK(m.kind == MethodKind::Reference);
    CHECK(m.operates_on == OperatesOn::Substance);
    CHECK(m.tier == 2);

    Applicability a;
    CHECK(!a.t_min && !a.t_max && !a.p_min && !a.p_max);
    CHECK(!a.is_hard);
}

void testEquality()
{
    SubstanceRecord a;
    a.symbol = "NaCl@";
    a.standard_state_properties[PropertyName::G] = Quantity{-393000.0, "kJ/mol", "", std::nullopt, {}};
    MethodApplication m;
    m.method = "HKF88";
    m.implementation = "gems";
    m.interpolation = Interpolation::CubicSpline;
    m.parameters["a1"].value = 1.5;
    a.methods.push_back(m);

    SubstanceRecord b = a;  // a copy is equal
    CHECK(a == b);

    b.methods[0].parameters["a1"].value = 1.6;
    CHECK(!(a == b));

    b = a;
    b.standard_state_properties[PropertyName::G].uncertainty = Uncertainty{};
    CHECK(!(a == b));

    b = a;
    b.convention = Convention{};
    CHECK(!(a == b));  // an empty convention is not the same as none

    b = a;
    b.methods[0].implementation = "reaktoro";
    CHECK(!(a == b));

    b = a;
    b.covariance.push_back({{"/standard_state_properties/G"}, {4.0}});
    CHECK(!(a == b));

    b = a;
    b.preferred_property_source = PropertySource::Reaction;
    CHECK(!(a == b));

    // a record held by a shared pointer is shared and cannot be changed through it
    const SubstanceRecordPtr shared = std::make_shared<const SubstanceRecord>(a);
    const SubstanceRecordPtr same = shared;
    CHECK(same == shared && *same == a);

    ReactionRecord r1, r2;
    r1.stoichiometry = {{"Na+", -1.0}, {"Cl-", -1.0}, {"NaCl@", 1.0}};
    r2 = r1;
    CHECK(r1 == r2);
    r2.stoichiometry[2].second = 2.0;
    CHECK(!(r1 == r2));

    TableData t1, t2;
    t1.temperatures = {273.15, 298.15};
    t2 = t1;
    CHECK(t1 == t2);
    t2.reference_pressure = 1e5;
    CHECK(!(t1 == t2));
}

}

void runRecords()
{
    testDefaults();
    testEquality();
}
