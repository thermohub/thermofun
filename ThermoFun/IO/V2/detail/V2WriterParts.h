// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// What the parts of the v2 writer share. Only for the .cpp files of this reader or writer.

#ifndef THERMOFUN_IO_V2_DETAIL_V2WRITERPARTS_H
#define THERMOFUN_IO_V2_DETAIL_V2WRITERPARTS_H

#include <ThermoFun/IO/V2/V2Writer.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <nlohmann/json.hpp>
#include <ThermoFun/Common/Numbers.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/Equation.h>
#include <ThermoFun/IO/V2/detail/Normalize.h>
#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::model::v2writer {

using json = nlohmann::ordered_json;

constexpr const char* formatVersion = "2.0.0-draft.1";

struct Written
{
    std::string unit;        ///< the unit of the numbers; empty: the unit of the slot
    bool from_scope = false; ///< the reader gets the unit from a scope or a default, so it is not written with the value
};

// J/bar on a molar volume is for one mole, as in the reader
auto molar(const std::string& unit, const std::string& slot) -> std::string;

// True if the two units are the same unit, in any spelling
auto sameUnit(const std::string& a, const std::string& b) -> bool;

// True if a value of the slot can be written in the unit
auto usable(const std::string& unit, const std::string& slot) -> bool;

// The shortest decimal that reads back as the same float
auto shortestFloat(double x) -> double;

// A limit of T or P has no flag of its source: a value that is a float printed with 13 or 14 digits is one, for example 700.00000610352
auto limit(double x, const V2WriterOptions& options) -> double;

// The number to write in `unit` for the SI value, so that the reader gets the same double, if a number near it does that
auto numberIn(double si, const std::string& unit, const std::string& slot) -> std::optional<double>;

// The same for an uncertainty; if no number reads back the same, the nearest one
auto differenceIn(double si, const std::string& from, const std::string& slot) -> double;

// A quantity in a unit: a bare number if it has only a value and the reader knows the unit
auto quantityJson(const Quantity& q, const std::string& slot, const Written& w, const V2WriterOptions& options) -> json;

// The unit of a value of a record: as written, without a unit that is the slot unit in another spelling or that cannot hold the value
auto writtenUnit(const Quantity& q, const std::string& slot, const std::string& source_unit) -> std::string;

auto simpleQuantity(const Quantity& q, const std::string& slot, const std::string& source_unit, const V2WriterOptions& options) -> json;

// a legacy record can have an empty citation
auto sourcesOf(const std::vector<std::string>& sources) -> std::vector<std::string>;

auto applicabilityJson(const Applicability& a, const V2WriterOptions& options) -> json;

auto conventionJson(const Convention& c) -> json;

// the unknown fields of a record, after the canonical ones; a canonical key wins
void addExtra(json& out, const std::string& extra);

auto referenceJson(const ReferenceConditions& rc) -> json;

auto covariancesJson(const std::vector<CovarianceBlock>& blocks) -> json;

// The `units` mapping of a method: the one that lets most values of the application be bare numbers in their unit
auto chooseScope(const std::map<std::string, std::string>& wanted, const MethodSpec* spec) -> std::map<std::string, std::string>;

auto parametersJson(const std::map<std::string, Quantity>& parameters, const MethodSpec* spec, const std::map<std::string, std::string>& scopeDefaults,
                    const std::map<std::string, std::string>& wanted, const V2WriterOptions& options) -> json;

auto methodsJson(const std::vector<MethodApplication>& methods, OperatesOn operatesOn, DataFormat source, const V2WriterOptions& options) -> json;

auto structuralJson(const std::map<std::string, Quantity>& structural, const V2WriterOptions& options) -> json;

// the molar mass: a bare number is in g/mol
auto molarMassJson(const Quantity& q, const V2WriterOptions& options) -> json;

auto elementJson(const ElementRecord& r, const V2WriterOptions& options) -> json;

auto substanceJson(const SubstanceRecord& r, const ThermoDataSetInfo& dataset, const V2WriterOptions& options) -> json;

auto reactionJson(const ReactionRecord& r, const ThermoDataSetInfo& dataset, const V2WriterOptions& options) -> json;

auto methodRecordJson(const MethodDefinition& m) -> json;

auto parameterSetJson(const MethodParameterSet& p, const std::vector<MethodDefinition>& methods, const V2WriterOptions& options) -> json;

auto dataSourceJson(const DataSource& d) -> json;

auto datasetJson(const ThermoDataSetInfo& d) -> json;

auto documentJson(const V2Records& r, const V2WriterOptions& options) -> json;

// A word that YAML reads as the same text without quotes
auto plainWord(const std::string& s) -> bool;

auto text(const std::string& s) -> std::string;

auto isBlock(const json& node) -> bool;

// The node on one line
auto flow(const json& node) -> std::string;

auto onlyNumbers(const json& node) -> bool;

// a mapping or list goes on one line if it is short, and a list of numbers always does
auto staysOnOneLine(const json& node, size_t room) -> bool;

void block(const json& node, size_t indent, std::string& out);

// The shortest number near `start` that the reader turns into `si`: the output depends on the value, not on how the file wrote it
template <class ReadBack>
auto shortestThatReadsBack(double si, double start, ReadBack read) -> std::optional<double>
{
    std::optional<double> best;
    size_t bestLength = 0;
    auto consider = [&](double x) {
        if (read(x) != si)
            return;
        const size_t length = nlohmann::json(x).dump().size();
        if (!best || length < bestLength)
        {
            best = x;
            bestLength = length;
        }
    };
    double up = start, down = start;
    consider(start);
    for (int i = 0; i < 8; ++i)
    {
        consider(up = std::nextafter(up, HUGE_VAL));
        consider(down = std::nextafter(down, -HUGE_VAL));
    }
    return best;
}

} // namespace ThermoFun::model::v2writer

#endif
