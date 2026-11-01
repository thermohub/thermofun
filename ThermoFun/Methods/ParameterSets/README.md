# Constants of the methods

Each file holds the constants of one model, written like the `parameter_sets` section of a database. A file is named `Method.yaml` or `Method-variant.yaml`. The build puts the files in the library, and a file is read when a method object first needs it. They are also installed in `include/ThermoFun/Methods/ParameterSets`, to read; the library does not read that folder. A record does not give these values.

Each set has:

- `method`, and `implementation` when the implementations of the method have different values;
- `default: false` for a variant. Only a record that names it with `parameter_set` uses it. One set per method and implementation is the default;
- `description`: what the values are, the formula, and the units. It appears on the page of the method in `docs/reference/methods`;
- `reference`: where the values come from;
- `parameters`: the values. `name_N` is place N of an array of the code.

Values are in the units of the formulas, as written in the paper, and are not converted when they are read.

A data file replaces a value like a parameter: inline on the method of the record, or with a `parameter_set` of the same method. The description says so when a record cannot replace a value. To add a variant with no rebuild, put its file in a dataset's `methods/parameter_sets` folder, or in a folder named by the environment variable `THERMOFUN_PARAMETER_PATH`. See `docs/how-to/add-data.md`, Section "Built-in constants".
