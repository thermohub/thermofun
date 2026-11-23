// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_BATCH_BATCHSETUP_H
#define THERMOFUN_BATCH_BATCHSETUP_H

#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Batch/OutputBatch.h>
#include <ThermoFun/Batch/RecordRules.h>
#include <ThermoFun/Batch/ThermoBatch.h>
#include <ThermoFun/Common/ThermoProperty.hpp>
#include <ThermoFun/Engine/Summary.h>

namespace ThermoFun {

class Database;

/// How the results of a batch calculation were made (decision 0032): the database, the templates, the summary of each engine and
/// the settings of the result.
struct Provenance
{
    std::string database;                 ///< the path of the setup, or "given in memory"
    std::vector<std::string> templates;   ///< the templates of the setup, each with where it was found
    std::vector<std::pair<std::string, EngineSummary>> calculations;   ///< one for each engine: its label in `compare`, and its summary
    std::string setup;                    ///< the settings of this result, as YAML

    /// Short lines of text, one after the other: what goes at the top of an output.
    auto text() const -> std::string;
    /// The same, as a list of lines.
    auto lines() const -> std::vector<std::string>;
    /// All of it as YAML, with the setup: what `provenance: file` writes.
    auto toYaml() const -> std::string;
};

struct ResultCell
{
    double value = 0.0;
    std::string text;                 ///< for a column of the status
    bool defined = false;
    bool has_result = false;
    Reaktoro_::ThermoProperty result;
};

struct ResultColumn
{
    std::string property, label, show, unit, format;
    std::optional<int> digits;
};

struct ResultRow
{
    std::string record;
    double T = 0.0, P = 0.0;          ///< in the units of the conditions
    bool saturation = false;          ///< P is the saturation pressure
    std::optional<double> P_used;     ///< for a saturation point: the saturation pressure that the calculation used, in the units of the conditions
    std::vector<ResultCell> cells;
};

struct BatchProblem
{
    std::string table, record, message;   ///< table: the name of the result
    double T = 0.0, P = 0.0;
    bool saturation = false, has_point = false;
};

/// One result table of a batch calculation: a row for each record and point, a column for each property (and each variant of compare).
class ResultTable
{
public:
    auto name() const -> const std::string& { return name_; }
    auto columns() const -> const std::vector<ResultColumn>& { return columns_; }
    auto rows() const -> const std::vector<ResultRow>& { return rows_; }
    auto problems() const -> const std::vector<BatchProblem>& { return problems_; }
    auto output() const -> const std::string& { return output_; }
    auto provenance() const -> const Provenance& { return provenance_; }

    /// The rows of numbers, in the units of the columns: NaN for a value that is not defined.
    auto values() const -> std::vector<std::vector<double>>;

    /// One cell. column: its label or its property. At the saturation pressure, give at_saturation.
    auto cell(const std::string& record, double T, double P, const std::string& column, bool at_saturation = false) const -> const ResultCell&;

    /// The pressure that the calculation of a row used, in the unit of the conditions: P itself, or the saturation pressure at T for a
    /// row at saturation (NaN if no value of that row could be calculated).
    auto pressureUsed(const std::string& record, double T, double P, bool at_saturation = false) const -> double;

    /// The header and the rows as text, in the layout long or wide.
    auto lines() const -> std::vector<std::vector<std::string>>;

    /// One grid for each column: T down and P across, or with `grid_across: T` the other way. Each is (label of the column, lines).
    auto propertyGrids() const -> std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>;

    /// The table in a format: csv, md or tex. With with_provenance, the lines of provenance() come first, as comments.
    auto toText(const std::string& format = "csv", bool with_provenance = false) const -> std::string;

    /// Writes to a path, or to output.file of the setup. The extension says the format, unless output.file_format does. Gives the path.
    /// The setting `provenance` says what else it writes: header (the default) puts the lines of provenance() at the top as comments,
    /// file writes all of it to <path>.provenance.yaml, none writes nothing.
    auto write(const std::string& path = "") const -> std::string;

private:
    friend class BatchSetup;
    auto number(const ResultColumn& column, const ResultCell* cell) const -> std::string;
    auto format(const ResultColumn& column, double value) const -> std::string;
    auto condition(double value, bool saturation, const std::optional<int>& digits, const std::optional<double>& used = std::nullopt) const -> std::string;
    /// A condition in the unit of output.units (T, P), from the unit of the conditions
    auto shown(double value, bool isT) const -> double;
    auto shownUsed(const ResultRow& row) const -> std::optional<double>;

    std::string name_, output_, layout_ = "long", separator_ = ",", not_defined_ = "NaN", number_format_, across_ = "P", loop_ = "records", file_format_, provenance_mode_ = "header";
    Provenance provenance_;
    std::string unit_T_, unit_P_, shown_T_, shown_P_;
    bool write_not_defined_ = true;
    std::optional<int> digits_, digits_T_, digits_P_;
    std::vector<ResultColumn> columns_;
    std::vector<ResultRow> rows_;
    std::vector<BatchProblem> problems_;
    std::vector<double> Ts_, Ps_;
    std::vector<char> Ps_saturation_;
};

/// The result tables of a batch calculation, in the order of the setup.
class BatchResults
{
public:
    auto tables() const -> const std::vector<ResultTable>& { return tables_; }
    auto at(const std::string& name) const -> const ResultTable&;

    /// Writes each result table that has output.file. Gives the paths.
    auto write() const -> std::vector<std::string>;
    auto problems() const -> std::vector<BatchProblem>;

private:
    friend class BatchSetup;
    std::vector<ResultTable> tables_;
};

/// One result of a setup as a calculation of ThermoBatch, the 0.x batch API.
/// A row of the catalog of the batch templates.
struct BatchTemplateInfo
{
    std::string name;
    std::string summary;              ///< what it makes: the first line of the file
    std::vector<std::string> blocks;  ///< the blocks it gives, in the order of a setup: database, engine, conditions, results, output;
                                      ///< a fragment has one of conditions or output, and no results
    std::string source;               ///< the path of its file, or "library"
};

class BatchCalculation
{
public:
    auto batch() -> ThermoBatch& { return batch_; }
    auto symbols() const -> const std::vector<std::string>& { return symbols_; }
    auto properties() const -> const std::vector<std::string>& { return properties_; }
    auto points() const -> const std::vector<std::vector<double>>& { return points_; }
    auto forReactions() const -> bool { return reactions_; }
    auto provenance() const -> const Provenance& { return provenance_; }

    /// Calculates, and gives the Output of ThermoBatch.
    auto run() -> Output;

    /// Calculates, and writes to a path or to the output of the setup, in the layout of the setup. Gives the path.
    auto write(const std::string& path = "") -> std::string;

private:
    friend class BatchSetup;
    explicit BatchCalculation(ThermoBatch batch) : batch_(std::move(batch)) {}
    ThermoBatch batch_;
    std::vector<std::string> symbols_, properties_;
    std::vector<std::vector<double>> points_;
    bool reactions_ = false;
    std::string layout_ = "long", provenance_mode_ = "header";
    Provenance provenance_;
};

/// A batch calculation (decision 0032): many records at many conditions at once. The setup has four blocks: `database` and
/// `engine` (with what), `conditions` (where: T and P), `results` (what: one entry for each result table) and `output` (how to give
/// the results). A block at the top is the default of every result; a result can give its own engine, conditions or output, which
/// replace the defaults key by key. See docs/reference/batch.md.
class BatchSetup
{
public:
    BatchSetup();
    BatchSetup(const BatchSetup& other);
    auto operator=(BatchSetup other) -> BatchSetup&;
    ~BatchSetup();

    /// From YAML or JSON text.
    static auto fromText(const std::string& text) -> BatchSetup;
    /// From a file. A database in the file is found next to it.
    static auto fromFile(const std::string& path) -> BatchSetup;
    /// From a batch template.
    static auto fromTemplate(const std::string& name) -> BatchSetup;
    /// The catalog of the batch templates, by name. Those of the folders of THERMOFUN_TEMPLATE_PATH come with those of the
    /// library, and replace one of the library that has the same name.
    static auto catalog() -> std::vector<BatchTemplateInfo>;
    /// One template of the catalog; an error names those there are
    static auto catalog(const std::string& name) -> BatchTemplateInfo;
    /// The YAML text of a template.
    static auto templateText(const std::string& name) -> std::string;
    /// The folders of THERMOFUN_TEMPLATE_PATH.
    static auto templateFolders() -> std::vector<std::string>;

    /// Puts a template under the settings: what it says replaces what the setup says, and a result with the name of one of
    /// the setup changes that result. A template can have no results: it is then a fragment, such as the conditions of a batch.
    auto useTemplate(const std::string& name) -> BatchSetup&;

    auto setDatabase(const Database& database) -> BatchSetup&;
    auto setDatabase(const std::string& path) -> BatchSetup&;

    /// The settings of all results (result empty) or of one result.
    auto setSubstances(const std::vector<std::string>& symbols, const std::string& result = "") -> BatchSetup&;
    auto setReactions(const std::vector<std::string>& symbols, const std::string& result = "") -> BatchSetup&;
    /// Substances or reactions by a rule, for example "{class: aqueous_solute, elements: [Ca]}": see Batch/RecordRules.h.
    auto setSubstanceRule(const std::string& rule, const std::string& result = "") -> BatchSetup&;
    auto setReactionRule(const std::string& rule, const std::string& result = "") -> BatchSetup&;
    /// The conditions at the top (result empty: for all results) or of one result.
    auto setTemperatures(const std::vector<double>& values, const std::string& unit = "K", const std::string& result = "") -> BatchSetup&;
    auto setTemperatures(double first, double last, double step, const std::string& unit = "K", const std::string& result = "") -> BatchSetup&;
    auto setPressures(const std::vector<double>& values, const std::string& unit = "Pa", const std::string& result = "") -> BatchSetup&;
    auto setPressures(double first, double last, double step, const std::string& unit = "Pa", const std::string& result = "") -> BatchSetup&;
    auto setSaturationPressure(const std::string& result = "") -> BatchSetup&;
    /// output.layout and output.file at the top, or of one result.
    auto setLayout(const std::string& layout, const std::string& result = "") -> BatchSetup&;
    auto setOutputFile(const std::string& path, const std::string& result = "") -> BatchSetup&;

    /// Changes a property in all results that have it. An empty unit or a negative digits leaves it as it is.
    auto setProperty(const std::string& property, const std::string& unit, int digits = -1, const std::string& result = "") -> BatchSetup&;
    auto addProperty(const std::string& property, const std::string& unit = "", int digits = -1, const std::string& result = "") -> BatchSetup&;
    auto removeProperty(const std::string& property, const std::string& result = "") -> BatchSetup&;
    /// Any setting of a property, as JSON text: for example changeProperty("G", "{\"show\": \"uncertainty\"}").
    auto changeProperty(const std::string& property, const std::string& json, const std::string& result = "") -> BatchSetup&;
    auto addPropertyFromJson(const std::string& json, const std::string& result = "") -> BatchSetup&;

    /// Any value of the setup by its path (a JSON pointer), as JSON text: for example set("/results/0/name", "\"species\"") or
    /// set("/engine/options/round_to_uncertainty", "true"). The keys on the way are made.
    auto set(const std::string& path, const std::string& json) -> BatchSetup&;
    /// The value at a path, as JSON text.
    auto get(const std::string& path) const -> std::string;

    /// The setup as YAML text, to keep it or to change it.
    auto toYaml() const -> std::string;
    auto save(const std::string& path) const -> void;

    /// All results, with the database of the setup. A placeholder <...> left in the setup is an error that names its paths.
    auto run() const -> BatchResults;

    /// One result as a calculation of ThermoBatch. Empty result: the setup must have one result. A ThermoBatch calculates the values
    /// of substances or reactions: a result with `compare`, with `show` other than value, or with properties of water is refused.
    auto batch(const std::string& result = "") const -> BatchCalculation;

    /// Sets the units, the digits and the preferences of a ThermoBatch that you have, from one result.
    auto applyTo(ThermoBatch& batch, const std::string& result = "") const -> void;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif
