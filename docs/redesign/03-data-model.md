# 03 — The format-neutral data model (`ThermoFun/Model/`)

The model holds what a file says, independent of whether it came from a legacy or a v2 file. Readers fill it;
writers read it; binding (§06) turns it into inputs for the model classes. It follows the v2 concepts (D8, D9,
D13, D15–D18, D24, D45, D64) but contains only what ThermoFun uses or must round-trip.

Rules:
- **All numeric values are SI** after reading: J/mol, J/(mol·K), m³/mol, K, Pa (§07). The unit written in the
  file is kept in `Quantity::unit_as_written` only for writing the file back.
- **Missing ≠ zero.** Optional fields are `std::optional`; a missing uncertainty is not a zero uncertainty (D9).
- **Plain structs with public fields**, value semantics, no virtual functions, no nlohmann types.
- `extra_json` keeps unknown fields of a record (JSON object text) so writers can round-trip them.

## 1. `Model/Quantity.h`

```cpp
#pragma once
#include <optional>
#include <string>
#include <vector>

namespace ThermoFun {

/// GUM-style uncertainty of a value (D9, D11). Stored as a standard uncertainty (k = 1) in SI units.
struct Uncertainty
{
    enum class Kind { Primary, Derived };

    /// The standard uncertainty u (k = 1), in the SI unit of the value it belongs to.
    double standard = 0.0;

    /// The coverage factor the uncertainty was written with in the file (u = U / k at reading); 1 if not given.
    double coverage_factor_as_written = 1.0;

    /// Primary (from measurement / evaluation, stored) or derived (computed by propagation).
    Kind kind = Kind::Primary;

    /// Optional distribution name ("normal", "uniform", ...), empty if not given (D9-D).
    std::string distribution;

    /// Optional degrees of freedom (D9-D).
    std::optional<double> degrees_of_freedom;

    /// True if the uncertainty came from a legacy file whose convention (u, U, CI) is not stated (05 §3.2).
    bool convention_unknown = false;
};

/// Optional metadata of one value (D17, D26). Empty strings mean "not given here" (see the fallback chain D48).
struct ValueMeta
{
    std::string quality;            ///< "core" | "recommended" | "supplemental" | "" (O50)
    std::string origin;             ///< "measured" | "entered" | "calculated" | "estimated" | "converted" | ""
    std::string calculation_path;   ///< free text when origin == "calculated"
    std::string source;             ///< DataSource symbol, e.g. "1995ROB/HEM"
};

/// A numeric value with its uncertainty and metadata (D9, D13, D17). `value` is in SI units.
struct Quantity
{
    double value = 0.0;                         ///< in the SI unit of the slot it fills
    std::string unit_as_written;                ///< unit string as in the file ("" = the default unit was used)
    std::optional<Uncertainty> uncertainty;     ///< missing = not assessed (not zero)
    ValueMeta meta;

    /// True if the value carries no metadata, no uncertainty and used the default unit (a bare number in v2).
    auto isBare() const -> bool;
};

/// A quantity that is a list of numbers sharing one unit (e.g. legacy coefficient arrays kept as written).
struct QuantityList
{
    std::vector<double> values;                 ///< SI
    std::vector<std::string> units_as_written;  ///< one per value, or empty
    std::vector<std::optional<double>> standard_uncertainties;
};

} // namespace ThermoFun
```

## 2. `Model/Vocabulary.h`

Controlled vocabularies as `enum class` with string conversion. The **string** is what v2 files contain; the
legacy integer codes are only used by the legacy reader/writer and the old API.

```cpp
namespace ThermoFun {

enum class PropertyName { G, H, S, Cp, V, U, A, Cv };   ///< standard-state properties of substances
enum class ReactionPropertyName { logK, lnK, dG, dH, dS, dCp, dV, dU, dA };

auto toString(PropertyName p) -> std::string;
auto propertyNameFromString(const std::string& s) -> std::optional<PropertyName>;
// same pair of functions for every enum below
}
```

### 2.1 Property names and SI units

| v2 key | Meaning | SI unit (UCUM) | Legacy key (substance) | Legacy unit |
|---|---|---|---|---|
| `G` | Δf G° (apparent) | `J/mol` | `sm_gibbs_energy` | J/mol |
| `H` | Δf H° | `J/mol` | `sm_enthalpy` | J/mol |
| `S` | S° (absolute) | `J/(mol.K)` | `sm_entropy_abs` | J/(mol*K) |
| `Cp` | Cp° | `J/(mol.K)` | `sm_heat_capacity_p` | J/(mol*K) |
| `V` | V° | `m3/mol` | `sm_volume` | J/bar (= 1e-5 m³/mol) |
| `U`, `A`, `Cv` | derived, rarely stored | J/mol, J/mol, J/(mol.K) | — | — |

| v2 key (reaction) | Meaning | SI unit | Legacy key | Legacy unit |
|---|---|---|---|---|
| `logK` | log10 K | `1` | `logKr` | 1 |
| `dG` | Δr G° | `J/mol` | `drsm_gibbs_energy` | J/mol |
| `dH` | Δr H° | `J/mol` | `drsm_enthalpy` | J/mol |
| `dS` | Δr S° | `J/(mol.K)` | `drsm_entropy` | J/(mol*K) |
| `dCp` | Δr Cp° | `J/(mol.K)` | `drsm_heat_capacity_p` | J/(mol*K) |
| `dV` | Δr V° | `m3/mol` | `drsm_volume` | J/bar |

Note: the engine computes volumes in J/bar today. The model stores m³/mol; the conversion back to J/bar happens
when the legacy view (`thermoReferenceProperties()`) is built (§05 §5), so results do not change.

### 2.2 Substance class (O30/O43) ↔ legacy `SubstanceClass`

| v2 `class` | Legacy code / name | Count in bundled data |
|---|---|---|
| `component` | 0 `SC_COMPONENT` (minerals, liquids, …) | 1580 |
| `gas_fluid` | 1 `SC_GASFLUID` | 481 |
| `aqueous_solute` | 2 `SC_AQSOLUTE` | 3452 |
| `water_solvent` | 3 `SC_AQSOLVENT` | 20 |
| `surface_species` | 4 `SC_SURFSPECIES` | 0 |
| `site_moiety` | 5 `SC_SITEMOIETY` | 0 |
| `other` | 101 `SC_OTHER` | 0 |

The design summary lists richer values (`mineral`, `pure_fluid`, `solid_solution`); the table above is the
lossless mapping of today's data. Finer classes are an open point for the schema (§12 Q12); ThermoFun treats any
unknown class string as `other` with a warning.

### 2.3 Aggregate state ↔ legacy `AggregateState`

| v2 `aggregate_state` | Legacy | | v2 | Legacy |
|---|---|---|---|---|
| `gas` | 0 GAS | | `solid` | 9 SOLID |
| `liquid` | 1 LIQUID | | `condensed` | 10 CONDENSED |
| `glass` | 2 GLASS | | `fluid` | 11 FLUID |
| `crystal` | 3 CRYSTAL | | `liquid_crystal` | 12 LIQUIDCRYSTAL |
| `aqueous` | 4 AQUEOUS | | `amorphous_solid` | 13 AMORPHOUSSOLID |
| `surface` | 5 SURFACE | | `monomeric` | 14 MONOMERIC |
| `ion_exchange` | 6 IONEX | | `polymeric` | 15 POLYMERIC |
| `plasma` | 8 PLASMA | | `solid_solution` | 16 SOLIDSOLUTION |
| `other` | 101 OTHER (also the value when missing) | | | |

(The examples of the design summary use `crystalline`; `crystal` is kept here to match the legacy names and
Reaktoro. Accept `crystalline` as an alias when reading — §12 Q12.)

### 2.4 Element class ↔ legacy element `class_`

| v2 `class` | Legacy | Count |
|---|---|---|
| `element` | 0 `ELEMENT` | 490 |
| `isotope` | 1 `ISOTOPE` | 0 |
| `ligand` | 2 `LIGAND` | 0 |
| `charge` | 4 `CHARGE` | 14 |
| `other` | other codes | 0 |

(Verify the codes 1–3 against `Element.cpp` / ChemicalFun before writing the table into code.)

### 2.5 Other vocabularies

| Name | Values |
|---|---|
| `Quality` (O50) | `core`, `recommended`, `supplemental` |
| `Origin` (O51) | `measured`, `entered`, `calculated`, `estimated`, `converted` |
| `PropertySource` (D16) | substance: `direct`, `reaction`; reaction: `direct`, `reactants` |
| `DatasetStatus` (D40) | `draft`, `published` |
| `MethodTier` (D4) | 1, 2, 3 |
| `MethodKind` (D5) | `symbolic`, `reference`, `composite` |
| `OperatesOn` (D23) | `substance`, `reaction` |

## 3. `Model/Applicability.h`

```cpp
/// T-P range in which a method application is valid (D19), SI. Missing bound = no limit.
struct Applicability
{
    std::optional<double> t_min, t_max;   ///< K
    std::optional<double> p_min, p_max;   ///< Pa
    bool is_hard = false;                 ///< true: outside the range is an error (today's TPlimit::isHard)
};

/// One piece of a piecewise-in-T method (several Cp intervals, phase transitions), Cantera-style ranges.
struct Segment
{
    double t_min = 0.0;                   ///< K
    double t_max = 0.0;                   ///< K
    std::map<std::string, Quantity> parameters;   ///< named parameters of this piece (SI)
};
```

## 4. `Model/MethodApplication.h`

```cpp
/// The use of one method on one record (D13, D45): which method, its parameter values, its range.
struct MethodApplication
{
    std::string method;                          ///< method id after alias resolution, e.g. "HKFReaktoro"
    std::string method_as_written;               ///< id or legacy name as in the file, for round-trip
    std::map<std::string, Quantity> parameters;  ///< symbol-as-key (D13), SI values
    std::string parameter_set;                   ///< reference to a MethodParameterSet (Tier 3), "" if none
    Applicability applicability;
    std::vector<Segment> segments;               ///< piecewise methods; empty for others
    /// phase transitions inside the range, each {T, dS, dH, dV, dTdP} (legacy m_phase_trans_props rows, 06 §4.1)
    std::vector<std::map<std::string, Quantity>> transitions;
    std::string extra_json;                      ///< unknown fields of this entry
};
```

Ordering: `SubstanceRecord::methods` keeps file order (for round-trip). The executor derives the order from the methods' declared effects (§06 §8); file order matters only among transformations.

## 5. Records

### 5.1 `Model/ElementRecord.h`

```cpp
struct ElementRecord
{
    std::string symbol;                 ///< "Fe", "Zz" (charge)
    std::string name;
    std::string class_ = "element";     ///< §2.4
    std::optional<int> isotope_mass;
    std::optional<int> number;          ///< atomic number
    std::optional<int> valence;
    std::optional<Quantity> molar_mass;          ///< kg/mol in SI (legacy atomic_mass g/mol)
    std::optional<Quantity> standard_entropy;    ///< J/(mol.K) (legacy entropy, "S0i")
    std::optional<Quantity> heat_capacity;       ///< J/(mol.K) (legacy heat_capacity)
    std::optional<Quantity> volume;              ///< m3/mol
    std::string reference_form;         ///< e.g. "graphite" (O36), optional
    std::vector<std::string> data_sources;
    std::string sourcetdb;
    std::string extra_json;
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;          ///< the record text as read (jsonString() returns it)
};
```

Note on molar mass: the legacy API returns g/mol (`Element::molarMass()`); the view converts (§05 §5).
Decision on whether the model keeps g/mol for molar masses to avoid conversions: §12 Q6 (recommendation: SI,
kg/mol, in the model; the views convert).

### 5.2 `Model/SubstanceRecord.h`

```cpp
struct ReferenceConditions { double T = 298.15; double P = 1e5; };   ///< K, Pa (D8)

struct SubstanceRecord
{
    // identity (D24)
    std::string symbol;                    ///< unique within a ThermoDataSet (D47)
    std::string class_ = "other";          ///< §2.2
    std::string sourcetdb;

    // description
    std::string name;
    std::string formula;                   ///< ChemicalFun formula
    std::optional<int> charge;             ///< legacy formula_charge
    std::string aggregate_state = "other"; ///< §2.3
    std::optional<Quantity> molar_mass;    ///< kg/mol (legacy mass_per_mole g/mol)

    // standard state (D8)
    std::string standard_state_convention; ///< "" = dataset default
    ReferenceConditions reference_conditions;
    std::map<PropertyName, Quantity> standard_state_properties;   ///< G, H, S, Cp, V (D13)

    // how properties at T, P are obtained
    std::vector<MethodApplication> methods;            ///< ordered (D13, D45)
    std::string defining_reaction;                      ///< reaction symbol (D15); legacy "reaction"
    Applicability applicability;                        ///< record-level range (legacy record limitsTP) [profile]
    std::string preferred_property_source;              ///< "direct" | "reaction" | "" (D16)

    // provenance and lifecycle
    std::string data_source;                            ///< record-level default source (D48)
    std::vector<std::string> data_sources;              ///< legacy "datasources"/"references" as written
    bool deprecated = false;                            ///< D39
    std::string deprecated_in_favour_of;

    // round-trip
    std::string extra_json;
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;
};
```

### 5.3 `Model/ReactionRecord.h`

```cpp
struct ReactionRecord
{
    std::string symbol;                       ///< legacy: equals the symbol of the substance it defines
    std::string sourcetdb;
    int level = 0;                            ///< D27 (legacy has none: 0)
    std::string name;
    std::string equation;                     ///< "1 Na+ + 1 HCO3- = NaHCO3@" (as written)
    std::vector<std::pair<std::string,double>> stoichiometry;  ///< ordered, ν < 0 reactants, > 0 products (D18)
    ReferenceConditions reference_conditions;
    std::map<ReactionPropertyName, Quantity> reaction_properties;  ///< logK, dG, dH, dS, dCp, dV
    std::vector<MethodApplication> methods;
    Applicability applicability;              ///< record-level range (legacy record limitsTP) [profile]
    std::string preferred_property_source;    ///< "direct" | "reactants" | ""
    std::string data_source;
    std::vector<std::string> data_sources;
    bool deprecated = false;
    std::string extra_json;
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;
};
```

Stoichiometry is a vector of pairs (not a map) to keep the written order for round-trip; lookups build a map.

### 5.4 Project-scoped records (D64)

```cpp
/// Declares one parameter of a method (in a v2 Method record or in a MethodSpec, §06).
struct ParameterDefinition
{
    std::string name;            ///< "a1"
    std::string unit;            ///< SI unit as UCUM, "J/(mol.Pa)"
    std::string description;
    bool required = true;
    std::optional<double> default_value;   ///< SI
};

/// A reference test of a method (D63): inputs → expected output ± tolerance.
struct ReferenceTestCase
{
    double T = 298.15, P = 1e5;                         ///< K, Pa
    std::map<std::string, double> parameters;           ///< SI
    std::map<std::string, double> expected;             ///< property name → SI value
    double relative_tolerance = 1e-9;
};

/// A v2 Method record (project scope, D64). ThermoFun reads it, checks it against its MethodSpec (§06),
/// and uses `expression` only for Tier-1 methods without a coded implementation (phase 5).
struct MethodDefinition
{
    std::string symbol;                 ///< method id, "HKFReaktoro"
    std::string kind = "reference";     ///< symbolic | reference | composite
    int tier = 2;
    std::string operates_on = "substance";
    std::vector<std::string> applies_to_classes;
    std::vector<std::string> contributes_to;   ///< "G","H","S","Cp","V"
    std::vector<ParameterDefinition> parameter_definitions;
    std::string implementation_id;      ///< for reference methods
    std::string expression;             ///< for symbolic methods (SymEngine syntax)
    std::vector<ReferenceTestCase> tests;
    std::string effects_json;           ///< v2 `effects` block as written (§06 §8.6), checked against the spec
    std::string citation;               ///< DataSource symbol
    std::string extra_json;             ///< includes composite-method fields (D7) kept but not executed
};

/// A shared, citable set of method constants (Tier 3, D4, O84).
struct MethodParameterSet
{
    std::string symbol;                 ///< "IAPWS95_constants_v1"
    std::string method;                 ///< method id it belongs to
    std::map<std::string, Quantity> parameters;
    std::map<std::string, Quantity> auxiliary;
    std::string reference;              ///< DataSource symbol
    std::string extra_json;
};

/// A citation (D26). `symbol` follows the NEA pattern YYYYAAA/BBB when possible.
struct DataSource
{
    std::string symbol;                 ///< "2017MIR/WAG"
    std::string title, authors, journal, doi, bibtex;
    std::optional<int> year;
    std::string extra_json;
};

/// Metadata of the dataset being loaded (D32, D40, D48), plus calculation conventions.
struct ThermoDataSetInfo
{
    std::string symbol;                 ///< "aq17", "cemdata18"
    std::string name;
    std::string version;                ///< required if status == "published"
    std::string status = "draft";
    std::string schema_version;         ///< version of the v2 format of the file ("" for legacy)
    std::string standard_state_convention = "IUPAC_1bar";
    ReferenceConditions reference_conditions;
    std::vector<std::string> sourcetdb;
    std::string solvent;                ///< symbol of the aqueous solvent ("H2O@"), "" = search (today's rule)
    ValueMeta defaults;                 ///< dataset-level fallback for value metadata (D48)
    std::string default_data_source;
    std::string water_triple_point_reference = "HKF_HGK_1974";   ///< today's DatabaseConventions
    std::vector<std::string> legacy_datasources;                 ///< legacy top-level "datasources"
    std::string legacy_date;                                     ///< legacy top-level "date"
    std::string extra_json;
};
```

## 6. Where records live

`Database::Impl` gains:

```cpp
std::map<std::string, MethodDefinition>   methods_map;          // project scope
std::map<std::string, MethodParameterSet> parameter_sets_map;
std::map<std::string, DataSource>         data_sources_map;
ThermoDataSetInfo                         dataset_info;
uint64_t                                  revision = 0;         // incremented by every add/set (binding cache key)
```

Elements, substances and reactions stay in today's maps (`ElementsMap`, `SubstancesMap`, `ReactionsMap`) as
`Element`, `Substance`, `Reaction` objects; each object now owns its record (§05 §5). This keeps
`mapSubstances()` (used by Reaktoro) returning the same type.

## 7. Invariants checked by `Validate` (§04 §7)

- every `symbol` non-empty and unique per record type in the database;
- every `methods[i].method` known to the registry (after aliases) or defined by a `MethodDefinition`;
- every required parameter of a method present (inline, in its `parameter_set`, or with a default);
- `parameter_set` exists and belongs to the same method;
- `defining_reaction` exists; the reaction contains the substance; no cycles through defining reactions;
- stoichiometry symbols exist as substances (warning, as today reactions can name missing species);
- reaction balance of elements and charge (warning; uses ChemicalFun);
- applicability: `t_min < t_max`, `p_min < p_max`; segments sorted, contiguous or with gaps reported;
- units: every unit string parses and has the dimension of its slot.
