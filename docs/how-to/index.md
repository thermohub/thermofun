<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# How-to guides

Steps for one job. The guides are in four groups: using ThermoFun to calculate, working with data, fitting parameters to measured data, and the technical guides for installing, upgrading and adding models.

## Calculate

- [Run a batch calculation](make-tables.md): many records at many conditions, from a YAML setup or a batch template, in C++ and Python.
- [Use ThermoBatch, the 0.x batch API](use-thermobatch.md)
- [Choose water models](choose-water-models.md)
- [Choose conventions](choose-conventions.md)
- [Propagate uncertainties](propagate-uncertainties.md)
- [See how a result is calculated](see-how-a-result-is-calculated.md)
- [Use a method alone to calculate](use-a-method-alone.md)

## Data

- [Add data](add-data.md): substances, reactions, methods, units and datasets.
- [Validate data](validate-data.md)
- [Convert a legacy database](convert-legacy-database.md)

## Fitting

- [Fit parameters to measured data](fit-parameters.md) (under development): name a parameter, its derivatives, a fit, constraints and a searched parameter, a fit from a template.

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
use-thermobatch
choose-water-models
choose-conventions
propagate-uncertainties
see-how-a-result-is-calculated
use-a-method-alone
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
:caption: Fitting
:maxdepth: 1
:hidden:

fit-parameters
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
