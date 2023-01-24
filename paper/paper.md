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

[ThermoFun](https://thermohub.org/thermofun) is an open source library that facilitates the calculation and retrieval of standard thermodynamic properties of chemical substances, compounds, and reactions among them at a range of temperatures and pressures. The library is developed in C++ for performance, but it also has a Python API for broader and more convenient usage. It employs a variety of thermodynamic models and equations of state for solid, aqueous, surface, gaseous, and molten substances, and their reactions with input properties and parameters from various thermodynamic datasets that are collected and curated in the in [ThermoHub](https://thermohub.org/thermohub) database. The library can be used as a standalone code for searching and tabulating thermodynamic properties or linked to other modeling codes that require standard thermodynamic data as input. It offers the flexibility to use different thermodynamic datasets, including custom datasets and datasets retrieved from the online ThermoHub database, and to choose the most suitable models for different classes of substances necessary in various modeling applications. It can serve as a common source of thermodynamic models for standard properties of substances and reactions that can be easily integrated and combined, significantly improving the modeling capabilities for diverse (geo)chemical systems and over wide ranges of conditions.

# Statement of need

Thermodynamic modeling is a powerful tool that allows for the investigation of equilibrium speciation in chemical systems under conditions and time scales that cannot be reached in laboratory settings. It requires the standard thermodynamic properties of all involved substances (species) or reactions at a specific temperature (T) and pressure (P) as input. Equations of state are used to compute these properties, known as standard thermodynamic models. However, existing thermodynamic modeling codes and standalone codes may not be directly compatible, making it difficult to retrieve properties calculated with different methods and use them in different codes, or to compare and tabulate thermodynamic data from different sources. The ThermoFun library simplifies this process by facilitating the computation of standard thermodynamic properties of substances and reactions (see \autoref{fig:properties}), and providing fast and simple access to many thermodynamic datasets, methods, and equations of state. This allows for more efficient use of these tools in modeling codes, or for the optimization and fitting of model parameters, evaluation of the models' performance, range of applicability, and accuracy. Additionally, ThermoFun is extendable with new models and has remote access to a curated collection of thermodynamic datasets on ThermoHub, making it a useful tool for various studies on hydrothermal processes, cementitious materials, nuclear engineering, waste incineration, radioactive and chemical management and disposal, and other (geo)chemical environments.

![Example of standard thermodynamic properties of substances and reactions calculated using ThermoFun as a function of temperature along the saturated water vapor pressure curve (input parameters from `aq17` thermodynamic dataset [@Miron2016;@Miron2017]).\label{fig:properties}](figure1.png)

The ThermoFun library can be linked to any C++ or Python geochemical equilibrium speciation, parameter optimization, or reactive mass-transport code as a source of standard thermodynamic properties of substances and/or reactions. As a standalone code, ThermoFun can be used simply for searching and tabulating thermodynamic properties of substances and reactions. For example, it has been used to calculate the standard properties of reactants and reactions at elevated temperatures when evaluating the performance of isocoulombic reactions, in order to extrapolate the properties of aqueous complexation reactions involving lanthanides and actinides to elevated temperatures [@Miron2020]. The library was also applied to evaluate the effect of using different equations of state for water-solvent on the calculated properties of dissolved aqueous species [@Miron2019]. 
ThermoFun has already been adopted as a source of thermodynamic data in the [GEMS](https://gems.web.psi.ch/) equilibrium speciation codes [@Kulik2013] and the [Reaktoro](https://reaktoro.org) chemical modeling framework [@Leal:2015].

# Benefits of using ThermoFun

* It simplifies the calculation of standard thermodynamic properties of substances and reactions at a range of temperatures and pressures.
* It includes a variety of equations of state and other thermodynamic models that can be used to calculate standard properties under different conditions, eliminating the need for the user to implement and test these models separately in their own code or spreadsheets.
* It uses input properties and parameters from various thermodynamic datasets that are consistently kept in the online ThermoHub database, reducing the need to collect all necessary thermodynamic data from literature or to write complex scripts to import the data from different formats.
* It uses automatic differentiation to provide temperature and pressure derivatives of the calculated properties. `autodiff` C++ library [@Leal:2018] will be used in ThermoFun for even more capable autodiff capabilities, as it has been used in both Reaktoro [@Leal:2015] and TEQP [@Bell:Deiters:Leal:2022].
* The thermodynamic models in ThermoFun are implemented and tested based on their original publications, and the thermodynamic data in ThermoHub is curated and documented by experts, saving time and effort and enabling more efficient use of geochemical code tools for scientific problems.
* It serves as a common source of thermodynamic models for standard properties of substances and reactions that can be easily integrated and combined, significantly improving modeling capabilities for a wide range of (geo)chemical systems and conditions.

Jupyter notebooks with examples on how to use `ThermoFun` can be found in the [thermofun-jupyter](https://github.com/thermohub/thermofun-jupyter) repository. More complex Python or C++ routines that use `ThermoFun` can be coded. For example, `ThermoFun` can be coupled to a phase diagram plotter, a chemical equilibrium solver, or a parameter optimization routine for fitting the standard properties and model parameters of substances and reactions and their temperature and pressure dependence.

# Codes with similar functionalities

ThermoFun is an open source library with several [benefits](#benefits-of-using-thermofun) that serves as a common source of ever-expanding thermodynamic models for standard properties of substances and reactions. If ThermoFun does not meet your needs, there are other alternatives offering related functionality, potentially using different methods, and sometimes embedded in larger applications. Below the (not exhaustive) list of codes that share some functionality with ThermoFun:

| Name  | Language  |  Reference |  Source |
|-------|-----------|------------|-------|
| Cantera | C++, Python, Matlab | [@cantera] | [Open](https://github.com/Cantera/cantera) |
| CHNOSZ | R, Python | [@Dick2019] | [Open](https://chnosz.net/) |
| coolprop | C++ | [@Bell2014] | [Open](https://github.com/CoolProp/CoolProp)|
| DEWPython | Python | [@Chan2021] | [Open](https://github.com/chandr3w/DEWPython) |
| GEM-Selektor | C++ | [@Kulik2013] | [Open](https://gems.web.psi.ch/GEMS3) |
| GeoTPD | R-Shiny | [@Bastrakov2019] | [Closed](https://geoscienceaustralia.shinyapps.io/GeoTPD/) |
| PyGCC | Python | [@AWOLAYO2022]  | [Open](https://bitbucket.org/Tutolo-RTG/pygcc/src/master/) |
| pyromat | Python | [@Martin2022] | [Open](https://github.com/chmarti1/PYroMat)|
| Reaktoro | C++, Python | [@Leal2017] | [Open](https://github.com/reaktoro) |
| REFPPROP | FORTRAN  | [@Huber2022] | [Closed](https://www.nist.gov/srd/refprop) |
| SteamTables | C++, Web | [@megawatsoft] | [Closed](https://www.steamtablesonline.com/steam97web.aspx) |
| SUPCRT92 | Fortran  |  [@JOHNSON1992] | [Open](https://github.com/justinnhli/supcrt92) |
| SUPCRTBL | Fortran | [@ZIMMER2016]  | [Closed](https://models.earth.indiana.edu/supcrtbl.php) |
| TEQP | C++, Python | [@Bell:Deiters:Leal:2022] | [Open](https://github.com/usnistgov/teqp) |
| Thermo | Python | [@bell] | [Open](https://github.com/CalebBell/thermo)|
| ThermoEngine | C++, Python | [@thermoengine] | [Open](https://gitlab.com/ENKI-portal/ThermoEngine) |


# Acknowledgements

Support to G. D. Miron by the German Federal Ministry for Education and Research (BMBF), ThermAc project (02NUK039A) is gratefully acknowledged. D. A. Kulik and G. D. Miron are grateful for the financial support provided by NAGRA. A.M.M. Leal is grateful for the financial support of the Swiss National Science Foundation (SNSF) through the Ambizione grant PZ00P2-179967.

# References
