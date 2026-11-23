<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Reference

Facts and the reasons behind them, in six sections. The pages of the methods, the diagnostics and the options are generated from the code.

## Methods

- [List of methods](methods/index.md): one page for each method, with its source, parameters and an example.
- [Parameters of all methods](method-parameters.md), with their units.
- [How methods combine](how-methods-combine.md): the base method and the increments of a record.

## Options

- [Options of the engine](engine-preferences.md) and its conventions.
- [Conventions](conventions.md): what the zero of an energy is, and the standard states.
- [Batch setups](batch.md)

## Formats

- [The data model](data-model.md): the records.
- [The v2 format](format-v2.md) and its [JSON Schema](format-v2-schema.md).
- [Allowed words](vocabulary.md) of `class`, `aggregate_state` and the other fields with a list.
- [The legacy format](legacy-format.md), and [how it differs from v2](legacy-and-v2.md).

## Diagnostics

- [Diagnostics](diagnostics.md): each code of a problem and how to correct it.

## Results and units

- [Results](results.md): what a calculation gives.
- [Units](units.md)
- [Derivatives](derivatives.md)
- [Uncertainties and rounding](uncertainty.md)

## API

- [Python API](python-api.md)
- [C++ API](https://docs.hdoc.io/dmiron/thermofun/search.html)

```{toctree}
:caption: Methods
:hidden:
:maxdepth: 1

methods/index
method-parameters
how-methods-combine
```

```{toctree}
:caption: Options
:hidden:
:maxdepth: 1

engine-preferences
conventions
batch
```

```{toctree}
:caption: Formats
:hidden:
:maxdepth: 1

data-model
format-v2
format-v2-schema
vocabulary
legacy-format
legacy-and-v2
```

```{toctree}
:caption: Diagnostics
:hidden:
:maxdepth: 1

diagnostics
```

```{toctree}
:caption: Results and units
:hidden:
:maxdepth: 1

results
units
derivatives
uncertainty
```

```{toctree}
:caption: API
:hidden:
:maxdepth: 1

python-api
```
