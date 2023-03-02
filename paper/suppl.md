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

The stability of a substance depends on its standard Gibbs energy function G$^{\circ}{T}$ (which represents the formation of the substance from chemical elements in their standard states), or on the equilibrium constant K$^{\circ}{T}$ of the reaction that forms it from other substances (master species reactants). In thermodynamic databases, standard Gibbs energy and equilibrium constant values are often only available at reference temperature 298.15 K (25 $^{\circ}$C). Other standard properties such as absolute entropy S$^{\circ}{298}$, enthalpy H$^{\circ}{298}$, heat capacity C${p,298}^{\circ}$, and the temperature-dependent function C${p,T}^{\circ}$ = f(T) are needed to evaluate G$^{\circ}{T}$ or K$^{\circ}{T}$. The standard molar volume V$^{\circ}{298}$ is the minimum data required to account for the effect of pressure on G$^{\circ}{T}$. In chemical equilibrium problems where other conditions are imposed (e.g., internal energy and volume instead of temperature and pressure), additional standard thermodynamic properties may be required (e.g., standard internal energies and standard volumes of the species). Therefore, modeling chemically reactive multiphase systems is only possible if standard thermodynamic properties of species or reactions can be computed [@Leal2017].
These properties are computed using equations of state that relate standard thermodynamic properties to temperature and pressure. Different models have been developed to evaluate standard thermodynamic properties of substances or reactions over a wide range of temperature and pressure conditions. These models may have complex mathematical formulations, a large number of input parameters, and may require iterative numerical procedures, or a combination of several methods to calculate the properties of a particular type of substance.The quality of thermodynamic modeling results is strongly influenced by the predictive capability of the model and the availability, accuracy, precision, and consistency of the input parameters in the thermodynamic datasets.
ThermoFun facilitates calculation of standard thermodynamic properties of substances and reactions at a range of temperatures and pressures using a variety of equations of state and other thermodynamic models with input properties and parameters from various thermodynamic datasets that are consistently kept in the online ThermoHub database

# Features

ThermoFun is initialized with a thermodynamic dataset, which is a database consisting of collections of records for chemical elements, substances (species), and optionally reactions between substances, needed to calculate equilibria in chemical systems. These records contain various data such as symbol, atomic mass, name, chemical formula, standard thermodynamic properties, equation of state (EoS) parameters, etc. for species; and stoichiometry coefficients of involved species, standard thermodynamic effects, and optionally model parameters coefficients, etc. for reactions. Dedicated research and development efforts are devoted to compiling and critically selecting these properties and parameters from literature or fitting them to experimental data. Thermodynamic datasets used in ThermoFun can be read from local files or retrieved via remote access to the ThermoHub online database (and saved locally). The ThermoHub database contains a collection of widely used thermodynamic of [thermodynamic datasets](https://thermohub.org/thermohub/thermohub/#thermodatasets) that are used in various modeling applications.

The online ThermoHub database is actively being extended with existing and newly updated and developed datasets prepared by thermodynamic database experts, which are automatically made available for calculations with ThermoFun. In general, calculations must be done using one internally consistent thermodynamic dataset, but it is also possible to use custom datasets, combine them, and append additional data from user-maintained files. Using the [ThermoHubClient](https://thermohub.org/thermohub/thermohubclient/) utility code, thermodynamic datasets can be retrieved into a simplified ThermoFun format and saved to text files that can be edited and extended.

The ThermoFun `ThermoEngine` class contains functions that return the standard state thermodynamic properties of a substance or a reaction at a given temperature and pressure. For water solvent, additional properties such as water density and dielectric constant can also be retrieved. There is also the option for low-level fine-grained access up to individual methods.

The `ThermoBatch` class can be used to run batch calculations for a list of substances, reactions, and temperature and pressure ranges. `ThermoBatch` can also be used to output the results in tabulated comma-separated values (CSV) or other data formats.

Reaction properties can be calculated for any `reaction` record present in the thermodynamic dataset, with the extrapolation to the given temperature and pressure done using the model code and its input parameters available in the record. Alternatively, the user can provide strings of reaction equations written using symbols of substances available in the thermodynamic dataset (e.g., Calcite $\leftrightharpoons$ Ca$^{2+}$ + CO$_{3}^{2-}$). In this case, the standard properties of reactions are calculated from the properties of reactants, each of which is first computed for the T, P of interest using the models and parameters defined in their respective `substance` records. Another option is to set a reaction with known properties to define the properties of a given substance. In this case, the standard properties of the reaction-defined substance at T, P are retrieved from the standard properties of the reaction calculated at T, P, along with the standard properties of the other reactants (substances) that are involved in the reaction. Thus, one thermodynamic dataset can contain a combination of substances, reaction-dependent substances, and reactions, whose consistent standard properties at T, P of interest are computed recursively.

ThermoFun takes advantage of automatic differentiation for efficient and convenient calculation of derivatives. This feature allows ThermoFun to produce not only the standard thermodynamic properties of species and reactions, but also their temperature and pressure derivatives. This is necessary, for example, when chemical equilibrium problems where temperature and/or pressure are unknown, and Newton method is used to calculate these variables together with the amounts of species in equilibrium, as is done in Reaktoro [@Leal:2015]. Without temperature and pressure derivatives of standard Gibbs energies, the algorithm would not be fast and robust for these specific calculations. We plan to use the `autodiff` C++ library [@Leal:2018] in ThermoFun for even more capable autodiff capabilities, as it has been used in both Reaktoro [@Leal:2015] and TEQP [@Bell:Deiters:Leal:2022]. 

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

# References
