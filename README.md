# ThermoFun

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

## Build and Run Tests on Linux

## Prepare building tools

* Make sure you have g++, cmake and git installed. If not, install them (on Ubuntu Linux):

```
#!bash
sudo apt-get install g++ cmake git
```

## Prepare folder structure and download ThermoFun source code

* In a terminal, at the home directory level e.g. ```<user>@ubuntu:~$``` copy-paste and run the following code:

```
#!bash
mkdir gitTHERMOFUN && cd gitTHERMOFUN && \
mkdir build && cd build && \
mkdir release && mkdir debug && cd .. && \
git clone https://bitbucket.org/gems4/thermofun.git && cd thermofun
```

* In the terminal you should be in ```~/gitTHERMOFUN/thermofun$``` and the directory structure should now look like this:

```
~/gitTHERMOFUN
    /build
        /debug
        /release
    /thermofun
```

## Install ArangoDB local

* For using ThermoFun with a local arangodb client. If only the remote db.thermohub.net database will be used, the installation of a local arangodb client is not necessary. Proceed to the next section Install Dependencies
* In a terminal copy-paste and run to flowing code: (for possibly newer versions of arangodb check [click here](https://www.arangodb.com/download-major/ubuntu/). In the arangodb packedge configuration we recommend to leave ```root``` password empty, and click enter for the following questions, using default selections. For Backup database files before upgrading select "Yes".

```
#!bash
sudo apt-get install curl && \
curl -OL https://download.arangodb.com/arangodb33/xUbuntu_17.04/Release.key && \
sudo apt-key add - < Release.key && \
echo 'deb https://download.arangodb.com/arangodb33/xUbuntu_17.04/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list && \
sudo apt-get install apt-transport-https && \
sudo apt-get update && \
sudo apt-get install arangodb3=3.3.19
```

## Build and install ThermoFun library 

Buil and ThermoFun library for use in C++ or python thirdparty codes.

### Install Dependencies

Installing dependencies needed to build ThermoFun on (k)ubuntu linux 16.04 or 18.04, in a terminal ```~/gitTHERMOFUN/thermofun$``` execute the following: 

```
#!bash
sudo ./install-dependencies.sh
```

### Compiling the C++ library

In the terminal ```~/gitTHERMOFUN/thermofun$```, execute the following commands:

```
#!bash
cd ../build/release && \
cmake ../../thermofun && \
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
cmake ../../thermofun -DCMAKE_INSTALL_PREFIX=/home/username/local/
```
then execute:

```
sudo make install 
```

To compile ThermoFun library in debug mode change directory to ```~/gitTHERMOFUN/build/debug``` and:

```
#!bash
cmake ../../thermofun/ -DCMAKE_BUILD_TYPE=Debug
```
then execute:

```
sudo make install 
```

### Build and run ThermoFun GUI Demo

To be able to build and run the ThemroFun GUI (graphical user interface) application demo, Qt needs to be installed.

* Download and install Qt 5.11.0 (https://www1.qt.io/download/) in your home directory ```~/Qt```. In the "Select components to install" menu select: Qt 5.11.0 with Desktop gcc 64-bit, Qt Charts, and Qt WebEngine

### Install Dependencies

Installing dependencies needed to build ThermoFun on (k)ubuntu linux 16.04 or 18.04, in the terminal go in ```~/gitTHERMOFUN/thermofun$``` and execute the following: 

```
#!bash
sudo ./install-dependencies-gui.sh $HOME/Qt/5.11.0/gcc_64
```

This step will download, configure, build, and install all dependencies: `lua5.3-dev`, `libboost-all-dev`, `libcurl4-openssl-dev`, `libboost-test-dev`, `automake`, `flex`, `bison`, `libssl-dev`, `pugixml`, `yaml-cpp`,  `thrift`, `velocypack`, `jsonio`, `jsonimpex`, `jsonui`. The script will check if the dependencies are not already present at the defalut instalation path ```/usr/local/``` and will only install them if not found. 
To reinstall dependencies open `install-dependencies.sh` and/or `install-dependencies-gui.sh` files with a text editor. At the beginning of the script file commands for removing dependency library files are present but they are commented out with `#` symbol. Remove `#` for each dependency you wish to be reinstalled. 

### Compiling the ThermoFun GUI demo

In the terminal at ```~/gitTHERMOFUN/thermofun$```, execute the following commands:

```
#!bash
cd .. && mkdir build-gui \
cd build-gui && mkdir release && cd release && \
cmake ../../thermofun/fungui -DBUILD_FUNGUI=ON -DBUILD_DEMO=ON -DCMAKE_PREFIX_PATH=$HOME/Qt/5.11.0/gcc_64 && \
make
``` 

The build script will also copy into the build folder the necessary /Resources folder. In the Resources folder a file named "ThermoFun-config.json" is present and contains the arangodb database connection preferences. 

To run the ThermoFun GUI demo in the terminal at ```~/gitTHERMOFUN/build-gui$``` execute:

```
./guidemo.sh
```

* For building using Qt Creator, use the ThermoFunDemoGUI.pro project file found in  ```~/gitTHERMOFUN/thermofun/fungui```.


### Automatic Test for comparing GEMS4 and ThermoFun calculations (OUTDATED - Under Construction / Update)

#### Build autoTest

Requires [CMake](http://www.cmake.org/) and [Qt5](http:/www.qt.io/) installed

* Let us call gitThermoFun and GEMS4R the directories where ThermoFun and gems4r repositories were cloned:
~~~
~/gitThermoFun
    /thermofun
    /gems4r
~~~

* In a terminal, run the following commands to clone the ThermoFun library:
~~~
$ cd ~/gitThermoFun/thermofun
$ git clone https://<your_user>@bitbucket.org/gems4/thermofun.git .
~~~

* In a terminal, run the following commands to clone the GEMS4R library:
~~~
$ cd ~/gitThermoFun/gems4r
$ git clone https://<your_user>@bitbucket.org/gems4/gems4r.git .
~~~

* In a linux terminal, cd inside ~/gitThermoFun/thermofun/tests/autoTest and type
~~~
$ ./install-thirdparty.sh
~~~

* If Qt5 libraries are installed locally (for instance in /home/your_user/Qt5/5.5/gcc64) then use the path to Qt libraries, as shown below:
~~~
$ ./install-thirdparty.sh /home/your_user/Qt5/5.5/gcc64
~~~

* This step will download, configure, build, and install all third-party libraries (bsonio, bsonui, EJDB, YAML-CPP, and pugixml) in build/{debug,release}/thirdparty.

* After this, headers and libraries of the third-party libraries can be found in build-auto-test/{debug,release}/thirdparty/{include,lib}. The .pro file of master project has already been adjusted to find these dependencies.

* Copy the Resources into the debug and release build folders 
~~~
~/gitThermoFun/thermofun/tests/autoTest/Resources
~~~

* Start QtCreator and configure autoTest (from: ~/gitThermoFun/thermofun/tests/autoTest/autoTest.pro) to build debug and release binaries respectively into
~~~
~/gitThermoFun/build-auto-test/debug
~/gitThermoFun/build-auto-test/release
~~~

* Now in QtCreator, build the *.pro project and then run autoTest code.

#### INPUT in the Test

* Resources/test_multi.VertexSubstance.json - list of substances exported from PMATCH++
* Resources/TestMulti - GEMS4R exported files containing the same sbustances as in the above list
* The test reads the list of substances exported from PMATCH++ and loads them in the TCorrPT internal data structure (Database)
* A GEMS node is initialized using the exported GEMS4R system files

#### OUTPUT from the Test

* writes warning messages in the terminal, if there is a relative difference between GEMS and TCorrPT calculated properties larger than tolerance = 1e-05
* writes calculation results in *.csv files. GEMS4 and TCorrPT calculated properties at different T and P  


### Simple API example (OUTDATED)

```
#!c++
int main(int argc, char const *argv[])
{
    Database database("database-name.json/xml/yaml");

    Thermo thermo(database);

    ThermoPropertiesSubstance tps;
    ThermoPropertiesReaction tpr;

    tps = thermo.thermoPropertiesSubstance("Substance Symbol", P, T);
    tpr = thermo.thermoPropertiesReaction("Reaction Symbol", P, T);

    return 0;
}
```

### What is this repository for? ###

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact