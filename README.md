# ThermoFun
[![Build Status](https://travis-ci.com/thermohub/thermofun.svg?branch=master)](https://travis-ci.com/thermohub/thermofun)
[![Build status](https://ci.appveyor.com/api/projects/status/13098q3j713uo06v/branch/master?svg=true)](https://ci.appveyor.com/project/gdmiron/thermofun-44e8m/branch/master)

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

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
    ThermoFun::Batch batch("Resources/Databases/aq17-thermofun.json");

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

* Using the database client and retrieving a ThermoDataSet from the remote database

```
#!c++
int main()
{
    // Set the file path to the database connection and preferences file (provided in the Resources/ folder)
    setDatabaseConnectionFilePath("fun-hubclient-config.json");

    // Initialize a database client object
    ThermoFun::DatabaseClient dbc;

    // Retrieve list of records given a ThermoDataSet symbol
    auto records = dbc.recordsFromThermoDataSet("psinagra07"); 

    // Create a ThermoFun database using the records list
    ThermoFun::Database db = databaseFromRecordList(dbc, records);

    // Initialize an batch object using the database
    ThermoFun::Batch batch (db);


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

### Simple Python API example

* Using a json database file

```
#!Python
import thermofun as fun
import thermohubclient as hubclient

properties = fun.ThermoPropertiesSubstance

engine = fun.ThermoEngine("Resources/Databases/aq17-thermofun.json")

prop = engine.thermoPropertiesSubstance(373.15, 100000000, "H2O@")

print(prop.gibbs_energy.val)
print(prop.gibbs_energy.ddt)
print(prop.entropy.val)
print(prop.gibbs_energy.ddp)
print(prop.gibbs_energy.err)
print(prop.gibbs_energy.sta)

# Create the engine object using a database file in JSON
batch = fun.ThermoBatch("Resources/aq17new-format.json")

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
                               ).toCSV("results.csv")                               # // output
```

* Using the database client and retrieving a ThermoDataSet from the remote database

```
#!Python
hubclient.setDatabaseConnectionFilePath("Resources/fun-hubclient-config.json")

print("\n# Initialize a database client object\n")
dbc = hubclient.DatabaseClient()

print("\n# Retrieve list of records given a ThermoDataSet symbol\n")
records = dbc.recordsFromThermoDataSet("cemdata18") 

print("\n# Create a ThermoFun database using the records list\n")
db = hubclient.databaseFromRecordList(dbc, records)

print("\n# Initialize an interface object using the database\n")
batch2 = fun.ThermoBatch(db)

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
batch2.thermoPropertiesSubstance([[25,1]], ["Na(CO3)-", "Mg+2"], ["gibbs_energy", "entropy",
                                "volume", "enthalpy"]).toCSV("results_dbc.csv")
```

## Installation using Conda

ThaemroFun can be easily installed using [Conda](https://conda.io/docs/) package manager. If you have Conda installed, install ThermoFun by executing the following command:

```
#!bash
conda install thermofun
```
Conda can be installed by installing [Miniconda](https://conda.io/miniconda.html).

## Install ThermoFun using CMake

* Make sure you have g++, cmake and git installed. If not, install them (on Ubuntu Linux):

```
#!bash
sudo apt-get install g++ cmake git
```

## Download ThermoFun source code using git clone

* In a terminal, at the home directory level e.g. ```<user>@ubuntu:~$``` copy-paste and run the following code:

```
#!bash
git clone https://github.com/thermohub/thermofun.git && cd thermofun 
```

* In the terminal you should be in ```~/thermofun$```.

## (A) Build and install ThermoFun library (working with json database files)

This option allows the user to build thermofun library that works with a user provided thermodynamic database file in json format and has only one thirdpary library dependency. To build thermofun with access to the thermohub thermodynamic database cloud and local server see bellow. 

### Install Dependencies (if not using Conda environment)

The thermofun library uses nlohmann/json.hpp as thirdparty dependency to parse database files in json format. To install the header only json library in a terminal ```~/thermofun$``` execute the following: 

```
#!bash
sudo ./install-dependencies.sh
```

### Compiling the C++ library

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

This will install Thermofun library and header files in the default installation directory of your system (e.g, ```/usr/local/``` ).

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

## (B) Build and install ThermoFun library (working with access to the local and cloud ThemroHub database)

This option builds thermofun library together with the dbclient, which provides access to the local and cloud thermohub databases, allowing specific a ThermoDataSet to be used or a selection on elements of the thermodynamic data.

### Install ThermoHubClient

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

## Install ThermoFun using CMake and Conda 

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
