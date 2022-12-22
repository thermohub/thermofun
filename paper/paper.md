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

[ThermoFun](https://thermohub.org/thermofun) is an open source library that simplifies the calculation and retrieval of standard thermodynamic properties of chemical substances, compounds, and reactions among them at a range of temperatures and pressures. The library is developed in C++ for performance, but it also has a Python API for broader and more convenient usage. It employs a variety of thermodynamic models and equations of state for solid, aqueous, surface, gaseous, and molten substances, and their reactions with input properties and parameters from various thermodynamic datasets that are collected and curated in the in [ThermoHub](https://thermohub.org/thermohub) database. The library can be used as a standalone code for searching and tabulating thermodynamic properties or linked to other modeling codes that require standard thermodynamic data as input. It offers the flexibility to use different thermodynamic datasets, including custom datasets and datasets retrieved from the online ThermoHub database, and to choose the most suitable models for different classes of substances necessary in various modeling applications. It can serve as a common source of thermodynamic models for standard properties of substances and reactions that can be easily integrated and combined, significantly improving the modeling capabilities for diverse (geo)chemical systems and over wide ranges of conditions.

# Benefits of using ThermoFun

* It simplifies the calculation of standard thermodynamic properties of substances and reactions at a range of temperatures and pressures.
* It includes a variety of equations of state and other thermodynamic models that can be used to calculate standard properties under different conditions, eliminating the need for the user to implement and test these models separately in their own code or spreadsheets.
* It uses input properties and parameters from various thermodynamic datasets that are consistently kept in the online ThermoHub database, reducing the need to collect all necessary thermodynamic data from literature or to write complex scripts to import the data from different formats.
* The thermodynamic models in ThermoFun are implemented and tested based on their original publications, and the thermodynamic data in ThermoHub is curated and documented by experts, saving time and effort and enabling more efficient use of geochemical code tools for scientific problems.
* It serves as a common source of thermodynamic models for standard properties of substances and reactions that can be easily integrated and combined, significantly improving modeling capabilities for a wide range of (geo)chemical systems and conditions.

# Statement of need

Thermodynamic modeling provides valuable insight into the equilibrium speciation in chemical systems, and allows for the investigation of processes under conditions and time scales that cannot be reached in laboratory settings. In order to perform these calculations, the standard thermodynamic properties of all involved substances (species) or reactions at a specific temperature (T) and pressure (P) are required as input. The stability of a substance depends on its standard Gibbs energy function G$^{\circ}{T}$ (which represents the formation of the substance from chemical elements in their standard states), or on the equilibrium constant K$^{\circ}{T}$ of the reaction that forms it from other substances (master species reactants). In thermodynamic databases, standard Gibbs energy and equilibrium constant values are often only available at reference temperature 298.15 K (25 $^{\circ}$C). Other standard properties such as absolute entropy S$^{\circ}{298}$, enthalpy H$^{\circ}{298}$, heat capacity C${p,298}^{\circ}$, and the temperature-dependent function C${p,T}^{\circ}$ = f(T) are needed to evaluate G$^{\circ}{T}$ or K$^{\circ}{T}$. The standard molar volume V$^{\circ}{298}$ is the minimum data required to account for the effect of pressure on G$^{\circ}{T}$. In chemical equilibrium problems where other conditions are imposed (e.g., internal energy and volume instead of temperature and pressure), additional standard thermodynamic properties may be required (e.g., standard internal energies and standard volumes of the species). Therefore, modeling chemically reactive multiphase systems is only possible if standard thermodynamic properties of species or reactions can be computed [@Leal2017].

Equations of state that relate standard thermodynamic properties to temperature and pressure are used to compute these properties. These are known as standard thermodynamic models. The quality of thermodynamic modeling results is strongly influenced by the predictive capability of the model and the availability, accuracy, precision, and consistency of the input parameters in the thermodynamic datasets. Different models have been developed to evaluate standard thermodynamic properties of substances or reactions over a wide range of temperature and pressure conditions. These models may have complex mathematical formulations, a large number of input parameters, and may require iterative numerical procedures, or a combination of several methods to calculate the properties of a particular type of substance. For example, the properties of the solvent (usually water) are used to calculate the properties of the solutes (aqueous species). Specific implementations of extrapolation methods and equations of state can be found in existing thermodynamic modeling codes or as standalone codes that work with specific thermodynamic datasets and file formats. However, most of these codes are not directly compatible, making it difficult to retrieve properties calculated with different methods and use them in different codes, or to compare and tabulate thermodynamic data from different sources. With the many existing thermodynamic datasets, equations of state, modeling codes, and file formats, it can be a time-consuming and error-prone effort to collect and find, implement, and apply suitable models in calculations. This calls for the efficient automation of these various models and data formats with integration into a consistent service.

The ThermoFun library facilitates the computation of standard thermodynamic properties of substances and reactions (see \autoref{fig:properties}). The library is extendable with new models and has remote access to the curated collection of thermodynamic datasets on ThermoHub. These datasets contain reference thermodynamic data that is applicable in various studies on hydrothermal processes, cementitious materials, nuclear engineering, waste incineration, radioactive and chemical management and disposal, and other (geo)chemical environments. Fast and simple access to many thermodynamic datasets, methods, and equations of state allows for more efficient use of these tools in modeling codes, or for the optimization and fitting of model parameters, evaluation of the models' performance, range of applicability, and accuracy. ThermoFun provides the flexibility to use thermodynamic datasets and appropriate models, or a combination of models, that are best suited for different classes of substances in various modeling applications.

![Example of standard thermodynamic properties of substances and reactions calculated using ThermoFun as a function of temperature along the saturated water vapor pressure curve (input parameters from `aq17` thermodynamic dataset [@Miron2016;@Miron2017]).\label{fig:properties}](figure1.png)

The ThermoFun library can be linked to any C++ or Python geochemical equilibrium speciation, parameter optimization, or reactive mass-transport code as a source of standard thermodynamic properties of substances and/or reactions. As a standalone code, ThermoFun can be used simply for searching and tabulating thermodynamic properties of substances and reactions. For example, it has been used to calculate the standard properties of reactants and reactions at elevated temperatures when evaluating the performance of isocoulombic reactions, in order to extrapolate the properties of aqueous complexation reactions involving lanthanides and actinides to elevated temperatures [@Miron2020]. The library was also applied to evaluate the effect of using different equations of state for water-solvent on the calculated properties of dissolved aqueous species [@Miron2019]. 
ThermoFun has already been adopted as a source of thermodynamic data in the [GEMS](https://gems.web.psi.ch/) equilibrium speciation codes [@Kulik2013] and the [Reaktoro](https://reaktoro.org) chemical modeling framework [@Leal2016].

# Features

ThermoFun is initialized with a thermodynamic dataset, which is a database consisting of collections of records for chemical elements, substances (species), and optionally reactions between substances, needed to calculate equilibria in chemical systems. These records contain various data such as symbol, atomic mass, name, chemical formula, standard thermodynamic properties, equation of state (EoS) parameters, etc. for species; and stoichiometry coefficients of involved species, standard thermodynamic effects, and optionally model parameters coefficients, etc. for reactions. Dedicated research and development efforts are devoted to compiling and critically selecting these properties and parameters from literature or fitting them to experimental data. Thermodynamic datasets used in ThermoFun can be read from local files or retrieved via remote access to the ThermoHub online database (and saved locally). The ThermoHub database contains a collection of widely used thermodynamic of [thermodynamic datasets](https://thermohub.org/thermohub/thermohub/#thermodatasets) that are used in various modeling applications.

The online ThermoHub database is actively being extended with existing and newly updated and developed datasets prepared by thermodynamic database experts, which are automatically made available for calculations with ThermoFun. In general, calculations must be done using one internally consistent thermodynamic dataset, but it is also possible to use custom datasets, combine them, and append additional data from user-maintained files. Using the [ThermoHubClient](https://thermohub.org/thermohub/thermohubclient/) utility code, thermodynamic datasets can be retrieved into a simplified ThermoFun format and saved to text files that can be edited and extended.

The ThermoFun `ThermoEngine` class contains functions that return the standard state thermodynamic properties of a substance or a reaction at a given temperature and pressure. For water solvent, additional properties such as water density and dielectric constant can also be retrieved. There is also the option for low-level fine-grained access up to individual methods.

The `ThermoBatch` class can be used to run batch calculations for a list of substances, reactions, and temperature and pressure ranges. `ThermoBatch` can also be used to output the results in tabulated comma-separated values (CSV) or other data formats.

Reaction properties can be calculated for any `reaction` record present in the thermodynamic dataset, with the extrapolation to the given temperature and pressure done using the model code and its input parameters available in the record. Alternatively, the user can provide strings of reaction equations written using symbols of substances available in the thermodynamic dataset (e.g., Calcite $\leftrightharpoons$ Ca$^{2+}$ + CO$_{3}^{2-}$). In this case, the standard properties of reactions are calculated from the properties of reactants, each of which is first computed for the T, P of interest using the models and parameters defined in their respective `substance` records. Another option is to set a reaction with known properties to define the properties of a given substance. In this case, the standard properties of the reaction-defined substance at T, P are retrieved from the standard properties of the reaction calculated at T, P, along with the standard properties of the other reactants (substances) that are involved in the reaction. Thus, one thermodynamic dataset can contain a combination of substances, reaction-dependent substances, and reactions, whose consistent standard properties at T, P of interest are computed recursively.

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

# Tools with similar functionalities

ThermoFun is an open source library that serves as a common source of ever-expanding thermodynamic models for standard properties of substances and reactions. If ThermoFun does not meet your needs, there are other alternatives that offer similar functionality, potentially using different methods, and sometimes embedded in larger applications. The following list is of codes that share some functionality with ThermoFun, but is not exhaustive:

| Name  | Language  |  Reference |  Source |
|-------|-----------|------------|-------|
| Cantera | C++, Python, Matlab | [@cantera] | [Open](https://github.com/Cantera/cantera) |
| CHNOSZ | R, Python | [@Dick2019] | [Open](https://chnosz.net/) |
| coolprop | C++ | [@Bell2014] | [Open](https://github.com/CoolProp/CoolProp)|
| DEWPython | Python | [@Chan2021] | [Open](https://github.com/chandr3w/DEWPython) |
| GeoTPD | R-Shiny | [@Bastrakov2019] | [Closed](https://geoscienceaustralia.shinyapps.io/GeoTPD/) |
| PyGCC | Python | [@AWOLAYO2022]  | [Open](https://bitbucket.org/Tutolo-RTG/pygcc/src/master/) |
| pyromat | Python | [@Martin2022] | [Open](https://github.com/chmarti1/PYroMat)|
| Reaktoro | C++, Python | [@Leal2017] | [Open](https://github.com/reaktoro) |
| REFPPROP | FORTRAN  | [@Huber2022] | [Closed](https://www.nist.gov/srd/refprop) |
| SteamTables | C++, Web | [@megawatsoft] | [Closed](https://www.steamtablesonline.com/steam97web.aspx) |
| SUPCRT92 | Fortran  |  [@JOHNSON1992] | [Open](https://github.com/justinnhli/supcrt92) |
| SUPCRTBL | Fortran | [@ZIMMER2016]  | [Closed](https://models.earth.indiana.edu/supcrtbl.php) |
| Thermo | Python | [@bell] | [Open](https://github.com/CalebBell/thermo)|
| ThermoEngine | C++, Python | [@thermoengine] | [Open](https://gitlab.com/ENKI-portal/ThermoEngine) |

# Acknowledgements

Support to G. D. Miron by the German Federal Ministry for Education and Research (BMBF), ThermAc project (02NUK039A) is gratefully acknowledged. D. A. Kulik and G. D. Miron are grateful for the financial support provided by NAGRA. A.M.M. Leal is grateful for the financial support of the Swiss National Science Foundation (SNSF) through the Ambizione grant PZ00P2-179967.

# References
