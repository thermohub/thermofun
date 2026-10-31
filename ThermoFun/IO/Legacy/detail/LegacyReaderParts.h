// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// What the parts of the legacy reader share. Only for the .cpp files of this reader or writer.

#ifndef THERMOFUN_IO_LEGACY_DETAIL_LEGACYREADERPARTS_H
#define THERMOFUN_IO_LEGACY_DETAIL_LEGACYREADERPARTS_H

#include <ThermoFun/IO/Legacy/LegacyReader.h>
#include <algorithm>
#include <regex>
#include <cctype>
#include <cmath>
#include <exception>
#include <limits>
#include <set>
#include <nlohmann/json.hpp>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/IO/Equation.h>
#include <ThermoFun/IO/Legacy/LegacyKeys.h>
#include <ThermoFun/IO/Legacy/LegacyMethodMap.h>
#include <ThermoFun/IO/Legacy/LegacyQuirks.h>
#include <ThermoFun/IO/Legacy/LegacyVocabulary.h>

namespace ThermoFun::v2::legacyreader {

using json = nlohmann::json;

constexpr double inf = std::numeric_limits<double>::infinity();

// The number of coefficients of one array that the legacy parser accepts
constexpr size_t maxCoefficients = 1000;

// GEMS writes the smallest float32 for a value that is empty
constexpr double emptyMarker = 1e-30;

struct Context
{
    const LegacyReadOptions& options;
    Diagnostics* diagnostics;
    std::string record;  // symbol of the record, for the diagnostics
};

inline legacy::Where where(const Context& c, const std::string& path = "")
{
    return {c.record, path};
}

struct EntryInfo
{
    const json* entry;
    const legacy::LegacyMethod* method;
    const MethodSpec* spec;
    size_t application;         // index in the list of applications
};

// What the entries of a record say besides its methods
struct MethodFlags
{
    bool gas_critical = false;   // fug_critical_param: a flag, not a method
};

// the keys that the reader handles, per kind of record
inline const std::set<std::string>& knownSubstanceKeys()
{
    static const std::set<std::string> k = {
        legacy::Symbol, legacy::Name, legacy::Formula, legacy::FormulaCharge, legacy::AggregateState, legacy::Class, legacy::Tst,
        legacy::Pst, legacy::SmGibbsEnergy, legacy::SmEnthalpy, legacy::SmEntropyAbs, legacy::SmHeatCapacityP, legacy::SmVolume,
        legacy::TPMethods, legacy::MExpansivity, legacy::MCompressibility, legacy::MassPerMole, legacy::Reaction,
        legacy::Datasources, legacy::References, legacy::Label, legacy::LimitsTP};
    return k;
}

inline const std::set<std::string>& knownReactionKeys()
{
    static const std::set<std::string> k = {
        legacy::Symbol, legacy::Name, legacy::Equation, legacy::Reactants, legacy::Tst, legacy::Pst, legacy::LogKr,
        legacy::DrsmGibbsEnergy, legacy::DrsmEnthalpy, legacy::DrsmEntropy, legacy::DrsmHeatCapacityP, legacy::DrsmVolume,
        legacy::LimitsTP, legacy::TPMethods, legacy::Datasources, legacy::References, legacy::Label};
    return k;
}

inline const std::set<std::string>& knownElementKeys()
{
    static const std::set<std::string> k = {legacy::Symbol, legacy::Name,     legacy::Class,       legacy::Entropy,    legacy::HeatCapacity,
                                            legacy::AtomicMass, legacy::Volume, legacy::IsotopeMass, legacy::Number,
                                            legacy::Datasources, legacy::References, legacy::Label};
    return k;
}

void report(const Context& c, Severity severity, const std::string& code, const std::string& path, const std::string& message);

auto isPresent(const json& j, const char* key) -> bool;

auto stringOf(const json& j, const char* key) -> std::string;

// The key of the first member of a map {"<code>": "<name>"}
auto codeOf(const json& j, const char* key, int fallback) -> int;

// The legacy record is the content of "properties" if the record is wrapped (an export of ArangoDB)
auto unwrap(json j, const Context& c) -> json;

auto parse(const std::string& text, const Context& c) -> json;

// True if the number is exactly a float32: the legacy GEMS exports write such numbers (162.3000030517578)
auto isSinglePrecision(double value) -> bool;

// A value with its unit: {"values": [x], "units": [u], "errors": [e]}
// The slot has a unit of the legacy file (unitDefault) and one in SI (si). A volume in J/bar is per mole: its
// unit in the conversion is m3 (siConvert), and the model keeps m3/mol.
auto readQuantity(const json& j, const char* key, const std::string& unitDefault, const std::string& siConvert, const Context& c)
    -> std::optional<Quantity>;

auto numbersOf(const json& entry, const char* field) -> std::vector<std::optional<double>>;

// The unit of the i-th value of an array: the unit of the file, or the unit of the model if the file gives none
auto unitOfValue(size_t i, const std::vector<std::string>& from, const std::string& model) -> std::string;

auto ownsKey(const legacy::LegacyMethod& method, const std::string& key) -> bool;

// The SI unit of a parameter of a method
auto siUnitOf(const MethodSpec& spec, const std::string& name) -> std::string;

// A legacy volume in J/bar is per mole
auto perMole(const std::string& unit) -> std::string;

// Reads the array of a key into the parameters of a method, in SI units. Values that no parameter takes are reported.
void readArray(const json& entry, const std::string& key, const legacy::LegacyMethod& method, const MethodSpec& spec,
               std::map<std::string, Quantity>& out, const Context& c);

// {lowerT, upperT, lowerP, upperP} of a limitsTP object; a missing bound is no limit
auto readLimits(const json& entry) -> Applicability;

// The T limits of a segment: a limit that the file does not give is infinite
auto segmentOf(const json& entry) -> Segment;

// A row of m_phase_trans_props: T, dS, dH, dV, dTdP
auto readTransition(const json& entry, const Context& c) -> std::map<std::string, Quantity>;

auto readTable(const json& table) -> TableData;

// Reads the keys of an entry that belong to its method into the application
void readEntryKeys(const json& entry, const legacy::LegacyMethod& method, const MethodSpec& spec, MethodApplication& app, const Context& c);

auto readMethods(const json& j, OperatesOn operatesOn, const Context& c, MethodFlags& flags) -> std::vector<MethodApplication>;

auto findMethod(std::vector<MethodApplication>& apps, const char* id) -> MethodApplication*;

// A record has one base method. The legacy logk_fpt_function and dr_heat_capacity_ft only hold coefficients for the
// log K method beside them, so their coefficients move to that method.
void mergeLogKMethods(std::vector<MethodApplication>& apps);

// Water vapour: HKF with the flag fug_critical_param, or with a water equation of state on a record that is not an
// aqueous solute, is water as a gas; so is a water equation of state on a record in the gas state.
void applyWaterVapourRule(SubstanceRecord& s, const MethodFlags& flags);

// alpha0 and kappa0 of the record-level keys m_expansivity and m_compressibility
void readHp98Values(const json& j, SubstanceRecord& s, const Context& c);

// The methods of a record, and what depends on them
void readSubstanceMethods(const json& j, SubstanceRecord& s, const Context& c);

void readReactionMethods(const json& j, ReactionRecord& r, const Context& c);

// Keys that ThermoFun does not read are reported, not kept
void reportUnknownKeys(const json& j, const std::set<std::string>& known, const Context& c);

// "datasources" and "references": a string or a list of strings
auto readSources(const json& j) -> std::vector<std::string>;

} // namespace ThermoFun::v2::legacyreader

#endif
