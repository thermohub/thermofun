// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_BATCH_TABLES_H
#define THERMOFUN_BATCH_TABLES_H

#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Common/ThermoProperty.hpp>

namespace ThermoFun {

class Database;

/// A setup that cannot run. The message says why.
struct TableSetupError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/// One value of a table: the number in the unit of its column, and the result that it comes from, in the units of the engine.
struct TableCell
{
    double value = 0.0;
    std::string text;                 ///< for a column of the status
    bool defined = false;
    bool has_result = false;
    Reaktoro_::ThermoProperty result;
};

struct TableColumn
{
    std::string property, label, show, unit, format;
    std::optional<int> digits;
};

struct TableRow
{
    std::string record;
    double T = 0.0, P = 0.0;          ///< in the units of the conditions
    bool saturation = false;          ///< P is the saturation pressure
    std::vector<TableCell> cells;
};

struct TableProblem
{
    std::string table, record, message;
    double T = 0.0, P = 0.0;
    bool saturation = false, has_point = false;
};

/// A table of properties: the result of a TableSetup.
class Table
{
public:
    auto name() const -> const std::string& { return name_; }
    auto columns() const -> const std::vector<TableColumn>& { return columns_; }
    auto rows() const -> const std::vector<TableRow>& { return rows_; }
    auto problems() const -> const std::vector<TableProblem>& { return problems_; }
    auto output() const -> const std::string& { return output_; }

    /// The rows of numbers, in the units of the columns: NaN for a value that is not defined.
    auto values() const -> std::vector<std::vector<double>>;

    /// One cell. column: its label or its property. At the saturation pressure, give at_saturation.
    auto cell(const std::string& record, double T, double P, const std::string& column, bool at_saturation = false) const -> const TableCell&;

    /// The header and the rows as text, in the layout long or transposed.
    auto lines() const -> std::vector<std::vector<std::string>>;

    /// One grid for each column: T down and P across, or with `across: T` the other way. Each is (label of the column, lines).
    auto propertyGrids() const -> std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>;

    /// The table in a format: csv, md or tex.
    auto toText(const std::string& format = "csv") const -> std::string;

    /// Writes to a path, or to the output of the setup. The extension says the format. Gives the path.
    auto write(const std::string& path = "") const -> std::string;

private:
    friend class TableSetup;
    auto number(const TableColumn& column, const TableCell* cell) const -> std::string;
    auto format(const TableColumn& column, double value) const -> std::string;
    auto condition(double value, bool saturation, const std::optional<int>& digits) const -> std::string;

    std::string name_, output_, layout_ = "long", separator_ = ",", not_defined_ = "NaN", number_format_, across_ = "P", loop_ = "records", file_format_;
    std::string unit_T_, unit_P_;
    bool write_not_defined_ = true;
    std::optional<int> digits_, digits_T_, digits_P_;
    std::vector<TableColumn> columns_;
    std::vector<TableRow> rows_;
    std::vector<TableProblem> problems_;
    std::vector<double> Ts_, Ps_;
    std::vector<char> Ps_saturation_;
};

/// The tables of a setup, in the order of the setup.
class TableResults
{
public:
    auto tables() const -> const std::vector<Table>& { return tables_; }
    auto at(const std::string& name) const -> const Table&;

    /// Writes each table that has an output. Gives the paths.
    auto write() const -> std::vector<std::string>;
    auto problems() const -> std::vector<TableProblem>;

private:
    friend class TableSetup;
    std::vector<Table> tables_;
};

/// How to make tables: a YAML file, a text, or a template, changed with these functions. See docs/how-to/make-tables.md.
/// A `table` argument names one table; empty means all tables.
class TableSetup
{
public:
    TableSetup();
    TableSetup(const TableSetup& other);
    auto operator=(TableSetup other) -> TableSetup&;
    ~TableSetup();

    /// From YAML or JSON text.
    static auto fromText(const std::string& text) -> TableSetup;
    /// From a file. A database in the file is found next to it.
    static auto fromFile(const std::string& path) -> TableSetup;
    /// From a template of the library.
    static auto fromTemplate(const std::string& name) -> TableSetup;
    /// The templates of the library: name and what it makes.
    static auto templates() -> std::map<std::string, std::string>;
    /// The YAML text of a template.
    static auto templateText(const std::string& name) -> std::string;

    auto setDatabase(const Database& database) -> TableSetup&;
    auto setDatabase(const std::string& path) -> TableSetup&;

    auto setRecords(const std::vector<std::string>& symbols, const std::string& table = "") -> TableSetup&;
    auto setReactions(const std::vector<std::string>& symbols, const std::string& table = "") -> TableSetup&;
    auto setTemperatures(const std::vector<double>& values, const std::string& unit = "K", const std::string& table = "") -> TableSetup&;
    auto setTemperatures(double first, double last, double step, const std::string& unit = "K", const std::string& table = "") -> TableSetup&;
    auto setPressures(const std::vector<double>& values, const std::string& unit = "Pa", const std::string& table = "") -> TableSetup&;
    auto setPressures(double first, double last, double step, const std::string& unit = "Pa", const std::string& table = "") -> TableSetup&;
    auto setSaturationPressure(const std::string& table = "") -> TableSetup&;
    auto setLayout(const std::string& layout, const std::string& table = "") -> TableSetup&;
    auto setOutput(const std::string& path, const std::string& table = "") -> TableSetup&;

    /// Changes a column in all tables that have it. An empty unit or a negative digits leaves it as it is.
    auto setColumn(const std::string& property, const std::string& unit, int digits = -1, const std::string& table = "") -> TableSetup&;
    auto addColumn(const std::string& property, const std::string& unit = "", int digits = -1, const std::string& table = "") -> TableSetup&;
    auto removeColumn(const std::string& property, const std::string& table = "") -> TableSetup&;

    /// Any setting of a table by its key, with its value as JSON text: for example set("options", "{\"round_to_uncertainty\": true}").
    auto set(const std::string& key, const std::string& json, const std::string& table = "") -> TableSetup&;
    /// Any setting of a column, as JSON text: for example changeColumn("G", "{\"show\": \"err\"}").
    auto changeColumn(const std::string& property, const std::string& json, const std::string& table = "") -> TableSetup&;
    auto addColumnFromJson(const std::string& json, const std::string& table = "") -> TableSetup&;

    /// The setup as YAML text, to keep it or to change it.
    auto toYaml() const -> std::string;
    auto save(const std::string& path) const -> void;

    /// All tables, with the database of the setup.
    auto run() const -> TableResults;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif
