# ThermoFun

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

## Build and Run Tests on Linux

## Pepare building tools

* Make sure you have g++, cmake and git installed. If not, install them (on ubuntu linux):
~~~
sudo apt-get install g++ cmake git
~~~

## Prepare folder structure and download ThermoFun source code

* The directory structure should now look like this:
~~~
~/gitTHERMOFUN
    /build
        /debug
        /release
    /thermofun
~~~

* In a terminal, run the following commands to clone the ThermoFun library:
~~~
$ cd ~/gitTHERMOFUN/thermofun
$ git clone https://bitbucket.org/gems4/thermofun.git .
~~~

## Install additional packages

* Install additional packages required by JSONIO library, which is used for operating with data and communicating with the local or remote ArangoDB database.

~~~
$ sudo apt-get install libboost-all-dev curl libcurl4-openssl-dev
~~~

* ThermoFun (via JSONIO) uses ArangoDB database with which it communicates with VelocyPack serializer. First install current version of ArangoDB server locally [from here](https://www.arangodb.com/download-major/ubuntu/):

~~~
curl -OL https://download.arangodb.com/arangodb33/xUbuntu_16.04/Release.key
sudo apt-key add - < Release.key
echo 'deb https://download.arangodb.com/arangodb33/xUbuntu_16.04/ /' | sudo tee /etc/apt/sources.list.d/arangodb.list
sudo apt-get install apt-transport-https
sudo apt-get update
sudo apt-get install arangodb3=3.3.8
~~~

Then build ArangoDB VelocyPack serializer as follows:

~~~
sudo apt-get update
cd ~
mkdir -p code && \
    cd code && \
    git clone https://github.com/arangodb/velocypack.git && \
    cd velocypack && \
    mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_CXX_FLAGS=-fPIC && \
    sudo make install
~~~

## Build ThermoFun library (release)

* For compiling ThermoFun library from its source code change folder to `~/gitTHERMOFUN/build/release` and execute:

~~~
cmake ../../thermofun/ -DCMAKE_BUILD_TYPE=Release
~~~

This will start the building process, first the JSONIO third party library, followed by the ThermoFun library. 

* For a global installation of the compiled libraries in your system, execute:

~~~
make install
~~~

This will install ThermoFun's header files and libraries in the default installation directory of your system (e.g, /usr/local/ or /opt/local/). 
Note that this installation mode might require administrator rights, so that you would need to execute 
~~~
sudo make install
~~~
instead.

* For a local installation, you can specify a directory path for the installed files as:

~~~
cmake .. -DCMAKE_INSTALL_PREFIX=/home/username/local/
make install
~~~

The above call to cmake will reconfigure the build process, but it will not require recompilation if ThermoFun's libraries have already been compiled.


### Build and run ThermoFun GUI Demo

To be able to build and run the ThemroFun GUI (graphical user interface) application demo, Qt needs to be installed.

* Download and install Qt 5.11.0 (https://www1.qt.io/download/). In the "Select components to install" menu select: Qt 5.11.0 with Desktop gcc 64-bit, Qt Charts, and Qt WebEngine

* in addition libgl-dev is required
~~~
$ sudo apt-get install libgl-dev
~~~

ThermoFun GUI demo uses the JSONUI as a thrid party library (https://bitbucket.org/gems4/jsonui) 

* Before installing the thirparty libraries, Apache Thrift and Lua need to be installed

* Install the Lua embedded scripts interpreter (on ubuntu linux):
~~~
sudo apt-get install lua5.3 lua5.3-dev
~~~

* Before building Apache Thrift
~~~
sudo apt-get install libssl-dev libtool byacc automake bison flex pkg-config libboost-all-dev
~~~

* Build and install the 0.11.0 version of the Apache Thrift by cloning it with git:
~~~
sudo apt-get install libssl-dev libtool byacc automake bison flex pkg-config libboost-all-dev

sudo apt-get install libssl-dev libtool byacc flex
sudo apt-get install libboost-dev libboost-test-dev libboost-program-options-dev libboost-filesystem-dev 
sudo apt-get install libboost-thread-dev libevent-dev automake libtool flex bison pkg-config g++ libssl-dev
sudo apt-get install libboost-all-dev

cd ~
mkdir thrift
cd thrift
git clone http://github.com/apache/thrift . -b 0.11.0
./bootstrap.sh
./configure --without-lua
sudo make install
sudo ldconfig
~~~

* To install the thirdparty libraries in a linux terminal, cd inside `~/gitTHERMOFUN/thermofun/DemoGUI` and type

~~~
$ ./install-thirdparty.sh /home/your_user/Qt/5.11.0/gcc_64
~~~

* This step will download, configure, build, and install all third-party libraries (JSONIO, JSONUI, JSONIMPEX, YAML-CPP, and pugixml) into build/{debug,release}/thirdparty. The build script will check if the libraries are already installed and only build and install them if not found in  /usr/local/. If the thirdparty libraries need to be refreshed/updated in the file `install-thirdparty.sh` change `-DREFRESH_THIRDPARTY=OFF` to `-DREFRESH_THIRDPARTY=ON`

* Now in QtCreator, build the ThermoFunDemoGUI.pro project using the same Qt version as was used for building thirdpary (i.e. Qt 5.11.0) and then run the DemoGUI code. 

* Before running DemoGUI copy the Resources folder found in thermofun/DemoGUI in the build folder

The ThermoFunDemoGUI.json file contains the settings for connecting to the local or remote ArangoDB server. To use the local server set `"CurrentDBConnection" :   1` to use the remote server set `"CurrentDBConnection" :   0`

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