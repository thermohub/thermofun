# Constants of the methods

Each file holds the constants of one model, written like the `parameter_sets` section of a database. A file is named `Method.yaml` or `Method-variant.yaml`. The build puts the files in the library, and a file is read when a method object first needs it. They are also installed in `include/ThermoFun/Methods/ParameterSets`, to read; the library does not read that folder. A record does not give these values.

Each set has:

- `method`, and `implementation` when the implementations of the method have different values;
- `default: false` for a variant. Only a record that names it with `parameter_set` uses it. One set per method and implementation is the default;
- `formula_units`: the units of the formulas the values are in. It is text for the reader: the values are used as they are written;
- `description`: what the values are, the formula, and the units. It appears on the page of the method in `docs/reference/methods`;
- `reference`: where the values come from;
- `parameters`: the values. `name_N` is place N of an array of the code.

Values are in the units of the formulas, as written in the paper, and are not converted when they are read. The units differ between models:

| File | T | P | Density | Other |
|---|---|---|---|---|
| AkinfievDiamond03 | | | | M in g/mol |
| BradleyPitzer79 | K | bar | | |
| DataPressure | | | kg/m3 | |
| ChurakovGottschalk03 | K | | reduced | the constants are for the dimensionless beta = epsilon/T and reduced density sigma^3 NA rho, with sigma and epsilon of each substance |
| CorkHP91 | K | kbar | | V in kJ/kbar |
| FernandezEtAl97 | K | | kg/m3 | SI |
| HelgesonKirkham74 | K | | g/cm3 | |
| HKF88 | K | bar | g/cm3 | energy in cal/mol |
| JohnsonNorton91 | K | | kg/m3 | |
| PengRobinson78, PRSV86, SoaveRedlichKwong72 | | | | numbers without units |
| PhysicalConstants | | | | SI |
| RyzhenkoBryzgalin | K | | g/cm3 | |
| SternerPitzer94 | K | MPa | kg/m3 | M in g/mol |
| SverjenskyEtAl14 | degC | | g/cm3 | |
| WaterHGK84, WaterIAPWS95 | K | | kg/m3 | |
| WaterHGK84-gems | K or degC | bar or MPa | | as in the code, by name: `visc_`, `cond_`, `born_` limits in degC and bar |
| WaterSaturation | K | Pa | kg/m3 | M in kg/mol |
| WaterTriplePoint | | | | energy in cal/mol, S in cal/(mol K) |
| WaterZhangDuan05 | K | Pa | | V in cm3/mol; `g1kb`: T in degC, G in cal/mol |

A data file replaces a value like a parameter: inline on the method of the record, or with a `parameter_set` of the same method. The description says so when a record cannot replace a value. To add a variant with no rebuild, put its file in a dataset's `methods/parameter_sets` folder, or in a folder named by the environment variable `THERMOFUN_PARAMETER_PATH`. See `docs/how-to/add-data.md`, Section "Built-in constants".
