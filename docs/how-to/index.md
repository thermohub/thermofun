<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# How-to guides

Steps for one job. The guides are in three groups: using ThermoFun to calculate, working with data, and the technical guides for installing, upgrading and adding models.

## Calculate

- [Make tables](make-tables.md): from a YAML setup or a template, in C++ and Python.
- [Choose water models](choose-water-models.md)
- [Choose conventions](choose-conventions.md)
- [Propagate uncertainties](propagate-uncertainties.md)
- [See what each method gives](get-per-method-results.md)
- [Use a method alone](use-a-method-alone.md)
- [Fit parameters to measured data](fit-parameters.md) (under development)

## Data

- [Add data](add-data.md): substances, reactions, methods, units and datasets.
- [Validate data](validate-data.md)
- [Convert a legacy database](convert-legacy-database.md)

## Technical guides

- [Install from the source](install-from-source.md)
- [Migrate from the old API](migrate-from-the-old-api.md)
- [Add a method as data](add-a-method-as-data.md) (for model authors)
- [Add a method in C++](add-a-cpp-method.md) (for model authors)

```{toctree}
:caption: Calculate
:maxdepth: 1
:hidden:

make-tables
choose-water-models
choose-conventions
propagate-uncertainties
get-per-method-results
use-a-method-alone
fit-parameters
```

```{toctree}
:caption: Data
:maxdepth: 1
:hidden:

add-data
validate-data
convert-legacy-database
```

```{toctree}
:caption: Technical guides
:maxdepth: 1
:hidden:

install-from-source
migrate-from-the-old-api
add-a-method-as-data
add-a-cpp-method
```
