// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_DIAGNOSTICS_H
#define THERMOFUN_DIAGNOSTICS_H

#include <string>
#include <vector>

namespace ThermoFun {

/// How serious a problem is.
enum class Severity
{
    Info,
    Warning,
    Error
};

/// One problem found while reading or validating data.
struct Diagnostic
{
    Severity severity = Severity::Error;

    /// Stable id, for example "V2-REF-001". The list is in diagnosticCatalog().
    std::string code;

    /// File name, if known.
    std::string file;

    /// Symbol of the record, if known, for example "NaCl@".
    std::string record;

    /// JSON pointer inside the record, for example "/methods/0/parameters/a1".
    std::string path;

    /// One sentence for the user.
    std::string message;
};

/// A list of problems. Reading and checking functions collect all problems here and do not stop at the first one.
class Diagnostics
{
public:
    /// Add a problem to the end of the list.
    auto add(Diagnostic d) -> void;

    /// True if the list has no problem with the severity Error.
    auto ok() const -> bool;

    /// All problems, in the order they were added.
    auto all() const -> const std::vector<Diagnostic>&;

    /// The problems with the severity Error.
    auto errors() const -> std::vector<Diagnostic>;

    /// One line for each problem: "error V2-REF-001 file:record:path: message". Empty parts are left out.
    auto toString() const -> std::string;

private:
    std::vector<Diagnostic> items_;
};

/// The text of one problem, as one line of Diagnostics::toString().
auto toString(const Diagnostic& d) -> std::string;

/// The text of a severity: "info", "warning" or "error".
auto toString(Severity severity) -> std::string;

/// One entry of the list of diagnostic codes.
struct DiagnosticInfo
{
    std::string code;
    Severity severity;
    std::string meaning;
};

/// All diagnostic codes with their severity and meaning.
auto diagnosticCatalog() -> const std::vector<DiagnosticInfo>&;

}

#endif
