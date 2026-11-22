<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Allowed words

Some fields of a record take one word of a fixed list, for example `class: component` or `aggregate_state: crystal`. A word that is not in the list gives the warning `V2-VOC-001`, with the nearest word (`Did you mean "crystal"?`), and the value `other` is used. Capitals and `-` for `_` are accepted.

## Substance

| Field | Allowed words |
|---|---|
| `class` | `component` (also `condensed_species`; see below), `gas_fluid`, `aqueous_solute`, `water_solvent` (also `solvent`), `surface_species`, `site_moiety`, `other` (default) |
| `aggregate_state` | `gas`, `liquid`, `glass`, `crystal` (also `crystalline`), `aqueous`, `surface`, `ion_exchange`, `plasma`, `solid`, `condensed`, `fluid`, `liquid_crystal`, `amorphous_solid`, `monomeric`, `polymeric`, `solid_solution`, `other` (default) |
| `preferred_property_source` | `direct` (the methods of the record), `reaction` (the defining reaction), `reactants`, `record` |

### What `class` means

The class says which family of rules ThermoFun applies to a substance (for example, the methods for aqueous species need the solvent). The aggregate state says only the state of matter. A substance has both.

| `class` | What it is | Examples |
|---|---|---|
| `component` (or `condensed_species`) | A component of a solid, a solid solution, a melt or another condensed phase: the substances that are none of the other classes. The name comes from the old format, where it meant a component of a phase. | calcite, quartz, a glass, a melt end member |
| `gas_fluid` | A gas or a fluid species. | CO2(g), H2O(g) |
| `aqueous_solute` | A species dissolved in the solvent. It is not a `component`, although it is a component of the aqueous phase. | Ca+2, NaCl(aq) |
| `water_solvent` (or `solvent`) | The solvent. Today only water; a solvent other than water is not implemented. | H2O@ |
| `surface_species`, `site_moiety` | A species on a surface, and a site of a solid solution. | |
| `other` | Not said. | |

The words in brackets are accepted when a file is read, and the record is written with the first word, the word of the format. The class and the aggregate state do not decide how a record is calculated: its methods do ([How methods combine](how-methods-combine.md)).

## Element

| Field | Allowed words |
|---|---|
| `class` | `element`, `charge`, `other` |

## Conventions

| Field | Allowed words |
|---|---|
| `standard_state.pressure` | `1bar` (default), `1atm` |
| `standard_state.concentration_scale` | `molality` (default), `molarity`, `mole_fraction` (also `amount_concentration`, `amount_fraction`) |
| `energy_reference.elements` | `benson_helgeson` (default), `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K` |
| `energy_reference.water` | `geochemical` (default), `steam_tables` |

See [Conventions](conventions.md) for what each word means.

## Methods and tables

| Field | Allowed words |
|---|---|
| `operates_on` of a method | `substance`, `reaction` |
| `kind` of a method | `symbolic`, `reference`, `composite` |
| `interpolation` of a table of values | `lagrange`, `cubic_spline` (also `spline`), `bilinear` |
| `status` of a dataset | `draft`, `published` |

The methods that a record can use are in the [list of methods](methods/index.md), the units in [Units](units.md), and the codes of the warnings in [Diagnostics](diagnostics.md).
