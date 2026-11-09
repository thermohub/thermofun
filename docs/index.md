<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# ThermoFun

ThermoFun calculates the standard-state thermodynamic properties of substances and reactions, and the properties of water, at a temperature T and a pressure P. It reads them from a database of records. Each result has its derivatives by T and P, an uncertainty and a status.

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
calcium = engine.thermoPropertiesSubstance(473.15, 2e8, "Ca+2")   # T in K, P in Pa
assert calcium.gibbs_energy.val < 0.0                             # J/mol
```

Install it with `conda install -c conda-forge thermofun`, or [install it from the source](how-to/install-from-source.md).

Databases are in `Resources/databases/` of the repository. The words of these pages are in the [glossary](glossary.md).

## Tutorials: learn by doing

1. [Your first calculation](tutorials/01-first-calculation.md): a substance, a reaction and water, and what a result holds.
2. [Load data](tutorials/02-load-data.md): read, choose, check and write records.
3. [Change the models](tutorials/03-change-models.md): methods, how they combine, implementations, options and conventions.
4. [Where the files are](tutorials/04-where-the-files-are.md)
5. [Templates](tutorials/05-templates.md)
6. [Output tables](tutorials/06-output-tables.md)
7. [Batch calculations](tutorials/07-batch-calculations.md)
8. [Fit parameters](tutorials/08-fit-parameters.md)

Each tutorial is a notebook. It runs each time the site is built, so its results come from the current code. To run one in Jupyter, download it from its page as `.ipynb`, or use the copies in [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter), which also has workshops and longer examples with other codes.

## How-to: steps for one job

- [Add data](how-to/add-data.md): the complete guide to substances, reactions, methods, units and datasets.
- [Make tables](how-to/make-tables.md): from a YAML setup or a template, in C++ and Python.
- [Convert a legacy database](how-to/convert-legacy-database.md)
- [Validate data](how-to/validate-data.md)
- [Choose water models](how-to/choose-water-models.md)
- [Choose conventions](how-to/choose-conventions.md)
- [Propagate uncertainties](how-to/propagate-uncertainties.md)
- [Fit parameters to measured data](how-to/fit-parameters.md), with derivatives by parameters and parameter handles.
- [Use a method alone](how-to/use-a-method-alone.md)
- [See what each method gives](how-to/get-per-method-results.md)
- [Migrate from the old API](how-to/migrate-from-the-old-api.md)
- [Add a method as data](how-to/add-a-method-as-data.md)
- [Add a method in C++](how-to/add-a-cpp-method.md)
- [Install from the source](how-to/install-from-source.md)

## Reference: facts

- [Methods](reference/methods/index.md), one page for each method. Generated from the code.
- [Diagnostics](reference/diagnostics.md): each code and how to correct it. Generated from the code.
- [Options of the engine](reference/engine-preferences.md). Generated from the code.
- [Results](reference/results.md): what a calculation gives.
- [Batch options](reference/batch.md)
- [The v2 format](reference/format-v2.md)
- [The JSON Schema of the v2 format](reference/format-v2-schema.md)
- [Units](reference/units.md)
- [The legacy format](reference/legacy-format.md)
- [Python API](reference/python-api.md)
- [C++ API](https://docs.hdoc.io/dmiron/thermofun/search.html)

## Explanation: background and reasons

- [The data model](explanation/data-model.md)
- [How methods combine](explanation/how-methods-combine.md)
- [Uncertainties and rounding](explanation/uncertainty.md)
- [Conventions](explanation/conventions.md)
- [Two formats](explanation/legacy-and-v2.md)
- [Derivatives with autodiff](explanation/autodiff.md)

## Contributing

- [Golden tests](contributing/golden-tests.md)
- [Writing rules](contributing/writing-rules.md)

```{toctree}
:hidden:

tutorials/index
how-to/index
reference/index
explanation/index
contributing/index
```
