# ThermoFun

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

## Prepare building tools

* Make sure you have g++, cmake and git installed. If not, install them (on Ubuntu Linux):

```
#!bash
sudo apt-get install g++ cmake git
```

## Download ThermoFun source code using git clone

* In a terminal, at the home directory level e.g. ```<user>@ubuntu:~$``` copy-paste and run the following code:

```
#!bash
git clone https://bitbucket.org/gems4/thermofun.git && cd thermofun 
```

* In the terminal you should be in ```~/thermofun$```.

## (A) Build and install ThermoFun library (working with json database files)

This option allows the user to build thermofun library that works with a user provided thermodynamic database file in json format and has only one thirdpary library dependency. To build thermofun with access to the thermohub thermodynamic database cloud and local server see bellow. 

### Install Dependencies

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

### Install Dependencies

Installing dependencies needed to build ThermoFun/dbclient on (k)ubuntu linux 16.04 or 18.04, in a terminal ```~/thermofun/build$``` execute the following: 

```
#!bash
sudo ./install-dependencies-dbclient.sh
```

### Compiling the C++ library

In the terminal ```~/thermofun/build$```, execute the following commands:

```
#!bash
cmake .. -DTFUN_BUILD_DBCLIENT=ON && \
make
``` 

To take advantage of parallel compilation use ```make -j3```. 3 representing the number of threads. 

For a global installation of the compiled libraries in your system, execute:

```
#!bash
sudo make install 
```

### Install the current version of ArangoDB server locally (if not yet installed)

For using ThermoFun with a local arangodb client. If only the remote db.thermohub.net database will be used, the installation of a local arangodb client is not necessary. Proceed to the next section Install Dependencies
In a terminal copy-paste and run to flowing code: (for possibly newer versions of arangodb check [click here](https://www.arangodb.com/download-major/ubuntu/). In the arangodb packedge configuration we recommend to leave ```root``` password empty, and click enter for the following questions, using default selections. For Backup database files before upgrading select "Yes".

On (K)Ubuntu linux, install the current version of ArangoDB server locally [from here](https://www.arangodb.com/download-major/ubuntu/) by using a terminal to copy-paste and run the following commands:

~~~
curl -OL https://download.arangodb.com/arangodb34/DEBIAN/Release.key
sudo apt-key add - < Release.key
echo 'deb https://download.arangodb.com/arangodb34/DEBIAN/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list
sudo apt-get install apt-transport-https
sudo apt-get update
sudo apt-get install arangodb3=3.4.4-1
~~~

The updates will come together with other ubuntu packages that you have installed.


## (C) Build and run ThermoFun GUI Demo

To be able to build and run the ThemroFun GUI (graphical user batch) application demo, Qt needs to be installed.

* Download and install Qt 5.12.2 (https://www1.qt.io/download/) in your home directory ```~/Qt```. In the "Select components to install" menu select: Qt 5.12.2 with Desktop gcc 64-bit, Qt Charts, and Qt WebEngine

### Install Dependencies

Installing dependencies needed to build ThermoFun on (k)ubuntu linux 16.04 or 18.04, in the terminal go in ```~/thermofun$``` and execute the following: 

```
#!bash
sudo ./install-dependencies-gui.sh $HOME/Qt/5.12.2/gcc_64
```

This step will download, configure, build, and install all dependencies: `lua5.3-dev`, `libboost-all-dev`, `libcurl4-openssl-dev`, `libboost-test-dev`, `automake`, `flex`, `bison`, `libssl-dev`, `pugixml`, `yaml-cpp`,  `thrift`, `velocypack`, `jsonio`, `jsonimpex`, `jsonui`. The script will check if the dependencies are not already present at the default installation path ```/usr/local/``` and will only install them if not found. 
To reinstall dependencies open `install-dependencies.sh` and/or `install-dependencies-gui.sh` files with a text editor. At the beginning of the script file commands for removing dependency library files are present but they are commented out with `#` symbol. Remove `#` for each dependency you wish to be reinstalled. 

### Compiling the ThermoFun GUI demo

In the terminal at ```~/thermofun/build$```, execute the following commands:

```
#!bash
cmake .. -DTFUN_BUILD_DBCLIENT=ON -DTFUN_BUILD_GUI=ON -DTFUN_BUILD_DEMOGUI=ON -DCMAKE_PREFIX_PATH=$HOME/Qt/5.11.2/gcc_64 && \
make
``` 

-DTFUN_BUILD_GUI=ON -DTFUN_BUILD_DEMOGUI=ON options trigger the compilation of ThermoFunGui library and the gui demo widget. 

The build script will also copy into the build folder the necessary /Resources folder. In the Resources folder a file named "fun-dbclient-config.json" is present and contains the arangodb database connection preferences. 

To run the ThermoFun GUI demo in the terminal at ```~/build$``` execute:

```
./guidemo.sh
```

You can change the arango database connection setup using the preferences icon on the start widget, or by making changes in ```Resources/fun-dbclient-config.json``` file.

* For building using Qt Creator, use the ThermoFunDemoGUI.pro project file found in  ```~/thermofun/fungui```.

## (D) Build and install the Python interface

### Install Dependencies

The thermofun python api uses pybind11 as thirdparty dependency. To install the header only json library in a terminal ```~/thermofun$``` execute the following: 

```
#!bash
sudo ./install-dependencies-py.sh
```

## Compiling the Python interface

In the terminal ~/thermofun/build$, execute the following commands:

```
#!bash
cmake .. -DTFUN_BUILD_PYTHON=ON
```

with DBClient

```
#!bash
cmake .. -DTFUN_BUILD_DBCLIENT=ON -DTFUN_BUILD_PYTHON=ON
```

then execute:

```
#!bash
make
```

After compilation, you should find the shared library PyThermoFun inside the directory ```lib``` in the build directory. This library is also a Python module, which can be imported from Python as:

```
from thermofun import *
```

In the build folder a test file text.py can be found, this can be run by executing ```python test.py``` and ```python test-dbc.py``` . If successful in addition to on screen output, two files, results.csv and results_dbc.csv files containing calculation results will be created. 

If compilation was successful the /Resources folder will be copied to the build folder. In this folder the necessary test files, connection to the arangodb database preferences, and data schemas are present. To change database connection without using the GUI, make changes in the ```Resources/fun-dbclient-config.json``` file.


### Simple C++ API example

* Using a json database file

```
#!c++
int main()
{
    // Create the batch object using a database file in JSON
    ThermoFun::Batch batch("aq17.json");

    // Optional: set the solvent symbol used for calculating properties of aqueous species
    batch.setSolventSymbol("H2O@");

    // Optional: change default units
    batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

    // Optional: change default digits
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
    setDatabaseConnectionFilePath("fun-dbclient-config.json");

    // Initialize a database client object
    ThermoFun::DatabaseClient dbc;

    // Retrieve list of records given a ThermoDataSet symbol
    auto records = dbc.recordsFromThermoDataSet("PSINagra07"); 

    // Create a ThermoFun database using the records list
    ThermoFun::Database db = databaseFromRecordList(dbc, records);

    // Initialize an batch object using the database
    ThermoFun::Batch batch (db);

    // Optional: set the solvent symbol used for calculating properties of aqueous species
    batch.setSolventSymbol("H2O@");

    // Optional set calculation and output preferences
    ThermoFun::OutputSettings op;
    op.isFixed = true;
    op.outSolventProp       = true;
    op.calcReactFromSubst   = false;
    op.calcSubstFromReact   = false;
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
import PyThermoFun
import PyThermoDBClient

properties = PyThermoFun.ThermoPropertiesSubstance

engine = PyThermoFun.ThermoEngine("Resources/aq17new-format.json")

prop = engine.thermoPropertiesSubstance(373.15, 100000000, "H2O@")

print(prop.gibbs_energy.val)
print(prop.gibbs_energy.ddt)
print(prop.entropy.val)
print(prop.gibbs_energy.ddp)
print(prop.gibbs_energy.err)
print(prop.gibbs_energy.sta)

# Create the engine object using a database file in JSON
batch = PyThermoFun.ThermoBatch("Resources/aq17new-format.json")

# Optional: set the solvent symbol used for claulating properties of aqueous species
batch.setSolventSymbol("H2O@")

# Optional: change default units
batch.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

# Optional: change default digits
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
PyThermoDBClient.setDatabaseConnectionFilePath("Resources/fun-dbclient-config.json")

print("\n# Initialize a database client object\n")
dbc = PyThermoDBClient.DatabaseClient()

print("\n# Retrieve list of records given a ThermoDataSet symbol\n")
records = dbc.recordsFromThermoDataSet("Cemdata18") 

print("\n# Create a ThermoFun database using the records list\n")
db = PyThermoDBClient.databaseFromRecordList(dbc, records)

print("\n# Initialize an interface object using the database\n")
batch2 = PyThermoFun.ThermoBatch(db)

print("\n# Optional: set the solvent symbol used for calculating properties of aqueous species\n")
batch2.setSolventSymbol("H2O@")

print("\n# Optional set calculation and output preferences\n")
op = PyThermoFun.BatchPreferences()
op.isFixed = True
op.outSolventProp       = True
op.calcReactFromSubst   = False
op.calcSubstFromReact   = False
batch2.setBatchPreferences(op)

print("\n# Optional set units and significant digits\n")
batch2.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

batch2.setPropertiesDigits(["gibbs_energy","entropy", "volume",
                            "enthalpy","logKr", "temperature", "pressure"], [0, 4, 4, 4, 4, 0, 0])

print("\n# Do calculations and write output\n")
batch2.thermoPropertiesSubstance([[25,1]], ["Na(CO3)-", "Mg+2"], ["gibbs_energy", "entropy",
                                "volume", "enthalpy"]).toCSV("results_dbc.csv")
```
