# 07 — Units and uncertainty

## 1. Principles (D12, D14)

- **Records are SI** (§03): J/mol, J/(mol·K), m³/mol, K, Pa, kg/mol; dimensionless = `1`.
- **Files may use any supported unit**; conversion happens once, when reading (`Normalize`, `LegacyReader`).
- **Model classes keep their own units** (bar, J/bar, cal, °C for one Landau parameter); `Binding` converts SI →
  `ParameterSpec::model_unit` once per record (§06 §6).
- **Engine inputs and outputs do not change**: K, Pa, J/mol, J/(mol·K), and volume in J/bar as today.
- `cal` is the thermochemical calorie, 4.184 J (already so in `Common/Units.cpp`); `cal_IT` = 4.1868 J is added.

## 2. The existing converter

`ThermoFun::units::convert(value, from, to)` and `convertible(from, to)` (`Common/Units.hpp`, derived from
Reaktoro) parse strings with `*`, `/`, `^`, parentheses, metric-prefixed atoms (`kJ`, `cm`, `kbar`), volume atoms
(`m3`, `cm3`, `L`), and affine temperatures (`degC`, `degF`, `degR`, `kelvin`, `celsius`). All legacy unit
strings in the bundled data (`J/(mol*K)`, `(cal*K)/(mol*bar)`, `(cm^3*K^0.5)/g`, `J/bar`, `K/bar`, `1e-05/K` via
quirk) are in this syntax.

## 3. UCUM front end (`Common/Ucum.{h,cpp}`)

v2 files use UCUM case-sensitive codes (D14). A small translator maps them to the converter's syntax; it does not
implement all of UCUM, only what thermodynamic data uses, and fails loudly on the rest.

```cpp
namespace ThermoFun::units {
/// Translate a UCUM unit string (or a string already in converter syntax) to converter syntax.
/// Throws std::invalid_argument naming the unsupported token.
auto fromUcum(const std::string& ucum) -> std::string;
/// Convert a value from a unit (UCUM or converter syntax) to another.
auto convertAny(double value, const std::string& from, const std::string& to) -> double;
/// Convert a difference (uncertainty, temperature interval): scale only, no offset.
auto convertDifference(double delta, const std::string& from, const std::string& to) -> double;
/// The dimension vector of a unit (exponents of kg, m, s, K, mol, A, cd), for slot checks.
auto dimension(const std::string& unit) -> std::array<int, 7>;
}
```

Translation rules, applied in order:

| UCUM | Converter | Example |
|---|---|---|
| `.` between atoms (multiplication) | `*` | `J/(mol.K)` → `J/(mol*K)` |
| atom followed by an integer exponent, e.g. `K2`, `cm3`, `s-1` | `atom^n` | `J/(mol.K2)` → `J/(mol*K^2)`; `cm3/mol` → `cm^3/mol` |
| `Cel` | `degC` | |
| `[degF]` | `degF` | |
| `[atm]` | `atm` | |
| `cal_IT` | `cal_IT` (new atom, 4.1868 J) | |
| `cal_th` | `cal` | |
| `10*n` factors (`10*-5/K`) | `1e-5*` prefix factor | |
| `1` | `1` | |
| `%` | `percent` | |

A string that contains `*` or `^` is taken as converter syntax and not translated (legacy compatibility). Tests:
a table of ~40 pairs in `tests/model/test_ucum.cpp`, including every unit string that occurs in the bundled data
and every `si_unit`/`model_unit` in the method catalog.

## 4. Dimension check

Every slot has an SI unit (§03 §2.1, §06 parameter tables). `Normalize` checks
`dimension(unit_as_written) == dimension(slot_si_unit)` (`V2-UNIT-002`). Temperature-offset units count as K.
Molar vs specific quantities differ in dimension (mol), so they cannot be mixed by mistake (D14).

## 5. Uncertainty

### 5.1 Model (§03 `Uncertainty`)

Stored as the standard uncertainty u (k = 1) in SI. Kind primary/derived (D11). Optional distribution and degrees
of freedom (D9-D). Missing ≠ zero.

### 5.2 Reading

| Source | Rule |
|---|---|
| v2 `uncertainty: x` | u = x (in the value's unit, converted as a difference) |
| v2 `{standard: u}` | u |
| v2 `{expanded: U, coverage_factor: k}` | u = U / k; `coverage_factor_as_written = k` |
| legacy `errors: [e]` | u = convertDifference(e) / `LegacyReadOptions::error_coverage_factor` (default 1 = today's behaviour); `convention_unknown = true` |
| legacy coefficient `errors` | per coefficient, as today (`read_values_units` converts `value+error` and takes the difference — kept for legacy; v2 uses `convertDifference`) |

Note: for affine units the legacy way (`convert(v+e) − convert(v)`) and `convertDifference` agree; for non-linear
conversions they would not, but no such unit exists in the converter.

### 5.3 Using uncertainties in calculations (no change)

The engine propagates first-order uncertainties today: `ThermoScalar::err` on reference properties, and
coefficient errors through `uncertainCoefficients()` (`ThermoParameters.h`) with autodiff. Binding fills the
existing fields from the record:
- `standard_state_properties[X].uncertainty->standard` → reference property `err` (in the engine's units);
- parameter uncertainties → `ThermoParametersSubstance::coefficient_errors[legacy_key][row][legacy_index]`, so
  `uncertainCoefficients()` works unchanged.

### 5.4 Writing

v2: `uncertainty: u` (bare) when only `standard` is set, kind primary, `coverage_factor_as_written == 1`; otherwise
the object form, written back with the coverage factor it was read with (`expanded = u·k`). Legacy: `errors:
[u·error_coverage_factor]`.

### 5.5 Not in scope

Covariance (D9-E) and Monte-Carlo propagation (D10) are not implemented; the reader keeps such fields in
`extra_json` (warning `V2-KEY-001`) so they round-trip.
