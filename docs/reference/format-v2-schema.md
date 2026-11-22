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
| `data_sources` | string or list of string |  | NEA reference codes (TDB-5 1.11) of the sources, or free text; a text is a list of one. The first is the source of a value that gives none. |
| `defaults` | object |  |  |
| `standard_state` | [standard_state](#standard_state) |  |  |
| `energy_reference` | [energy_reference](#energy_reference) |  |  |

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
| `reference_state` | string |  | The reference state of the element at the reference temperature and the standard pressure (NEA TDB-5, Table 7): formula unit and state, for example O2(g), C(cr, graphite), Fe(cr, bcc). |
| `data_sources` | string or list of string |  | NEA reference codes (TDB-5 1.11) of the sources, or free text; a text is a list of one. The first is the source of a value that gives none. |
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
| `standard_state` | [standard_state](#standard_state) |  |  |
| `energy_reference` | [energy_reference](#energy_reference) |  |  |
| `reference_conditions` | [reference_conditions](#reference_conditions) |  |  |
| `standard_state_properties` | object |  |  |
| `methods` | [methods_of_a_record](#methods_of_a_record) |  |  |
| `defining_reaction` | string |  |  |
| `preferred_property_source` | string |  |  |
| `applicability` | [applicability](#applicability) |  |  |
| `data_sources` | string or list of string |  | NEA reference codes (TDB-5 1.11) of the sources, or free text; a text is a list of one. The first is the source of a value that gives none. |
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
| `data_sources` | string or list of string |  | NEA reference codes (TDB-5 1.11) of the sources, or free text; a text is a list of one. The first is the source of a value that gives none. |
| `applicability` | [applicability](#applicability) |  |  |
| `standard_state` | [standard_state](#standard_state) |  |  |
| `energy_reference` | [energy_reference](#energy_reference) |  |  |
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
| `region` | [region](#region) |  |  |
| `table` | object |  |  |
| `units` | [units](#units) |  |  |
| `implementation` | string |  |  |
| `interpolation` | string |  |  |
| `interpolation_order` | integer |  |  |

## quantity

A value: a number in the default unit of its field, a text "value unit", or an object.

One of: number; string; an object with `value`, `unit`, `uncertainty`, `quality`, `origin`, `derivation`, `source`

## uncertainty

A standard uncertainty (a number), or an object: standard; expanded with coverage_factor; or half_width with distribution uniform (u = a/sqrt(3)) or triangular (u = a/sqrt(6)).

One of: number; an object with `standard`, `distribution`, `kind`; an object with `expanded`, `coverage_factor`, `distribution`, `kind`; an object with `half_width`, `distribution`, `kind`

## standard_state

The standard state of the values: the standard pressure p° and, for solutes, the concentration scale.

| Key | Type | Required | Note |
|---|---|---|---|
| `pressure` | `1bar`, `1atm` or [quantity](#quantity) |  | p°: 1bar, 1atm, or any pressure (a number in Pa, or with a unit) |
| `concentration_scale` | `molality`, `mole_fraction`, `molarity`, `amount_fraction`, `amount_concentration` |  |  |

## energy_reference

From what zero G, H and S are counted: of the elements, of water and of ions.

| Key | Type | Required | Note |
|---|---|---|---|
| `elements` | `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K` |  |  |
| `water` | `helgeson_kirkham_1974`, `codata_1989`, `steam_tables`, `geochemical` |  | The water anchor, data of water used inside the calculation (on the dataset or the water record): helgeson_kirkham_1974 (the default; G of liquid water at 25 degC -237181 J/mol), codata_1989 (-237140 J/mol); or steam_tables, the scale of the values of a water record. geochemical is the 0.x word of helgeson_kirkham_1974. |
| `ions` | `hydrogen_ion`, `none` |  | hydrogen_ion (the default): H+(aq) is 0 at every T and P, and the electron is from the standard hydrogen electrode, G(e-) = 1/2 G(H2, g, p°) - G(H+) (NEA TDB-5 1.6, 1.7.e); none: the records give H+ and e-. |

## reference_conditions

The reference temperature Tr. The reference pressure is the standard pressure p° of standard_state.

| Key | Type | Required | Note |
|---|---|---|---|
| `T` | [quantity](#quantity) |  |  |

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
| `operates_on` | string |  |  |
| `applies_to_classes` | list of string |  |  |
| `contributes_to` | list of string |  |  |
| `parameter_definitions` | object or array |  |  |
| `implementation_id` | string |  |  |
| `formula` | string or object |  | The formula of a symbolic method as the author writes it: form and expression. |
| `expression` | string or object |  |  |
| `generator` | string |  |  |
| `formula_hash` | string |  |  |
| `tests` | array |  |  |
| `effects` | object |  |  |
| `data_sources` | string or list of string |  |  |

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
| `data_sources` | string or list of string |  |  |
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

## region

| Key | Type | Required | Note |
|---|---|---|---|
| `t_min` | [quantity](#quantity) |  |  |
| `t_max` | [quantity](#quantity) |  |  |
| `p_min` | [quantity](#quantity) |  |  |
| `p_max` | [quantity](#quantity) |  |  |
