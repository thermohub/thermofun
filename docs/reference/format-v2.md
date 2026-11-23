<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# The v2 format

A file is YAML or JSON. [Add data](../how-to/add-data.md) explains each field with examples. This page lists them. The [JSON Schema](format-v2-schema.md) gives the type of each key, and checks a file.

## A file

| Key | Content |
|---|---|
| `thermohub` | The version of the format. It marks the file as v2. |
| `generator` | The tool that wrote the file. |
| `thermodataset` | Symbol, name, version, conventions and solvent. |
| `units` | The default units of the file. |
| `include` | Other files to read. |
| `elements`, `substances`, `reactions` | The records, by symbol. |
| `methods` | Method records. |
| `parameter_sets` | Named groups of values. |
| `data_sources` | Citations. |

A key that starts with `_` is ignored. A file with one record has `record_type`, and no `thermohub`.

## A substance

`name`, `formula`, `charge`, `class`, `aggregate_state`, `molar_mass`, `sourcetdb`, `standard_state`, `energy_reference`, `reference_conditions`, `standard_state_properties`, `methods`, `defining_reaction`, `preferred_property_source`, `applicability`, `data_sources`, `structural`, `deprecated`, `deprecated_in_favour_of`, `units`, `covariances`.

## A reaction

`name`, `equation`, `stoichiometry`, `reference_conditions`, `reaction_properties`, `methods`, `level`, `data_sources`, `applicability`, `standard_state`, `energy_reference`, `preferred_property_source`, `sourcetdb`, `deprecated`, `units`, `covariances`.

## An element

`name`, `class`, `number`, `isotope_mass`, `valence`, `molar_mass`, `standard_entropy`, `heat_capacity`, `volume`, `reference_state`, `data_sources`, `sourcetdb`, `structural`, `units`, `legacy_class`.

## A method in a record

`method`, `implementation`, `parameters`, `parameter_set`, `segments`, `transitions`, `applicability`, `region`, `table`, `interpolation`, `order`, `units`.

`applicability` says where a method is valid: outside it the value is calculated, with a warning. `region` (`t_min`, `t_max`, `p_min`, `p_max`, in K and Pa) says where a method is used: see [Methods by region](how-methods-combine.md#methods-by-region).

## A parameter set

`method`, `implementation`, `default`, `formula_units`, `description`, `data_sources`, `parameters`, `units`.

## A Method record

`kind`, `operates_on`, `applies_to_classes`, `contributes_to`, `parameter_definitions`, `implementation_id`, `formula`, `expression`, `generator`, `formula_hash`, `tests`, `data_sources`.
`contributes_to` maps each property to `sets` (the whole value), `adds` (an increment) or `uses` (an input only), for example
`{G: sets, H: sets, S: sets, Cp: sets}`; a property not listed is not touched. A list of properties is the short form, in which
each adds. See [Add a method as data](../how-to/add-a-method-as-data.md).
