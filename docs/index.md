<!--
 Copyright (C) 2022 dmiron
 
 This file is part of thermofun.
 
 thermofun is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 thermofun is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with thermofun.  If not, see <http://www.gnu.org/licenses/>.
-->

# [ThermoFun](https://thermohub.org/thermofun/thermofun/)

#### Details on the API functionality can be found by [searching](https://docs.hdoc.io/dmiron/thermofun/search.html) the documentation. The naming is the  same in C++ and Python.

Using ThermoFun is as simple as loading a [database](https://github.com/thermohub/thermofun-jupyter/blob/master/how-to-use-a-database.ipynb) initializing a [ThermoEngine](https://github.com/thermohub/thermofun-jupyter/blob/master/how-to-calculate-properties-thermoengine.ipynb)/[ThermoBatch](https://github.com/thermohub/thermofun-jupyter/blob/master/how-to-do-batch-calculations.ipynb) and calling the function to [calculate the properties](https://github.com/thermohub/thermofun-jupyter/blob/master/how-to-calculate-properties-substance-reaction.ipynb) for given substance(s)/reaction(s) at given T-P condition(s).

```python
  import thermofun as fun
  database = fun.Database('aq17-thermofun.json')
  engine = fun.ThermoEngine(database)
  #                                        T(K)   P(Pa)  symbol
  Ca_ion = engine.thermoPropertiesSubstance(473, 2000e5, 'Ca+2')
  print(f'G0 {Ca_ion.gibbs_energy.val} J/mol')
  #                                                     T(K)   P(Pa)  reaction equation
  calcite_dissolution = engine.thermoPropertiesReaction(348.15, 1e5, 'Calcite = Ca+2 + CO3-2')
  print(f'drS0 of (Cal = Ca+2 + CO3-2) is {calcite_dissolution.reaction_entropy.val}')
  print(f'drG0 of (Cal = Ca+2 + CO3-2) is {calcite_dissolution.reaction_gibbs_energy.val}')
  print(f'logK0 of (Cal = Ca+2 + CO3-2) is {calcite_dissolution.log_equilibrium_constant.val}')
```

In Python using the `help` function can provide hints on the usability.

```python
help(fun.ThermoEngine)

Help on class ThermoEngine in module thermofun.PyThermoFun:

class ThermoEngine(pybind11_builtins.pybind11_object)
 |  Contains methods for calculating the thermodynamic properties of the substances and reactions
 ...
```

Try the ThermoFun examples in your browser:

[![Launch ThermoFun tutorials](https://img.shields.io/badge/launch-ThermoFun%20tutorials-brightgreen?style=for-the-badge&logo=jupyter)](https://mybinder.org/v2/gh/thermohub/thermofun-jupyter/master?urlpath=lab/)

## The main ThermoFun classes

* [Database](https://docs.hdoc.io/dmiron/thermofun/r820E4166D6856B34.html): The Database class stores maps of elements, substances and reactions. A database instance can be used to create a ThermoEngine instance which can be further used to calculate the standard thermodynamic properties of substances and reactions at T and P

[![Launch example on initializing a database from file](https://img.shields.io/badge/launch-Database%20example-brightgreen?style=for-the-badge&logo=jupyter)](https://mybinder.org/v2/gh/thermohub/thermofun-jupyter/master?urlpath=lab/tree/how-to-use-a-database.ipynb)

* [ThermoEngine](https://docs.hdoc.io/dmiron/thermofun/r15CBE4920E8D3EF1.html): The main ThermoFun class for doing calculations. It calculates the thermodynamic properties of the substances from the database. It also calculates the electro-chemical and other physico-chemical properties of the solvent.

[![Launch example on using ThermoEngine to calculate thermodynamic properties](https://img.shields.io/badge/launch-ThermoEngine%20example-brightgreen?style=for-the-badge&logo=jupyter)](https://mybinder.org/v2/gh/thermohub/thermofun-jupyter/master?urlpath=lab/tree/how-to-calculate-properties-thermoengine.ipynb)

* [ThermoBatch](https://docs.hdoc.io/dmiron/thermofun/r88DCC0D87A971DFF.html): The Batch class is useful for doing batch calculations for a given list of substances, reactions, properties, and temperature and pressure grid. Options to set the input and output properties units are available.

[![Launch example on doing batch calculations, for a list of substances, T-P intervals](https://img.shields.io/badge/launch-ThermoBatch%20example-brightgreen?style=for-the-badge&logo=jupyter)](https://mybinder.org/v2/gh/thermohub/thermofun-jupyter/master?urlpath=lab/tree/how-to-do-batch-calculations.ipynb)
