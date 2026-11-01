# Constants of the methods

Each file holds the constants of one model, written like the `parameter_sets` section of a database. The build puts the files in the library, so nothing is read from this folder when the library runs. A record does not give these values.

Each set has:

- `method`, and `implementation` when the implementations of the method have different values;
- `description`: what the values are, the formula, and the units. It appears on the page of the method in `docs/reference/methods`;
- `reference`: where the values come from;
- `parameters`: the values. `name_N` is place N of an array of the code.

Values are in the units of the formulas, as written in the paper, and are not converted when they are read.

A data file replaces a value like a parameter: inline on the method of the record, or with a `parameter_set` of the same method. The description says so when a record cannot replace a value. See `docs/how-to/add-data.md`, Section "Built-in constants".
