// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Diagnostics.h"

namespace ThermoFun {

auto Diagnostics::add(Diagnostic d) -> void
{
    items_.push_back(std::move(d));
}

auto Diagnostics::ok() const -> bool
{
    for (const auto& d : items_)
        if (d.severity == Severity::Error)
            return false;
    return true;
}

auto Diagnostics::all() const -> const std::vector<Diagnostic>&
{
    return items_;
}

auto Diagnostics::errors() const -> std::vector<Diagnostic>
{
    std::vector<Diagnostic> out;
    for (const auto& d : items_)
        if (d.severity == Severity::Error)
            out.push_back(d);
    return out;
}

auto Diagnostics::toString() const -> std::string
{
    std::string out;
    for (const auto& d : items_)
        out += ThermoFun::toString(d) + "\n";
    return out;
}

auto toString(Severity severity) -> std::string
{
    switch (severity)
    {
    case Severity::Info: return "info";
    case Severity::Warning: return "warning";
    case Severity::Error: return "error";
    }
    return "error";
}

auto toString(const Diagnostic& d) -> std::string
{
    std::string line = toString(d.severity);
    if (!d.code.empty())
        line += " " + d.code;

    std::string where;
    for (const std::string* part : {&d.file, &d.record, &d.path})
    {
        if (part->empty())
            continue;
        if (!where.empty())
            where += ":";
        where += *part;
    }
    if (!where.empty())
        line += " " + where;

    if (!d.message.empty())
        line += ": " + d.message;
    return line;
}

auto diagnosticCatalog() -> const std::vector<DiagnosticInfo>&
{
    static const std::vector<DiagnosticInfo> catalog = {
        {"V2-VER-001", Severity::Error, "unsupported major format version"},
        {"V2-VER-002", Severity::Warning, "newer minor version than known"},
        {"V2-KEY-001", Severity::Warning, "unknown key (kept)"},
        {"V2-ID-001", Severity::Error, "missing or empty symbol"},
        {"V2-ID-002", Severity::Error, "symbol inside record differs from map key"},
        {"V2-ID-003", Severity::Warning, "duplicate symbol, later record replaces earlier (today's behaviour)"},
        {"V2-QTY-001", Severity::Error, "string quantity is not \"number unit\""},
        {"V2-QTY-002", Severity::Error, "quantity object without value"},
        {"V2-QTY-003", Severity::Warning, "unknown key in quantity"},
        {"V2-UNIT-001", Severity::Error, "unit string cannot be parsed"},
        {"V2-UNIT-002", Severity::Error, "unit has the wrong dimension for the slot"},
        {"V2-UNC-001", Severity::Error, "`expanded` uncertainty without `coverage_factor`"},
        {"V2-UNC-002", Severity::Error, "both `standard` and `expanded` given"},
        {"V2-UNC-003", Severity::Error, "negative uncertainty"},
        {"V2-UNC-004", Severity::Error, "coverage factor not finite or not > 0"},
        {"V2-DS-001", Severity::Error, "published ThermoDataSet without `version`"},
        {"V2-KEY-002", Severity::Warning, "an extension field collides with a canonical field on writing (canonical wins)"},
        {"V2-VOC-001", Severity::Warning, "unknown vocabulary value (class, aggregate_state, ...), `other` used"},
        {"V2-MTH-001", Severity::Error, "unknown method id"},
        {"V2-MTH-002", Severity::Error, "unknown parameter name for the method"},
        {"V2-MTH-003", Severity::Error, "both `parameters` and `segments`"},
        {"V2-MTH-004", Severity::Error, "required parameter missing"},
        {"V2-MTH-005", Severity::Error, "method not applicable to the record's class (D23)"},
        {"V2-MTH-006", Severity::Warning, "segments overlap or leave gaps"},
        {"V2-MTH-007", Severity::Error, "two methods in the same slot (phase 2 only; replaced by 012-015 from step S2.10)"},
        {"V2-MTH-008", Severity::Error, "method known but not implemented (06 section 5 `impl = no`)"},
        {"V2-MTH-012", Severity::Error, "not exactly one base method (06 section 8.2)"},
        {"V2-MTH-013", Severity::Error, "two methods set the same property"},
        {"V2-MTH-014", Severity::Warning, "a method adds to V but no earlier step defines V"},
        {"V2-MTH-015", Severity::Warning, "a transformation is listed after setters/adders (it runs before them)"},
        {"V2-MTH-016", Severity::Info, "an inline parameter overrides an entry of the referenced parameter set"},
        {"V2-MTH-017", Severity::Error, "more than one transformation method in a record"},
        {"V2-MTH-018", Severity::Error, "a `table` has arrays whose size is not nT x nP, or unsorted grids"},
        {"V2-MTH-010", Severity::Error, "Method record's parameter definitions disagree with the built-in spec"},
        {"V2-MTH-011", Severity::Error, "Method record's `operates_on`/`contributes_to` disagree with the built-in spec"},
        {"V2-MTH-020", Severity::Error, "composite method used in a calculation"},
        {"V2-RXN-001", Severity::Error, "equation and stoichiometry disagree"},
        {"V2-RXN-002", Severity::Warning, "reaction not balanced in elements or charge"},
        {"V2-REF-001", Severity::Error, "`defining_reaction` not found"},
        {"V2-REF-002", Severity::Warning, "stoichiometry names a substance that is not in the database"},
        {"V2-REF-003", Severity::Error, "unknown method id or alias"},
        {"V2-REF-004", Severity::Error, "`parameter_set` not found or belongs to another method"},
        {"V2-REF-005", Severity::Warning, "`data_source` not found"},
        {"V2-REF-010", Severity::Error, "cycle of defining reactions"},
        {"V2-INC-001", Severity::Error, "include cycle"},
        {"CNV-001", Severity::Error, "a conversion needs element entropies that are missing (names the elements)"},
        {"CNV-002", Severity::Error, "molarity conversion without a solvent"},
        {"CNV-003", Severity::Warning, "a method's native convention differs from the dataset's and the record declares none (conversion applied)"},
        {"CNV-004", Severity::Warning, "a reaction mixes species of different energy references (conversion applied; informational)"},
        {"CNV-005", Severity::Error, "unknown convention value"},
        {"V2-SYM-001", Severity::Error, "unknown variable or function in a symbolic expression"},
        {"V2-SYM-002", Severity::Error, "symbolic expression does not parse"},
        {"V2-SYM-003", Severity::Warning, "`source` changed after `expression` was generated (regenerate)"},
        {"V2-SYM-004", Severity::Error, "a reference test case of the method fails"},
        {"V2-SYM-006", Severity::Error, "a symbolic expression exceeds the size limits or uses a disallowed construct"},
        {"V2-MTH-021", Severity::Warning, "a dataset-local Method record overrides a project or built-in method"},
        {"V2-MTH-022", Severity::Warning, "a method's symbolic expression and its coded implementation disagree on the test cases"},
        {"V2-SYM-005", Severity::Error, "the expression does not have the dimension of energy per mole"},
        {"V2-SRC-001", Severity::Warning, "DataSource symbol not in NEA pattern"},
        {"LEG-QRK-001", Severity::Info, "legacy quirk: method id and name disagree (id wins)"},
        {"LEG-QRK-002", Severity::Info, "legacy quirk: method name has leading or trailing spaces (trimmed)"},
        {"LEG-QRK-003", Severity::Info, "legacy quirk: eos_hkf_coeffs units[2] is \"(cal*K)/mol\" (units replaced by the expected list)"},
        {"LEG-QRK-004", Severity::Info, "legacy quirk: m_expansivity unit is \"kbar\" (read as 1/K, value x 1e5)"},
        {"LEG-QRK-005", Severity::Info, "legacy quirk: m_compressibility unit is \"1e-05/K\" (read as kbar)"},
        {"LEG-QRK-006", Severity::Info, "legacy quirk: aggregate_state missing or empty (other is used, not gas)"},
        {"LEG-QRK-007", Severity::Info, "legacy quirk: record wrapped in `properties` (unwrapped)"},
        {"LEG-QRK-008", Severity::Info, "legacy quirk: `values: [null]` (property missing)"},
        {"LEG-QRK-009", Severity::Info, "legacy quirk: more than 1000 coefficients (error, today's limit)"},
        {"LEG-QRK-010", Severity::Info, "legacy quirk: coefficient array longer than the method uses"},
        {"LEG-QRK-011", Severity::Info, "legacy quirk: `units` list shorter than `values`, or empty strings in `units` (the legacy unit of the method is used)"},
        {"LEG-MTH-001", Severity::Error, "legacy method id unknown"},
        {"LEG-MTH-002", Severity::Warning, "legacy method has no implementation or is ignored today (ids 1, 6, 30, 39, 41, 42; reaction 8)"},
        {"LEG-KEY-002", Severity::Warning, "coefficient key does not belong to the entry's method (kept)"},
        {"LEG-WRT-001", Severity::Warning, "part of a record cannot be written in the legacy format"},
        {"LEG-KEY-001", Severity::Warning, "legacy key not read by ThermoFun"},
    };
    return catalog;
}

}
