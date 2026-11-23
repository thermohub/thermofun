# Allowed words

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`. The words are in `docs/reference/vocabulary.yaml`.

Some fields of a record take one word of a fixed list, for example `class: component` or `aggregate_state: crystal`. A word that is not in the list gives the warning `V2-VOC-001`, with the nearest word (`Did you mean "crystal"?`), and the value `other` is used. Capitals and `-` for `_` are accepted.

## Substance

| Field | Allowed words |
|---|---|
| `class` | `component` (also `condensed_species`; see below), `gas_fluid`, `aqueous_solute`, `water_solvent` (also `solvent`), `surface_species`, `site_moiety`, `other` (default) |
| `aggregate_state` | `gas`, `liquid`, `glass`, `crystal` (also `crystalline`), `aqueous`, `surface`, `ion_exchange`, `plasma`, `solid`, `condensed`, `fluid`, `liquid_crystal`, `amorphous_solid`, `monomeric`, `polymeric`, `solid_solution`, `other` (default) |
| `preferred_property_source` | `direct` (the methods of the record), `reaction` (the defining reaction), `reactants`, `record` |

### What `class` means

The class says which family of rules ThermoFun applies to a substance (for example, the methods for aqueous species need the solvent), and each class has its standard state. The aggregate state says only the state of matter. A substance has both.

| `class` | What it is | Standard state | Examples |
|---|---|---|---|
| `component` (or `condensed_species`) | A component of a solid, a solid solution, a melt or another condensed phase: the substances that are none of the other classes. The name comes from the old format, where it meant a component of a phase. | the pure phase at p° | calcite, quartz, a glass, a melt end member |
| `gas_fluid` | A gas or a fluid species. | the ideal gas at p° | CO2(g), H2O(g) |
| `aqueous_solute` | A species dissolved in the solvent. It is not a `component`, although it is a component of the aqueous phase. | the ideal-dilute solution at m° = 1 mol/kg (or c°, x°) and p° | Ca+2, NaCl(aq) |
| `water_solvent` (or `solvent`) | The solvent. Today only water; a solvent other than water is not implemented. | the pure phase at p° | H2O@ |
| `surface_species` | A species on a surface. | as a solute, on the molality scale, at p° | |
| `site_moiety` | A site of a solid solution. | none of its own: part of the end members | |
| `other` | Not said. | | |

The words in brackets are accepted when a file is read, and the record is written with the first word, the word of the format. The class and the aggregate state do not decide how a record is calculated: its methods do ([How methods combine](how-methods-combine.md)).

## Element

| Field | Allowed words |
|---|---|
| `class` | `element`, `charge`, `other` |

## Conventions

| Field | Allowed words |
|---|---|
| `standard_state.pressure` | `1bar` (default), `1atm`, or any pressure |
| `standard_state.concentration_scale` | `molality` (default), `molarity`, `mole_fraction` (also `amount_concentration`, `amount_fraction`) |
| `energy_reference.elements` | `benson_helgeson` (default), `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K` |
| `energy_reference.water` | `helgeson_kirkham_1974` (default), `codata_1989` (the anchors, data of water), `steam_tables` (a scale) |
| `energy_reference.ions` | `hydrogen_ion` (default), `none` |

See [Conventions](conventions.md) for what each word means.

## Methods and tables

| Field | Allowed words |
|---|---|
| `operates_on` of a method | `substance`, `reaction` |
| `kind` of a method | `symbolic`, `reference`, `composite` |
| `contributes_to` of a method | `sets` (the whole value), `adds` (an increment to the value before), `uses` (an input only); a property not named is not touched |
| `interpolation` of a table of values | `lagrange`, `cubic_spline` (also `spline`), `bilinear` |
| `status` of a dataset | `draft`, `published` |

The methods that a record can use are in the [list of methods](methods/index.md), the units in [Units](units.md), and the codes of the warnings in [Diagnostics](diagnostics.md).

## Results and batch calculations

| Field | Allowed words |
|---|---|
| `by` of a route | `methods`, `reaction`, `reactants`, `water`, `reference`, `hydrogen_ion`, `hydrogen_electrode` |
| `role` of a need of a route | `solvent`, `defining_reaction`, `reactant` |
| `what` of a conversion of a route | `convention`, `gas_options`, `fallback`, `rounding` |
| part of `routeTable` | `contribution` (default), `terms`, `variables` |
| `show` of a property of a batch result | `value` (default), `uncertainty`, `ddt`, `ddp`, `status` |
| `points` of the conditions | `grid` (default), `pairs` |
| `vary_first` of the conditions | `P` (default), `T` |
| `layout` of the output | `long` (default), `wide`, `grid` |
| `rows_by` of the output | `substance` (default), `condition` |
| `provenance` of the output | `header` (default), `file`, `none` |

See [Results](results.md), [See how a result is calculated](../how-to/see-how-a-result-is-calculated.md) and [Batch setups](batch.md).
