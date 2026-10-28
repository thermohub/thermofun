// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of Common/Diagnostics.h: collecting problems, the text of a problem and the list of codes.

#include <set>
#include <string>

#include "Common/Diagnostics.h"
#include "check.h"

using namespace ThermoFun;

namespace {

Diagnostic make(Severity severity, const char* code, const char* file, const char* record, const char* path,
                const char* message)
{
    Diagnostic d;
    d.severity = severity;
    d.code = code;
    d.file = file;
    d.record = record;
    d.path = path;
    d.message = message;
    return d;
}

void testCollect()
{
    Diagnostics list;
    CHECK(list.ok());
    CHECK(list.all().empty());
    CHECK(list.toString().empty());

    list.add(make(Severity::Warning, "V2-KEY-001", "a.yaml", "NaCl@", "/x", "unknown key"));
    CHECK(list.ok());  // a warning is not an error
    CHECK(list.errors().empty());

    list.add(make(Severity::Info, "LEG-QRK-001", "", "", "", "id wins"));
    CHECK(list.ok());

    list.add(make(Severity::Error, "V2-REF-001", "a.yaml", "CaCO3", "/defining_reaction", "not found"));
    CHECK(!list.ok());
    CHECK(list.all().size() == 3);
    CHECK(list.errors().size() == 1);
    CHECK(list.errors()[0].code == "V2-REF-001");
    CHECK(list.all()[0].code == "V2-KEY-001");  // the order is kept
}

void testText()
{
    CHECK(toString(make(Severity::Error, "V2-REF-001", "f.yaml", "NaCl@", "/a/b", "not found")) ==
          "error V2-REF-001 f.yaml:NaCl@:/a/b: not found");
    // empty parts are left out
    CHECK(toString(make(Severity::Warning, "V2-ID-003", "", "NaCl@", "", "duplicate")) ==
          "warning V2-ID-003 NaCl@: duplicate");
    CHECK(toString(make(Severity::Info, "LEG-QRK-001", "", "", "", "id wins")) == "info LEG-QRK-001: id wins");
    CHECK(toString(make(Severity::Error, "", "f.json", "", "", "bad")) == "error f.json: bad");
    CHECK(toString(make(Severity::Error, "V2-VER-001", "", "", "", "")) == "error V2-VER-001");

    Diagnostics list;
    list.add(make(Severity::Error, "A", "", "", "", "one"));
    list.add(make(Severity::Info, "B", "", "", "", "two"));
    CHECK(list.toString() == "error A: one\ninfo B: two\n");
}

void testCatalog()
{
    const auto& catalog = diagnosticCatalog();
    CHECK(catalog.size() > 70);

    std::set<std::string> codes;
    for (const auto& info : catalog)
    {
        CHECK(!info.code.empty());
        CHECK(!info.meaning.empty());
        CHECK(codes.insert(info.code).second);  // each code once
    }

    auto severityOf = [&](const std::string& code) {
        for (const auto& info : catalog)
            if (info.code == code)
                return info.severity;
        return Severity::Info;
    };
    CHECK(codes.count("V2-REF-001") == 1);
    CHECK(codes.count("CNV-005") == 1);
    CHECK(codes.count("LEG-QRK-011") == 1);
    CHECK(codes.count("LEG-KEY-001") == 1);
    CHECK(severityOf("V2-REF-001") == Severity::Error);
    CHECK(severityOf("V2-ID-003") == Severity::Warning);
    CHECK(severityOf("V2-MTH-016") == Severity::Info);
    CHECK(severityOf("CNV-003") == Severity::Warning);
}

}

void runDiagnostics()
{
    testCollect();
    testText();
    testCatalog();
}
