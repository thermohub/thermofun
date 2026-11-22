<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Glossary

Each word has one meaning in these pages.

| Word | Meaning |
|---|---|
| Base method | A method that gives all the properties of a record. A record has one. |
| Canonical convention | The convention that the engine calculates in: `benson_helgeson`, molality, 1 bar, and the anchor of water of the dataset. |
| Catalog | The list of all methods of the library, with their parameters. `tf.methods.catalog()` gives it. |
| Contributes to | What a method does to each property of a record: `sets` the whole value, `adds` an increment, or `uses` it as an input. The page of each method gives it. |
| Convention | How values are defined: energy reference, concentration scale, standard pressure. |
| Dataset | The information about a database: symbol, name, version, conventions. The key is `thermodataset`. |
| Defining reaction | The reaction that gives the properties of a substance. |
| Dielectric model | A method that gives the dielectric constant of water and the Born functions. |
| Engine | The object that calculates: `ThermoEngine`. |
| Equation of state of water | A method that gives the density of water and its properties as a substance. |
| Fluid | A gas or a supercritical fluid with an equation of state, such as CORK. |
| HKF | The Helgeson-Kirkham-Flowers equation of state of aqueous species. |
| Implementation | One code of a method. `reaktoro` and `gems` are the implementations of `WaterHGK84`. |
| Increment | A method that adds to what the base method gives: a phase transition, a volume, a term of the pressure. |
| Legacy format | The JSON format before version 1.0, with keys such as `sm_gibbs_energy` and `TPMethods`. |
| Method | A calculation of properties at T and P, with its parameters. A record lists its methods. |
| Model parameter | A value of a model, the same for every record (NEA TDB-4), for example HKF eta or the coefficients of IAPWS-95. It is in a file of `ThermoFun/Methods/ParameterSets`. A **parameter** is a value of one record; a **coefficient** multiplies a term; a **constant** is a physical constant. |
| Parameter | A value that a method reads from a record, such as `a1` of `HKF88`. It can be fitted. |
| Parameter set | A named group of values for a method. A record names it with `parameter_set`. |
| Property source | Where the properties of a record come from: its methods, its defining reaction, or its reactants. |
| Reactant | A substance in a reaction, with its coefficient. The coefficient is negative on the left side of the equation. |
| Record | One element, substance, reaction, method definition, parameter set or data source. |
| Reference values | The values of a record at its reference T and P. The key is `standard_state_properties`. |
| Segment | A T interval of a method, with its own parameters. |
| Solute | A substance dissolved in water. Its class is `aqueous_solute`. |
| Solvent | The water record that solutes read. `engine.solventSymbol()` gives its symbol. |
| Status | How a value was made: `calculated`, `read`, `assigned` or `notdefined`. |
| Symbol | The name of a record in a database, such as `Ca+2`. |
| Symbolic method | A method that is an expression in a Method record, with no C++ code. |
| v2 format | The format since version 1.0, in YAML or JSON, with the key `thermohub`. |
| Variant | A parameter set with `default: false`. A record uses it only if it names it. |
| Water anchor | The data of liquid water that tie an equation of state of water to the values of formation: `helgeson_kirkham_1974` or `codata_1989`. Data of the dataset, not a convention: never converted. |
