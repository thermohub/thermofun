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
    // the location first, as compilers write it
    auto d = make(Severity::Error, "V2-MTH-001", "aq17.yaml", "Ca+2", "/methods/0/method", "Unknown method HKF8.");
    d.line = 1303;
    d.column = 5;
    d.fix = "Use HKF88.";
    CHECK(toString(d) == "aq17.yaml:1303:5: error V2-MTH-001 Ca+2 /methods/0/method: Unknown method HKF8. Use HKF88.");
    d.column = 0;
    CHECK(toString(d).rfind("aq17.yaml:1303: error", 0) == 0);
    d.line = 0;
    CHECK(toString(d).rfind("aq17.yaml: error", 0) == 0);

    // without a fix of its own, the fix of the code in the catalog is used
    auto e = make(Severity::Error, "V2-REF-001", "f.yaml", "NaCl@", "/a/b", "not found");
    CHECK(toString(e) == "f.yaml: error V2-REF-001 NaCl@ /a/b: not found " + diagnosticInfo("V2-REF-001")->fix);

    // empty parts are left out; an unknown code has no fix
    CHECK(toString(make(Severity::Info, "X-1", "", "", "", "id wins")) == "info X-1: id wins");
    CHECK(toString(make(Severity::Error, "", "f.json", "", "", "bad")) == "f.json: error: bad");
    CHECK(toString(make(Severity::Error, "X-2", "", "", "", "")) == "error X-2");

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
        CHECK(!info.fix.empty());  // a fix for every code
        CHECK(info.meaning.back() == '.' && info.fix.back() == '.');  // full sentences
        CHECK(codes.insert(info.code).second);  // each code once
        CHECK(diagnosticInfo(info.code) == &info);
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
    CHECK(diagnosticInfo("NO-SUCH-CODE") == nullptr);
}

}

void runDiagnostics()
{
    testCollect();
    testText();
    testCatalog();
}
