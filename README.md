# ThermoFun
Linux, OSX, Windows

[![Build Status](https://dev.azure.com/mirondanro/thermohub/_apis/build/status/thermohub.thermofun?branchName=master)](https://dev.azure.com/mirondanro/thermohub/_build/latest?definitionId=9&branchName=master)

A code for calculating the standard state thermodynamic properties of substances and reactions at a given temperature and pressure. 

**[Guide: basic usage and options](docs/GUIDE.md)** — engine and batch calculations, preferences, derivatives (autodiff), error propagation, rounding.

If you use it in your work please cite the JOSS publication 
[![DOI](https://joss.theoj.org/papers/10.21105/joss.04624/status.svg)](https://doi.org/10.21105/joss.04624)

- [Guide: usage and options](docs/GUIDE.md)
- [Code documentation](https://docs.hdoc.io/dmiron/thermofun/?target=_blank)
- [Simple C++ API example](#simple-c-api-example)
- [Try ThermoFun](#try-thermofun-in-your-browser-click-launch-binder)
- [Python example](#simple-python-api-example)
- [Install using Conda](#installation-using-conda)
- [Build and install using cmake](#install-thermofun-using-cmake)
- [Build and install using cmake and conda](#compile-and-install-thermofun-using-cmake-and-conda)
- [Reporting bugs](#reporting-bugs)
- [Contributing](#contributing-with-development)

## Try ThermoFun in your browser click launch binder

[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/thermohub/thermofun-jupyter/master?urlpath=lab/)

Wait until the Jupyter Lab Notebook server starts (~1 min) then double click on any `how-to-...` tutorial notebook. Binder is a free service and not using the browser tab for more than a few miuntes will turn off the virutal server. To restart the Jupyter Lab Notebook server click again on the launch binder icon above. Refreshing the webpage will not help restarting the  server. 

More information on Jupyter Notebooks: [Jupyter Documentation](https://jupyter.readthedocs.io/en/latest/index.html)

## Simple C++ API example

* Using a json database file

```
#!c++
int main()
{
    // Create the batch object using a database file in JSON
    ThermoFun::ThermoBatch batch("Resources/Databases/aq17-thermofun.json");

    // Optional: set units, default units are in SI
    batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

    // Optional: change default significant digits
    batch.setPropertiesDigits({"gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"}, {0, 1, 2, 0, 0, 0});

    // Retrieve the entropy of H2O
    double H2Oentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble();

    // Retrieve the derivative of G with respect to T
    double H2OdGdT = batch.thermoPropertiesSubstance( 300, 2000, "H2O", "entropy").toThermoScalar().ddt;

    // Write results to a comma separate files for a list of T-P pairs, substances, and properties
    batch.thermoPropertiesSubstance({{25, 1},{40, 1},{70, 100},{90, 100},{100, 100}}, // list of T-P pairs
                                    {"Al+3", "OH-", "SiO2@"},                         // list of substance symbols
                                    {"gibbs_energy","entropy", "volume", "enthalpy"}  // list of properties
                                   ).toCSV("results.csv");                            // output
    return 0;
}
```

* Using the database client and retrieving a ThermoDataSet from the remote database. This example uses the [`thermohubclient`](https://github.com/thermohub/thermohubclient)

```
#!c++
int main()
{
    // Initialize a database client object
    ThermoFun::DatabaseClient dbc;

    // Create a ThermoFun database using the records list
    ThermoFun::Database db(dbc.getDatabase('aq17'));

    // Initialize an batch object using the database
    ThermoFun::ThermoBatch batch (db);

    // Optional set calculation and output preferences
    ThermoFun::OutputSettings op;
    op.isFixed = true;
    op.outputSolventProperties       = true;
    op.reactionPropertiesFromReactants   = false;
    op.substancePropertiesFromReaction   = false;
    batch.setOutputSettings(op);

    // Optional set units and significant digits
    batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});
    batch.setPropertiesDigits({ "reaction_gibbs_energy","reaction_entropy", "reaction_volume",
                                "reaction_enthalpy","logKr", "temperature", "pressure"}, {0, 4, 4, 4, 4, 0, 0});

    batch.thermoPropertiesReaction({{25,1}}, {"AmSO4+", "MgSiO3@"}, {"reaction_gibbs_energy", "reaction_entropy",
                                    "reaction_volume", "reaction_enthalpy", "logKr"}).toCSV("results.csv");

    batch.thermoPropertiesReaction({0,20,50,75},{0,0,0,0},{"AmSO4+", "MgSiO3@"}, {"reaction_gibbs_energy", "reaction_entropy",
                                    "reaction_volume", "reaction_enthalpy", "logKr"}).toCSV("results.csv");
}

```

## Simple Python API example

* Using a json database file

```
#!Python
import thermofun as fun
import thermohubclient as hubclient

properties = fun.ThermoPropertiesSubstance

engine = fun.ThermoEngine("Resources/databases/aq17-thermofun.json")

prop = engine.thermoPropertiesSubstance(373.15, 100000000, "H2O@")

print(prop.gibbs_energy.val)
print(prop.gibbs_energy.ddt)
print(prop.entropy.val)
print(prop.gibbs_energy.ddp)
print(prop.gibbs_energy.err)
print(prop.gibbs_energy.sta)

# Create the engine object using a database file in JSON
batch = fun.ThermoBatch("Resources/databases/aq17-thermofun.json")

# Optional: change default units
batch.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

# Optional: change default significant digits
batch.setPropertiesDigits(["gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"], [0, 1, 2, 0, 0, 0])

H2Oentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble()
print(H2Oentropy)

V = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "volume").toThermoScalar()

# Write results to a comma separate files for a list of T-P pairs, substances, and properties
batch.thermoPropertiesSubstance( [[25, 1],[40, 1],[70, 100],[90, 100],[100, 100]],  # // list of T-P pairs
                                 ["Al+3", "OH-", "SiO2@"],                          # // list of substance symbols
                                 ["gibbs_energy","entropy", "volume", "enthalpy"]   # // list of properties
                               ).toCSV("results.csv")    
```

* Using the database client and retrieving a ThermoDataSet from the remote database. This example uses the `thermohubclient`, that can be installed from conda-forge executing `conda install -c conda-forge thermohubclient`

```
#!Python
import thermofun as fun
import thermohubclient as hubclient

print("\n# Initialize a database client object\n")
dbc = hubclient.DatabaseClient()

print("ThermoDataSets")
for t in dbc.availableThermoDataSets():
    print(f'{t}')
print('\n')

aq17 = fun.Database(dbc.getDatabase('aq17'))

print("\n# Initialize an interface object using the database\n")
batch2 = fun.ThermoBatch(aq17)

print("\n# Optional: set the solvent symbol used for calculating properties of aqueous species\n")
batch2.setSolventSymbol("H2O@")

print("\n# Optional set calculation and output preferences\n")
op = fun.BatchPreferences()
op.isFixed = True
op.outputSolventProperties       = True
op.reactionPropertiesFromReactants   = False
op.substancePropertiesFromReaction   = False
batch2.setBatchPreferences(op)

print("\n# Optional set units and significant digits\n")
batch2.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

batch2.setPropertiesDigits(["gibbs_energy","entropy", "volume",
                            "enthalpy","logKr", "temperature", "pressure"], [0, 4, 4, 4, 4, 0, 0])

print("\n# Do calculations and write output\n")
batch2.thermoPropertiesSubstance([[25,1]], ["NaCO3-", "Mg+2"], ["gibbs_energy", "entropy",
                                "volume", "enthalpy"]).toCSV("results_dbc.csv")
```

## Installation using Conda

ThermoFun can be easily installed using [Conda](https://conda.io/docs/) package manager. If you have Conda installed, first add the conda-forge channel by executing 

```
#!bash
conda config --add channels conda-forge
```

install ThermoFun by executing the following command:

```
#!bash
conda install thermofun
```

Conda can be installed from [Miniconda](https://conda.io/miniconda.html).

## Install ThermoFun using CMake

* Make sure you have g++, cmake and git installed. If not, install them (on Ubuntu Linux):

```
#!bash
sudo apt-get install g++ cmake git
```

* Download ThermoFun source code using git clone

* In a terminal, at the home directory level e.g. ```<user>@ubuntu:~$``` copy-paste and run the following code:

```
#!bash
git clone https://github.com/thermohub/thermofun.git && cd thermofun 
```

* In the terminal you should be in ```~/thermofun$```.

### (A) Build and install ThermoFun library (working with json database files)

This option allows the user to build thermofun library that works with a user provided thermodynamic database file in json format and has only one thirdpary library dependency. To build thermofun with access to the thermohub thermodynamic database cloud and local server see bellow. 

#### Install Dependencies (if not using Conda environment)

ThermoFun computes derivatives of the thermodynamic properties with [autodiff](https://autodiff.github.io/) (header-only, v1.1.1 or newer). If autodiff is not installed, CMake fetches it automatically (see [Derivatives with autodiff](#derivatives-with-autodiff)). The models calculate with `autodiff::real` (two passes, with the temperature and with the pressure seeded, see `Common/ThermoProperty.hpp`); the results are `ThermoProperty` instances (`ThermoScalar` in Python) with the value `val`, the derivatives `ddt` and `ddp`, the error `err` and the status `sta`.

The thermofun library uses nlohmann/json.hpp as thirdparty dependency to parse database files in json format. To install the header only json library in a terminal ```~/thermofun$``` execute the following: 

```
#!bash
sudo ./install-dependencies.sh
```

#### Compiling the C++ library

In the terminal ```~/thermofun$```, execute the following commands:

```
#!bash
mkdir build && \
cd build && \
cmake .. && \
make
``` 

To take advantage of parallel compilation use ```make -j3```. 3 representing the number of threads. 

For a global installation of the compiled libraries in your system, execute:

```
#!bash
sudo make install 
```

This will install Thermofun library and header files in the default installation directory of your system (e.g, ```/usr/local/``` or if conda is active, in the instalation directory of the conda environment).

For a local installation, you can specify a directory path for the installed files as follows:

```
#!bash
cmake .. -DCMAKE_INSTALL_PREFIX=/home/username/local/
```
then execute:

```
sudo make install 
```

To compile ThermoFun library in debug mode:

```
#!bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
then execute:

```
sudo make install 
```

### (B) Build and install ThermoFun library (working with access to the local and cloud ThemroHub database)

This option builds thermofun library together with the dbclient, which provides access to the local and cloud thermohub databases, allowing specific a ThermoDataSet to be used or a selection on elements of the thermodynamic data.

#### Install ThermoHubClient

Clone and install ThermoHubClient library 

```
#!bash
git clone https://bitbucket.org/gems4/thermohubclient.git
cd thermohubclient
sudo ./install-dependencies.sh
mkdir build
cd build
cmake ..
make
```

For a global installation of the compiled library in your system, execute:

```
#!bash
sudo make install 
```

## Compile and install ThermoFun using CMake and Conda 

This procedure uses Conda for handling all the dependencies of ThermoFun and builds ThermoFun for Windows, Mac OS X, and Linux. 

Once you have conda installed execute:

```
#!bash
conda install -n base conda-devenv
```
This installs conda-devenv, a conda tool used to define and initialize conda environments.

Download ThermoFun from github

```
#!bash
git clone https://github.com/thermohub/thermofun.git && cd thermofun 
```

In the next step we create a clean environment with all dependencies necessary to build ThermoFun, executing:

```
#!bash
conda devenv 
```

In the next step we need to activate the thermofun environment 

```
#!bash
conda activate thermofun
```

Remember to always activate thermofun environment whenever you use ThermoFun from C++ or Python. This is because conda will adjust some environment variables in your system. 

Now we can proceed and [build ThermoFun using CMake.](#install-thermofun-using-cmake)

## Reporting bugs

To report a bug, please go to [ThermoFun's Issues](https://github.com/thermohub/thermofun/issues/new) and enter a *descriptive title* and *write your issue with enough details*. Please provide a *minimum reproducible example* to be more efficient in identifying the bug and fixing it. 

For questions and issues don't hesitate to chat with us on [Gitter](https://gitter.im/thermofun/community).

## Contributing with development

The [Fork & Pull Request Workflow](https://docs.github.com/en/get-started/quickstart/contributing-to-projects) is used. Below is a summary of the necessary steps you need to take:

1. Fork this repository
2. Clone the repository at your machine
3. Add your changes in a branch named after what's being done (`lower-case-with-hyphens`)
4. Make a pull request to `thermohub/thermofun`, targeting the `main` branch

## Derivatives with autodiff

Every thermodynamic property is a `ThermoProperty` (`ThermoScalar` in Python), which carries its value together with the derivatives with respect to temperature (`ddt`) and pressure (`ddp`), plus an error and a status. The derivatives are computed by forward-mode automatic differentiation with [autodiff](https://autodiff.github.io/) (`autodiff::real`), not by finite differences. Inside the models the calculation runs twice with `autodiff::real`, once with T seeded and once with P seeded (`twoPass`); the result stores the value `val` and the derivatives `ddt`, `ddp` as plain members, with the error `err` and the status `sta`.

Derivatives of composite quantities follow from the chain rule, e.g. the derivative of the reaction `logK` or of the Gibbs energy of a reaction is obtained from the derivatives of its reactants, with no extra code in the models.

**C++**

```cpp
#include <ThermoFun/ThermoFun.h>

ThermoFun::Database db("Resources/Databases/aq17-thermofun.json");
ThermoFun::ThermoEngine engine(db);

// T in K, P in Pa
auto prop = engine.thermoPropertiesSubstance(373.15, 1e8, "H2O@");

double G    = prop.gibbs_energy.val;   // value
double dGdT = prop.gibbs_energy.ddt;   // d G / d T  (= -S)
double dGdP = prop.gibbs_energy.ddp;   // d G / d P  (= V)

// the same through the batch interface
ThermoFun::ThermoBatch batch("Resources/Databases/aq17-thermofun.json");
double dSdT = batch.thermoPropertiesSubstance(100, 1000, "H2O@", "entropy").toThermoScalar().ddt;
```

**Python**

```python
import thermofun as fun

engine = fun.ThermoEngine("Resources/Databases/aq17-thermofun.json")
prop = engine.thermoPropertiesSubstance(373.15, 1e8, "H2O@")

print(prop.gibbs_energy.val)   # value
print(prop.gibbs_energy.ddt)   # d G / d T
print(prop.gibbs_energy.ddp)   # d G / d P
```

### Disabling autodiff

By default the models calculate with `autodiff::real` and the derivatives are propagated alongside the values. What you can control:

* **Build without autodiff.** `cmake -DTFUN_USE_AUTODIFF=OFF ...`: the models calculate with plain numbers (`Common/Real.hpp`), autodiff is not needed (not found, not downloaded, not linked) and the calculations are faster (about 1.3 to 1.8 times in a quick measurement). All derivatives `ddt`, `ddp` are 0, the values, errors and statuses are identical to the autodiff build, and the properties whose value needs a derivative (Cv of substances, `densityTP` of the GEMS HGK water) are not defined. `thermofun.with_autodiff` (Python) is `False` in this build; projects that use ThermoFun through CMake get the setting automatically. The derivative tests (`tests/autodiff`, `pytests/test_autodiff.py`) are not built or are skipped.
* **Ignore the derivatives.** Use only `.val` or `toDouble()` on the batch results. The `ddt`/`ddp` parts are then simply not read.
* **Do not download it during the build.** If autodiff is found by CMake (`find_package(autodiff 1.1.1)`), it is used and nothing is fetched. Install it beforehand with `sudo ./install-dependencies.sh`, or with conda (`conda install autodiff -c conda-forge`), or point CMake to an existing installation with `-Dautodiff_DIR=<path>`. The fetch from GitHub only happens when no installation is found.
* **Skip its tests.** The autodiff test in `tests/autodiff` is only built with `-DTFUN_BUILD_TESTS=ON` (or `-DTFUN_BUILD_ALL=ON`), so leave those off.
