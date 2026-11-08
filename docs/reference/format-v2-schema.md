# The JSON Schema of the v2 format

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`.

The schema is `ThermoFun/IO/V2/thermohub.schema.json` (JSON Schema 2020-12), also installed with the headers. A file that is valid there is read without a warning. A key that starts with `_` is allowed and ignored. [The v2 format](format-v2.md) and [Add data](../how-to/add-data.md) explain the fields.

## document

| Key | Type | Required | Note |
|---|---|---|---|
| `thermohub` | string | yes | The version of the format. |
| `generator` | string |  |  |
| `thermodataset` | [thermodataset](#thermodataset) |  |  |
| `units` | [units](#units) |  |  |
| `include` | string or list of string |  |  |
| `elements` | mapping to [element](#element) or list of [element](#element) |  |  |
| `substances` | mapping to [substance](#substance) or list of [substance](#substance) |  |  |
| `reactions` | mapping to [reaction](#reaction) or list of [reaction](#reaction) |  |  |
| `methods` | mapping to [method_definition](#method_definition) or list of [method_definition](#method_definition) |  |  |
| `parameter_sets` | mapping to [parameter_set](#parameter_set) or list of [parameter_set](#parameter_set) |  |  |
| `data_sources` | mapping to [data_source](#data_source) or list of [data_source](#data_source) |  |  |

## single_record

A file of one record: its record_type, and the record without thermohub.

One of: record_type `element` and [element](#element); record_type `substance` and [substance](#substance); record_type `reaction` and [reaction](#reaction); record_type `method` and [method_definition](#method_definition); record_type `parameter_set` and [parameter_set](#parameter_set); record_type `data_source` and [data_source](#data_source)

## thermodataset

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `name` | string |  |  |
| `version` | string |  |  |
| `status` | string |  |  |
| `reference_conditions` | [reference_conditions](#reference_conditions) |  |  |
| `sourcetdb` | string or list of string |  |  |
| `solvent` | string |  |  |
| `data_source` | string |  |  |
| `defaults` | object |  |  |
| `conventions` | object |  |  |
| `convention` | [convention](#convention) |  |  |

## element

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `name` | string |  |  |
| `class` | string |  |  |
| `number` | integer |  |  |
| `isotope_mass` | number |  |  |
| `valence` | integer |  |  |
| `molar_mass` | [quantity](#quantity) |  |  |
| `standard_entropy` | [quantity](#quantity) |  |  |
| `heat_capacity` | [quantity](#quantity) |  |  |
| `volume` | [quantity](#quantity) |  |  |
| `reference_form` | string |  |  |
| `data_sources` | list of string |  |  |
| `sourcetdb` | string or list of string |  |  |
| `structural` | object |  |  |
| `units` | [units](#units) |  |  |
| `legacy_class` | string |  |  |

## substance

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `name` | string |  |  |
| `formula` | string | yes |  |
| `charge` | integer |  |  |
| `class` | string |  |  |
| `aggregate_state` | string |  |  |
| `sourcetdb` | string or list of string |  |  |
| `molar_mass` | [quantity](#quantity) |  |  |
| `convention` | [convention](#convention) |  |  |
| `reference_conditions` | [reference_conditions](#reference_conditions) |  |  |
| `standard_state_properties` | object |  |  |
| `methods` | [methods_of_a_record](#methods_of_a_record) |  |  |
| `defining_reaction` | string |  |  |
| `preferred_property_source` | string |  |  |
| `applicability` | [applicability](#applicability) |  |  |
| `data_source` | string |  |  |
| `data_sources` | list of string |  |  |
| `structural` | object |  |  |
| `deprecated` | boolean |  |  |
| `deprecated_in_favour_of` | string |  |  |
| `units` | [units](#units) |  |  |
| `covariances` | [covariances](#covariances) |  |  |

## reaction

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `name` | string |  |  |
| `equation` | string |  |  |
| `stoichiometry` | mapping to number |  |  |
| `reference_conditions` | [reference_conditions](#reference_conditions) |  |  |
| `reaction_properties` | object |  |  |
| `methods` | [methods_of_a_record](#methods_of_a_record) |  |  |
| `level` | integer |  |  |
| `data_source` | string |  |  |
| `data_sources` | list of string |  |  |
| `applicability` | [applicability](#applicability) |  |  |
| `convention` | [convention](#convention) |  |  |
| `preferred_property_source` | string |  |  |
| `sourcetdb` | string or list of string |  |  |
| `deprecated` | boolean |  |  |
| `units` | [units](#units) |  |  |
| `covariances` | [covariances](#covariances) |  |  |

## method_application

| Key | Type | Required | Note |
|---|---|---|---|
| `method` | string | yes |  |
| `parameters` | [parameters](#parameters) |  |  |
| `parameter_set` | string |  |  |
| `segments` | list of object |  |  |
| `transitions` | list of object |  |  |
| `applicability` | [applicability](#applicability) |  |  |
| `table` | object |  |  |
| `units` | [units](#units) |  |  |
| `implementation` | string |  |  |
| `interpolation` | string |  |  |
| `order` | integer |  |  |

## quantity

A value: a number in the default unit of its field, a text "value unit", or an object.

One of: number; string; an object with `value`, `unit`, `uncertainty`, `quality`, `origin`, `calculation_path`, `source`

## uncertainty

A standard uncertainty (a number), or an object: standard; expanded with coverage_factor; or half_width with distribution uniform (u = a/sqrt(3)) or triangular (u = a/sqrt(6)).

One of: number; an object with `standard`, `distribution`, `kind`; an object with `expanded`, `coverage_factor`, `distribution`, `kind`; an object with `half_width`, `distribution`, `kind`

## convention

| Key | Type | Required | Note |
|---|---|---|---|
| `energy_reference` | `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K` |  |  |
| `standard_pressure` | `1bar`, `1atm` |  |  |
| `concentration_scale` | `molality`, `mole_fraction`, `molarity` |  |  |
| `water_reference` | `geochemical`, `steam_tables` |  |  |

## reference_conditions

| Key | Type | Required | Note |
|---|---|---|---|
| `T` | [quantity](#quantity) |  |  |
| `P` | [quantity](#quantity) |  |  |

## applicability

| Key | Type | Required | Note |
|---|---|---|---|
| `t_min` | [quantity](#quantity) |  |  |
| `t_max` | [quantity](#quantity) |  |  |
| `p_min` | [quantity](#quantity) |  |  |
| `p_max` | [quantity](#quantity) |  |  |

## units

Default units of everything inside the level where it is.

| Key | Type | Required | Note |
|---|---|---|---|
| `energy` | string |  |  |
| `pressure` | string |  |  |
| `temperature` | string |  |  |
| `length` | string |  |  |
| `mass` | string |  |  |
| `amount` | string |  |  |
| `volume` | string |  |  |
| `entropy` | string |  |  |
| `heat_capacity` | string |  |  |

## covariances

Type: list of object

## parameters

Parameters of a method, by name.

Type: mapping to [quantity](#quantity) or array

## method_definition

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `kind` | string |  |  |
| `tier` | integer |  |  |
| `operates_on` | string |  |  |
| `applies_to_classes` | list of string |  |  |
| `contributes_to` | list of string |  |  |
| `parameter_definitions` | object or array |  |  |
| `implementation_id` | string |  |  |
| `source` | string or object |  |  |
| `expression` | string or object |  |  |
| `generator` | string |  |  |
| `source_hash` | string |  |  |
| `tests` | array |  |  |
| `effects` | object |  |  |
| `citation` | string |  |  |

## parameter_set

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `method` | string | yes |  |
| `implementation` | string |  |  |
| `default` | boolean |  |  |
| `formula_units` | object |  |  |
| `description` | string |  |  |
| `parameters` | [parameters](#parameters) | yes |  |
| `auxiliary` | object |  |  |
| `reference` | string |  |  |
| `units` | [units](#units) |  |  |

## data_source

| Key | Type | Required | Note |
|---|---|---|---|
| `symbol` | string |  |  |
| `record_type` | string |  |  |
| `title` | string |  |  |
| `authors` | string or list of string |  |  |
| `year` | integer |  |  |
| `journal` | string |  |  |
| `doi` | string |  |  |
| `bibtex` | string |  |  |

## methods_of_a_record

Type: list of [method_application](#method_application)
