# 14 — Thermodynamic conventions: declare, convert, compute (decision Q30)

ThermoFun v2 must use data written in **different conventions** (geochemical apparent properties, Berman–Brown,
CALPHAD SER, NASA/JANAF assigned enthalpies, Extended UNIQUAC, DFT energies, 1 atm vs 1 bar, molal vs mole-fraction
solutes, water steam-table references) **in one calculation**, without the user doing conversions by hand and
without silent errors when species of different conventions meet in a reaction.

The rule is simple:

```
 declared input convention  ──►  canonical (internal) convention  ──►  requested output convention
 (per method, record, dataset)    (one, fixed, used by the engine)      (EngineConventions, per engine)
```

- **Declare.** Every value set has a known convention: from the method (its native output convention), else the
  record, else the dataset, else ThermoFun's default. Unknown = default (today's behaviour), with a validation
  warning when the method's native convention differs from the dataset's (§6).
- **Convert in.** Each method's result is converted to the canonical convention right after the method runs,
  before results are combined (§8 of 06) or used in reactions. So mixed records and mixed reactions are always
  consistent.
- **Convert out.** At the end, the canonical result is converted to the convention the user asked for (today's
  `EngineConventions.apparent_properties = "BERMAN_BROWN"` and `water_properties = "STEAM_TABLES"` become cases of
  this general step).

Conversions are cheap closed-form corrections (constants, terms linear in T, R T ln(...)), applied to values,
T and P derivatives and statuses as today's `toBermanBrown`/`toSteamTables` do.

---

## 1. Convention dimensions

| Dimension | Values (v2 strings) | Canonical (internal) | Affects |
|---|---|---|---|
| `energy_reference` | `benson_helgeson` (apparent formation, geochemical), `berman_brown`, `ser` (CALPHAD), `assigned_enthalpy` (NASA, JANAF), `gibbs_helmholtz_formation` (Extended UNIQUAC as published), `dft_formation_0K` | `benson_helgeson` | G, H, S, U, A |
| `standard_pressure` | `1bar` (1e5 Pa), `1atm` (101325 Pa) | `1bar` | G, S of gases (condensed: V·ΔP, optional) |
| `concentration_scale` (solutes) | `molality` (1 mol/kg, infinite dilution), `mole_fraction`, `molarity` (1 mol/L) | `molality` | G, S (and H, Cp, V for molarity, via water density) |
| `water_reference` | `geochemical` (HKF/HGK 1974 triple point reference), `steam_tables` (IAPWS: U, S = 0 at the triple point) | `geochemical` | G, H, S, U, A of water |
| `log_base` (reaction functions) | `log10`, `ln` | log10 (`logK`) | adapters only (v2 always stores log10 K) |
| `amount_basis` | `per_formula`, `per_atom` | per formula unit | adapters (DFT, eV/atom) |
| `reference_temperature` | number (K) | record value, default 298.15 | already in records (`reference_conditions`) |

The existing enum `ConcentrationScales` (`GlobalVariables.h`) and `DatabaseConventions.water_triple_point_reference`
map onto these.

## 2. Energy references in one formula

All energy references used by ThermoFun's sources differ from **Berman–Brown** (`G_BB = ΔfH°(Tr) + ∫Cp dT − T·S°(T)`)
only by terms in Σ S°(elements) of the formula, `Σel` (computed today by `Database::elementalEntropyFormula`):

| `energy_reference` | G − G_BB | S − S° (absolute) | H − H_BB | Source examples |
|---|---|---|---|---|
| `berman_brown` | 0 | 0 | 0 | Berman (1988), Holland–Powell |
| `ser` | 0 (G − H_SER is G_BB: element enthalpies 0 at 298.15 K) | 0 | 0 | SGTE, CALPHAD TDB |
| `assigned_enthalpy` | 0 (H includes ΔfH°(298.15)) | 0 | 0 | NASA-7/9 (`NASA9`), JANAF, Burcat |
| `benson_helgeson` | + Tr · Σel | 0 | 0 | SUPCRT, slop, aq17, PSI/Nagra, Cemdata (ThermoFun default) |
| `gibbs_helmholtz_formation` | + T · Σel | − Σel (entropy is ΔfS) | 0 | Extended UNIQUAC (`ExtendedUNIQUAC`) |
| `dft_formation_0K` | treated as `berman_brown` with ΔfH ≈ ΔfE(0 K) | 0 | 0 | MP, OQMD (estimate; quality `supplemental`) |

So converting from any reference X to the canonical `benson_helgeson`:

```
G_BH = G_X − offsetG_X + Tr·Σel        where offsetG_X = 0 | Tr·Σel | T·Σel
S_BH = S_X − offsetS_X                 where offsetS_X = 0 | −Σel
dG/dT: subtract d(offsetG_X)/dT (0 or Σel);  dS/dT, H, Cp, V unchanged;  U, A recomputed
```

Notes:
- `Σel` needs the element entropies of the dataset's elements (`ElementRecord.standard_entropy`), which every
  ThermoHub database has. A conversion that needs `Σel` when an element entropy is missing is an error
  (`CNV-001`), never a silent zero.
- The convention for **water** (`water_solvent`) is separate (§4): today's code converts water only with
  `toSteamTables`, and non-water species with `toBermanBrown`. This stays.
- **Reactions are invariant** when all species share one reference (Σν·Σel = 0 for a balanced reaction). The
  conversion matters precisely when species of different references meet — e.g. a `NASA9` gas with HKF aqueous
  species — and then it is required for correct log K.

## 3. Standard pressure and concentration scale

**Standard pressure** (gases, ideal-gas standard state):
`S(1 bar) = S(1 atm) + R ln(1.01325)`, `G(1 bar) = G(1 atm) − R T ln(1.01325)`; H, Cp unchanged.
Condensed phases and solutes: `G += V°·(1e5 − 101325)` (optional, `ConversionOptions.condensed_pressure`, default off:
the difference is below data uncertainty).

**Concentration scale** (aqueous solutes; molality is canonical):
- mole fraction → molality: `G_m = G_x − R T ln(1000 / M_w)` with M_w = 18.01528 g/mol (`ln 55.508`);
  `S_m = S_x + R ln(1000/M_w)`; H, Cp, V unchanged.
- molarity → molality: `G_m = G_c + R T ln(ρ_w / 1000)` with ρ_w(T, P) the water density in kg/m³ from the solvent;
  depends on T and P, so S, H, Cp, V also change (derivatives of ln ρ_w — available from the solvent model with
  autodiff). Needs the solvent; error `CNV-002` without one.

## 4. Water

`water_reference`: `geochemical` (canonical; today's default, with `water_triple_point_reference` choosing the
HKF/HGK 1974 reference values) or `steam_tables` (IAPWS reference: U and S of liquid water zero at the triple
point). Input conversion uses the existing `WaterTripleProperties` table (`waterTripleData`); output conversion is
today's `toSteamTables`. Both stay exactly as computed today (golden suite).

## 5. Where conventions are declared

Most specific wins:

1. **Method** — `MethodSpec.native_convention` (built-in) or the v2 Method record's `convention` block: what the method's
   output is in. Examples: `NASA9` → `assigned_enthalpy`; `ExtendedUNIQUAC` → `gibbs_helmholtz_formation`;
   `CpIntegration`, `HKF*`, water and gas methods → `inherit` (the record's reference values decide).
2. **Record** — `standard_state_convention` block on a substance or reaction (v2):
   ```yaml
   convention: {energy_reference: berman_brown, standard_pressure: 1atm}
   ```
3. **Dataset** — `thermodataset.convention` (same block), the default for all its records.
4. **ThermoFun default** — `benson_helgeson`, `1bar`, `molality`, `geochemical` (today's behaviour; legacy files
   get exactly this, so nothing changes for them).

For methods that **inherit**, the convention is that of the reference values they integrate from. A record whose
standard-state properties are in `berman_brown` and whose method is `CpIntegration` produces `berman_brown` output,
which the engine converts.

## 6. Engine: input and output conversion

In `Engine/SubstanceExecutor.cpp` (phase 3, step S3.11):

```
for each method step (06 §8.2 order):
    tps_step = method.evaluate(...)
    tps_step = toCanonical(tps_step, effectiveConvention(method, record, dataset), record, T, P)   # §2–§4
    combine into result
result = engine steps (U, A, Cv; fallback)
result = fromCanonical(result, engine.conventions, record, T, P)          # replaces toBermanBrown / toSteamTables
```

- `toCanonical` is the identity when the effective convention is canonical, so legacy data (all canonical by
  default) take the same code path with zero cost and identical results.
- For an `Add` step (increments), only conventions that change increments matter (standard pressure for gases);
  energy-reference offsets apply to base steps only (the offset is a property of the whole value, not of an
  increment).
- Reactions: computed from species (`fromReactants`) in canonical form, so mixed conventions are consistent; a
  reaction's own `reaction_properties` (log K, ΔrG) carry no element offsets, only `standard_pressure` and
  `concentration_scale` conversions of the species they involve (applied through the species' declared
  conventions, using the stoichiometry).
- `EngineConventions` gains `energy_reference`, `standard_pressure`, `concentration_scale` (output); the existing
  `apparent_properties` and `water_properties` strings stay and map to `energy_reference` and `water_reference`
  (`"BERMAN_BROWN"` → `berman_brown`, `"BENSON_HELGESON"` → `benson_helgeson`, `"STEAM_TABLES"` → `steam_tables`).

## 7. Implementation layout

```
ThermoFun/Conventions/
    Convention.h            struct Convention {energy_reference, standard_pressure, concentration_scale,
                            water_reference}; parse/print; canonical(); merge(most-specific-wins)
    ConventionConverter.h/.cpp
                            toCanonical / fromCanonical for ThermoPropertiesSubstance (values, ddt, ddp, status,
                            err unchanged except where noted); needs Σel (from Database), solvent density (for
                            molarity), water triple data
```

Layer: `Conventions/` depends on `Model/` and `Common/`; the engine and `Validate` use it. `Model/` records hold the
declared `Convention` (optional fields); `MethodSpec` gets `native_convention` (06 §1).

Diagnostics:

| Code | Sev | Meaning |
|---|---|---|
| CNV-001 | E | a conversion needs element entropies that are missing (names the elements) |
| CNV-002 | E | molarity conversion without a solvent |
| CNV-003 | W | a method's native convention differs from the dataset's and the record declares none (conversion applied) |
| CNV-004 | W | a reaction mixes species of different energy references (conversion applied; informational) |
| CNV-005 | E | unknown convention value |

## 8. Tests

- **Same species, every convention**: one mineral (Cp + volume) and one gas written in `benson_helgeson`,
  `berman_brown`, `ser`, `assigned_enthalpy` and `1atm` variants give identical canonical results (1e-12).
- **Mixed reactions**: a reaction between a `NASA9` gas, an `ExtendedUNIQUAC` species and HKF aqueous species gives the
  same log K as the same reaction with all species converted to one convention by hand (independent computation in
  the test).
- **Output conventions**: requesting `berman_brown` / `steam_tables` output reproduces today's `toBermanBrown` /
  `toSteamTables` results bit for bit (golden `berman` and `steam` variants).
- **Concentration scales**: mole-fraction and molarity solutes convert to molality at 25 °C and 200 °C (molarity uses
  ρ_w from the solvent model).
- **Legacy**: golden suite unchanged (all legacy data are canonical by default).

## 9. Relation to other decisions

Supersedes the separate entries Q24 (energy reference) and Q26 (standard pressure), which become parts of this
design and move from Phase 7 to Phase 3 (step S3.11), because the `new_models` methods (`NASA9`,
`ExtendedUNIQUAC`, Q29) already need them. Phase 7 keeps only source-specific work (SER element data for SGTE,
DFT amount basis). Proposed to the ThermoHub schema as a `convention` block on datasets, records and Method records.
