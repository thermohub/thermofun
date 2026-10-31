// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reading and writing quantities of the v2 format: every row of the table of the format, every error, the default units
// of a scope, and the write of the shortest form.

#include <cmath>
#include <string>

#include <nlohmann/json.hpp>

#include "IO/V2/detail/Normalize.h"
#include "check.h"

namespace {

using nlohmann::json;
using namespace ThermoFun;
using namespace ThermoFun::v2;

auto close(double a, double b) -> bool
{
    return std::fabs(a - b) <= 1e-12 * std::fabs(b) + 1e-300;
}

// reads a quantity; the code of the first diagnostic is returned in `code`
auto read(const std::string& text, const std::string& slot, std::string& code, const UnitScope& scope = {}) -> std::optional<Quantity>
{
    Diagnostics diag;
    auto q = readQuantity(json::parse(text), slot, scope, diag, "/x");
    code = diag.all().empty() ? "" : diag.all().front().code;
    return q;
}

}

void runNormalize()
{
    std::string code;

    // the table of the format
    auto q = read("-237183", "J/mol", code);
    CHECK(q && q->value == -237183 && q->unit_as_written.empty() && !q->uncertainty && q->isBare() && code.empty());
    q = read(R"({"value": -237183})", "J/mol", code);
    CHECK(q && q->value == -237183 && q->isBare());
    q = read(R"({"value": -237.183, "unit": "kJ/mol"})", "J/mol", code);
    CHECK(q && close(q->value, -237183) && q->unit_as_written == "kJ/mol" && q->value_as_written == "-237.183");
    q = read(R"({"value": -237183, "uncertainty": 100})", "J/mol", code);
    CHECK(q && q->uncertainty && q->uncertainty->standard == 100 && q->uncertainty->coverage_factor_as_written == 1.0);
    q = read(R"({"value": -237183, "uncertainty": {"standard": 100}})", "J/mol", code);
    CHECK(q && q->uncertainty && q->uncertainty->standard == 100);
    q = read(R"({"value": -237183, "uncertainty": {"expanded": 200, "coverage_factor": 2}})", "J/mol", code);
    CHECK(q && q->uncertainty && q->uncertainty->standard == 100 && q->uncertainty->coverage_factor_as_written == 2.0);
    q = read(R"({"value": 1, "uncertainty": {"standard": 0.1, "distribution": "normal", "degrees_of_freedom": 12, "kind": "primary"}})",
             "J/mol", code);
    CHECK(q && q->uncertainty && q->uncertainty->distribution == "normal" && *q->uncertainty->degrees_of_freedom == 12 &&
          q->uncertainty->kind == Uncertainty::Kind::Primary);
    q = read(R"({"value": 1, "quality": "core", "origin": "calculated", "calculation_path": "a+b", "source": "2018LOT/MAT"})", "J/mol", code);
    CHECK(q && q->meta.quality == Quality::Core && q->meta.origin == Origin::Calculated && q->meta.calculation_path == "a+b" &&
          q->meta.source == "2018LOT/MAT");
    q = read(R"("-237.183 kJ/mol")", "J/mol", code);
    CHECK(q && close(q->value, -237183) && q->unit_as_written == "kJ/mol");
    // a defined zero uncertainty is kept
    q = read(R"({"value": 5, "uncertainty": 0})", "J/mol", code);
    CHECK(q && q->uncertainty && q->uncertainty->standard == 0.0);

    // an uncertainty is a difference: no offset of the temperature scale
    q = read(R"({"value": 25, "unit": "Cel", "uncertainty": 0.5})", "K", code);
    CHECK(q && close(q->value, 298.15) && close(q->uncertainty->standard, 0.5));
    // the uncertainty is in the unit of the value
    q = read(R"({"value": 1, "unit": "kJ/mol", "uncertainty": 0.1})", "J/mol", code);
    CHECK(q && close(q->uncertainty->standard, 100));

    // the errors
    CHECK(!read(R"("abc kJ/mol")", "J/mol", code) && code == "V2-QTY-001");
    CHECK(!read(R"("12")", "J/mol", code) && code == "V2-QTY-001");
    CHECK(!read(R"({"unit": "kJ/mol"})", "J/mol", code) && code == "V2-QTY-002");
    CHECK(!read("true", "J/mol", code) && code == "V2-QTY-002");
    q = read(R"({"value": 1, "colour": "red"})", "J/mol", code);
    CHECK(q && code == "V2-QTY-003");
    CHECK(!read(R"({"value": 1, "unit": "furlong"})", "J/mol", code) && code == "V2-UNIT-001");
    CHECK(!read(R"({"value": 1, "unit": "kg"})", "J/mol", code) && code == "V2-UNIT-002");
    CHECK(!read(R"({"value": 1, "uncertainty": {"expanded": 2}})", "J/mol", code) && code == "V2-UNC-001");
    CHECK(!read(R"({"value": 1, "uncertainty": {"standard": 1, "expanded": 2, "coverage_factor": 2}})", "J/mol", code) &&
          code == "V2-UNC-002");
    CHECK(!read(R"({"value": 1, "uncertainty": -1})", "J/mol", code) && code == "V2-UNC-003");
    CHECK(!read(R"({"value": 1, "uncertainty": {"expanded": 2, "coverage_factor": 0}})", "J/mol", code) && code == "V2-UNC-004");
    CHECK(!read(R"({"value": 1, "uncertainty": {"expanded": 2, "coverage_factor": -2}})", "J/mol", code) && code == "V2-UNC-004");
    CHECK(!read(R"({"value": 1, "uncertainty": {"expanded": 2, "coverage_factor": "x"}})", "J/mol", code) && code == "V2-UNC-004");

    // the default units of a scope
    Diagnostics diag;
    const UnitScope outer = readUnitScope(json::parse(R"({"energy": "cal", "pressure": "bar"})"), nullptr, diag, "/units");
    CHECK(diag.all().empty());
    CHECK(scopedUnit("J/(mol.K)", outer) == "cal/(mol.K)");
    CHECK(scopedUnit("Pa", outer) == "bar");
    CHECK(scopedUnit("J/(mol.Pa)", outer) == "cal/(mol.bar)");
    CHECK(scopedUnit("K", outer) == "K");
    CHECK(scopedUnit("m3/mol", outer) == "m3/mol");
    UnitScope inner;
    inner.parent = &outer;
    inner.defaults["energy"] = "kJ";
    inner.defaults["length"] = "cm";
    CHECK(scopedUnit("J/mol", inner) == "kJ/mol");
    CHECK(scopedUnit("m3/mol", inner) == "cm3/mol");
    CHECK(scopedUnit("Pa", inner) == "bar");
    q = read("1", "J/mol", code, outer);
    CHECK(q && close(q->value, 4.184) && q->unit_as_written == "cal/mol" && q->value_as_written == "1.0");
    q = read("1", "J/mol", code, inner);
    CHECK(q && close(q->value, 1000));
    q = read(R"({"value": 1, "unit": "J/mol"})", "J/mol", code, inner);
    CHECK(q && q->value == 1 && q->unit_as_written.empty());
    // a unit of the scope with the wrong dimension is refused
    Diagnostics bad;
    readUnitScope(json::parse(R"({"energy": "kg"})"), nullptr, bad, "/units");
    CHECK(!bad.all().empty() && bad.all().front().code == "V2-UNIT-002");

    // the writer: the shortest form, and the read of what was written gives the same quantity
    for (const char* text : {"-237183", R"({"value":-237.183,"unit":"kJ/mol"})", R"({"value":-237183,"uncertainty":100})",
                             R"({"value":-237183,"uncertainty":{"expanded":200,"coverage_factor":2}})",
                             R"({"value":1,"uncertainty":{"standard":0.1,"distribution":"normal","degrees_of_freedom":12}})",
                             R"({"value":1,"quality":"core","origin":"calculated","source":"2018LOT/MAT"})",
                             R"({"value":-237.183,"unit":"kJ/mol","uncertainty":0.1})"})
    {
        const json node = json::parse(text);
        auto first = readQuantity(node, "J/mol", {}, diag, "/x");
        CHECK(first.has_value());
        if (!first)
            continue;
        const json written = writeQuantity(*first, "J/mol");
        CHECK(written == node);
        auto second = readQuantity(written, "J/mol", {}, diag, "/x");
        CHECK(second && *second == *first);
    }
    // the shortest forms
    Quantity bare;
    bare.value = 5;
    CHECK(writeQuantity(bare, "J/mol") == json(5));
    Quantity withUncertainty = bare;
    withUncertainty.uncertainty = Uncertainty{};
    withUncertainty.uncertainty->standard = 0.0;
    CHECK(writeQuantity(withUncertainty, "J/mol") == json::parse(R"({"value":5,"uncertainty":0.0})"));
}
