---
title: 'ThermoFun: A C++/Python library for computing standard thermodynamic properties of substances and reactions across wide ranges of temperatures and pressures'
tags:
  - C++
  - Python
  - thermodynamics
  - standard state thermodynamic properties
  - equations of state
  - materials
authors:
  - name: George Dan Miron^[corresponding author]
    affiliation: 1
    orcid: 0000-0002-6923-7300 
  - name: Allan M. M. Leal
    affiliation: 2
  - name: S.V. Dmytrieva
    affiliation: 3
  - name: Dmitrii A. Kulik
    affiliation: 1
affiliations:
 - index: 1
   name: Laboratory for Waste Management LES, Paul Scherrer Institut, 5232 Villigen, Switzerland
 - index: 2
   name: Geothermal Energy and Geofluids Group, Institute of Geophysics, ETH Zurich, Switzerland
 - index: 3
   name: Cosylab Switzerland GmbH, Badenerstrasse 13, CH–5200 Brugg, Switzerland
date: "26 June 2022"
bibliography: paper.bib
---

# Summary

[ThermoFun](https://thermohub.org/thermofun) is an open source library that greatly facilitates the computation and retrieval of standard thermodynamic properties of chemical substances, compounds and reactions among them at temperature and pressure ranges of interest. This is done using many thermodynamic models with input standard properties and parameters from various thermodynamic datasets collected and curated in [ThermoHub](https://thermohub.org/thermohub) database. The library with its API is developed in C++ for performance reasons. A Python API is maintained for broader and more convenient usage. Both single and batch calculations are possible, as well as coupling ThermoFun with other C++ or Python modelling codes that require standard thermodynamic data as input, e.g., for chemical equilibria calculations, plotting phase diagram plotters, or model parameter fitting. Results can be written to file in tabular or other formats, or passed in memory via the C++ or Python API within a coupled code. For its calculations, ThermoFun can use different thermodynamic datasets that are read from local files (extendable) or are retrieved via remote access to ThermoHub online database. The library contains a comprehensive collection of models and equations of state for solid, aqueous, surface, gaseous, and melt substances, and their reactions, arranged in a modular and extendable way permitting new methods and equations of state to be implemented incrementally. Examples for using ThermoFun are available in its open source [code repository](https://github.com/thermohub/thermofun). This ensures a broad availability of ThermoFun and of the thermodynamic datasets from ThermoHub in Earth and material science and engineering.

# Benefits of using ThermoFun

* It facilitates and simplifies the computation of standard thermodynamic properties of substances and reactions at temperature and pressure of interest.
* It employs many equations of state and other thermodynamic models needed for calculating standard properties at different conditions; the user does not need to implement and test them separately in own codes or spreadsheets.
* It uses input standard properties and parameters from various thermodynamic datasets consistently kept in the (online) ThermoHub database, thus removing the burden of collecting all necessary standard thermodynamic data from vast literature or of writing complex scripts for importing these data from different formats.
* The available thermodynamic models in ThermoFun are implemented and tested based on their original publications; the thermodynamic data found in ThermoHub is curated and documented by experts, allowing the modeler to save time and effort for a more efficient use of geochemical code tools to address scientific problems.
* It can serve as a common source of thermodynamic models for standard properties of substances and reactions that can be easily integrated and combined, significantly improving the modeling capabilities for diverse (geo)chemical systems and over wide ranges of conditions.

# Statement of need

Thermodynamic modelling provides a valuable insight into the equilibrium speciation in chemical systems, also allowing for investigation of processes under conditions and time scales that cannot be reached in laboratory settings. Any such calculation requires as input the standard thermodynamic properties of all involved substances (species) or reactions at temperature (T) and pressure (P) of interest. The stability of a substance depends on its standard Gibbs energy function G$^{\circ}_{T}$ (of formation from chemical elements in their standard states), or on the equilibrium constant K$^{\circ}_{T}$ of reaction of its formation from other substances (master species reactants). In thermodynamic databases, usually only G$^{\circ}_{298}$ or K$^{\circ}_{298}$ values at reference temperature 298.15 K (25 $^{\circ}$C) can be found. Other standard properties – absolute entropy S$^{\circ}_{298}$ or enthalpy H$^{\circ}_{298}$ along with the heat capacity C$_{p,298}^{\circ}$ or its function of temperature T, C$_{p,T}^{\circ}$ = f(T) – are needed for G$^{\circ}_{T}$ or K$^{\circ}_{T}$ evaluation. The standard molar volume V$^{\circ}_{298}$ is the minimal data needed to account for the effect of pressure onto G$^{\circ}_{T}$. In chemical equilibrium problems where other conditions are imposed (e.g., internal energy and volume instead of temperature and pressure), other standard thermodynamic properties are additionally (e.g., standard internal energies and standard volumes of the species). Thus, modeling chemically reactive multiphase systems is only possible as long as standard thermodynamic properties of species or reactions can be computed [@Leal2017].

Equations of state that relate standard thermodynamic properties to temperature and pressure are used for their computation. We call these standard thermodynamic models. The quality of thermodynamic modelling results are strongly influenced by the predictive capability of the model and the availability, accuracy, precision and consistency of model input parameters available in thermodynamic datasets. Different models have been developed for the evaluation of standard thermodynamic properties of substances or reactions over a wide range of temperature and pressure conditions. These models can have complex mathematical formulations, a large number of input parameters, may require iterative numerical procedures, or a combination of several methods may be needed to calculate properties of a certain type of substance. For example, the properties of the solvent (usually water) are used to calculate the properties of the solutes (aqueous species). Specific implementations of extrapolation methods and equations of state can be found in existing thermodynamic modeling codes or as standalone codes that work with given thermodynamic datasets and file formats. Most of them are not directly compatible, which makes it difficult to retrieve the properties calculated with different methods and use them in different codes, or to investigate, compare and tabulate thermodynamic data coming from different sources. With many existing thermodynamic datasets, equations of state, modeling codes and file formats, it may be a difficult, error-prone and time consuming effort to collect such data and to find, implement and apply suitable models in calculations. This calls for efficient automation of these various models and data formats with integration into a consistent service.

The ThermoFun library as a service to the community that facilitates the computation of standard thermodynamic properties of substances and reactions \autoref{fig:properties} using an evolving, extensive collection of EoS thermodynamic models and input standard properties and parameters from different thermodynamic datasets retrievable from the ThermoHub online database server or provided as local files. The library is extendable with new models and has remote access to the curated collection of thermodynamic datasets on ThermoHub. These datasets contain reference thermodynamic data applicable in various studies on hydrothermal processes, cementitious materials, nuclear engineering, waste incineration, radioactive and chemical management and disposal, and other (geo)chemical environments. Fast and simple access to many thermodynamic datasets, methods and equations of state allows for more efficient use of these in modeling codes, or to optimize/fit model parameters, evaluate the models performance, range of applicability and accuracy. ThermoFun provides the flexibility of using thermodynamic datasets and appropriate models, or a combination of models that are best suited for different classes of substances necessary in various modeling applications.

![Example of standard thermodynamic properties of substances and reactions calculated using ThermoFun as function of temperature along the saturated water vapor pressure curve (input parameters from `aq17` thermodynamic dataset [@Miron2016;@Miron2017]).\label{fig:properties}](figure1.png)

As a source of standard thermodynamic properties of substances and/or reactions, the ThermoFun library can be linked to any C++ or Python geochemical equilibrium speciation, parameter optimization, or reactive mass-transport code. As a standalone code, ThermoFun can be used simply for searching and tabulating thermodynamic properties of substances and reactions. For example, ThermoFun has been employed to calculate the standard properties of reactants and reactions at elevated temperatures when evaluating the performance of isocoulombic reactions to extrapolate the properties of aqueous complexation reactions involving lanthanides and actinides to elevated temperatures [@Miron2020]. The library was also applied to evaluate the effect of using different equations of state for water-solvent on the calculated properties of dissolved aqueous species [@Miron2019].
ThermoFun is already adopted as a source of thermodynamic data in [GEMS](https://gems.web.psi.ch/) equilibrium speciation codes [@Kulik2013] and in [Reaktoro](https://reaktoro.org) chemical modeling framework [@Leal2016].

# Features

ThermoFun is initialized with a thermodynamic dataset, defined as a database consisting of collections of records for chemical elements, substances (species) and optionally reactions between substances, needed to calculate equilibria in chemical systems. These records contain various data like symbol, atomic mass, name, chemical formula, standard thermodynamic properties, equation of state (EoS) parameters, etc. for species; stoichiometry coefficients of involved species, standard thermodynamic effects, optionally model parameters coefficients, etc. for reactions. Dedicated research and development efforts are devoted to compile, critically select these properties and parameters from literature or to fit them against experimental data. Thermodynamic datasets used in ThermoFun can be read from local files or retrieved via remote access to ThermoHub online database (and saved locally). ThermoHub database contains a collection of [thermodynamic datasets](https://thermohub.org/thermohub/thermohub/#thermodatasets) that are widely used in various modeling applications.

The online ThermoHub database is being actively extended with existing, and newly updated and developed datasets prepared by thermodynamic database experts and thus automatically made available for calculations with ThermoFun. In general, the calculations have to be done using one internally consistent thermodynamic dataset, but it is also possible to use custom datasets, combine them, and append additional data from user-maintained files. Using the [ThermoHubClient](https://thermohub.org/thermohub/thermohubclient/) utility code, thermodynamic datasets can be retrieved into a simplified ThermoFun format and saved to text files that can be edited and extended.

The ThermoFun `ThermoEngine` class contains functions that return the standard state thermodynamic properties of a substance or a reaction at the given temperature and pressure. For water solvent, additional properties like water density, dielectric constant can be retrieved. A low-level fine-grained access up to individual methods is also possible.

The `ThermoBatch` class can be used to run batch calculations, for a list of substances, reactions, and temperature and pressure ranges. `ThermoBatch` can be also used for outputting the results in tabulated comma separated values (CSV) or other data formats.

Reaction properties can be calculated for any `reaction` record present in thermodynamic dataset; the extrapolation to given temperature and pressure is done using the model code and its input parameters available in the record. Another option for the user is to provide strings of reaction equations written using symbols of substances available in the thermodynamic dataset (e.g., Calcite $\leftrightharpoons$ Ca$^{2+}$ + CO$_{3}^{2-}$). In this case, the standard properties of reactions are calculated from properties of reactants, each first computed for T, P of interest using the models and parameters defined in their respective `substance` records. Yet another option is to set a reaction with known properties to define the properties of a given substance. In this case, the standard properties of the reaction-defined substance at T, P are retrieved from the standard properties of reaction calculated at T, P along with the standard properties of other reactants (substances) that are involved in the reaction. Thus, one thermodynamic dataset can contain a combination of substances, reaction dependent substances and reactions, whose consistent standard properties at T, P of interest are computed recursively.

# Basic examples

Loading the `aq17` thermodynamic dataset [@Miron2016;@Miron2017] and calculating standard properties of a substance (calcium aqueous ion $Ca^{2+}$). Output values are rounded for two decimal places.

```python
  import thermofun as fun
  database = fun.Database('aq17-thermofun.json')
  engine = fun.ThermoEngine(database)
  #                                        T(K)   P(Pa)  symbol
  Ca_ion = engine.thermoPropertiesSubstance(473, 2000e5, 'Ca+2')
  print(f'G0 {Ca_ion.gibbs_energy.val} J/mol')
```

  `G0 -545301.29 J/mol`

Calculating the properties of a reaction given as a reaction equation (calcite dissolution):

```python
  #                                    T(K)   P(Pa)  reaction equation
  R = engine.thermoPropertiesReaction(348.15, 1e5, 'Calcite = Ca+2 + CO3-2')
  print(f'drS0 of (Cal = Ca+2 + CO3-2) is {R.reaction_entropy.val}')
  print(f'drG0 of (Cal = Ca+2 + CO3-2) is {R.reaction_gibbs_energy.val}')
  print(f'logK0 of (Cal = Ca+2 + CO3-2) is {R.log_equilibrium_constant.val}')
```

```
drS0 of (Cal = Ca+2 + CO3-2) is -259.12
drG0 of (Cal = Ca+2 + CO3-2) is 59914.09
logK0 of (Cal = Ca+2 + CO3-2) is -8.99
```

Using the batch class to do sequential calculations and output the results to a CSV file:

```python
  batch = fun.ThermoBatch(database)
  batch.setPropertiesUnits(['temperature', 'pressure'],['degC','bar'])
  batch.setPressureIncrement(0,0,0)
  batch.setTemperatureIncrement(0,300, 5)
  substances = ['Na+', 'Mg+2', 'Ca+2', 'SiO2@']
  properties = ['heat_capacity_cp','entropy', 'volume']
  batch.thermoPropertiesSubstance(substances, properties).toCSV('results.csv')
```

Jupyter notebooks with examples on how to use `ThermoFun` can be found in the [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter) repository. More complex Python or C++ routines that use `ThermoFun` can be coded. For example, `ThermoFun` can be coupled to a phase diagram plotter, a chemical equilibrium solver, or a parameter optimization routine for fitting the standard properties and model parameters of substances and reactions and their temperature and pressure dependence.

# Acknowledgements

Support to G. D. Miron by the German Federal Ministry for Education and Research (BMBF), ThermAc project (02NUK039A) is gratefully acknowledged. D. A. Kulik and G. D. Miron are grateful for the financial support provided by NAGRA. A.M.M. Leal is grateful for the financial support of the Swiss National Science Foundation (SNSF) through the Ambizione grant PZ00P2-179967.

# References
