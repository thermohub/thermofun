<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# The v2 format

A file is YAML or JSON. [Add data](../how-to/add-data.md) explains each field with examples. This page lists them.

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

`name`, `formula`, `charge`, `class`, `aggregate_state`, `molar_mass`, `sourcetdb`, `convention`, `standard_state_convention`, `reference_conditions`, `standard_state_properties`, `methods`, `defining_reaction`, `preferred_property_source`, `applicability`, `data_source`, `data_sources`, `structural`, `deprecated`, `deprecated_in_favour_of`, `units`, `covariances`.

## A reaction

`name`, `equation`, `stoichiometry`, `reference_conditions`, `reaction_properties`, `methods`, `level`, `data_source`, `data_sources`, `applicability`, `convention`, `preferred_property_source`, `sourcetdb`, `deprecated`, `units`, `covariances`.

## An element

`name`, `class`, `number`, `isotope_mass`, `valence`, `molar_mass`, `standard_entropy`, `heat_capacity`, `volume`, `reference_form`, `data_sources`, `sourcetdb`, `structural`, `units`, `legacy_class`.

## A method in a record

`method`, `implementation`, `parameters`, `parameter_set`, `segments`, `transitions`, `applicability`, `table`, `interpolation`, `order`, `units`.

## A parameter set

`method`, `implementation`, `default`, `formula_units`, `description`, `reference`, `parameters`, `auxiliary`, `units`.

## A Method record

`kind`, `tier`, `operates_on`, `applies_to_classes`, `contributes_to`, `parameter_definitions`, `implementation_id`, `source`, `expression`, `generator`, `source_hash`, `tests`, `effects`, `citation`. See [Add a method as data](../how-to/add-a-method-as-data.md).
