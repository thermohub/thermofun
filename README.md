# ThermoFun #

A code for calculating the standard state thermodynamic properties at a given temperature and pressure.

## Build and Run Tests on Linux ##

#### Install Apache Thrift ####

* Make sure you have cmake installed. If not, install it (on ubuntu linux):
~~~
sudo apt-get install cmake
~~~

* Install Apache Thrift v.0.9.1 (on ubuntu linux) - open the terminal and run the command:
~~~
sudo apt-get install thrift-compiler
~~~

#### Install Lua

* Install Lua embedded scripts interpreter (on ubuntu linux):
~~~
sudo apt-get install lua5.2 lua5.2-dev
~~~

### Automatic Test for comparing GEMS4 and ThermoFun calculations ###

#### Build autoTest

Requires [CMake](http://www.cmake.org/) and [Qt5](http:/www.qt.io/) installed

* Let’s call gitThermoFun and GEMS4R the directories where ThermoFun and gems4r repositories were cloned:
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

This step will download, configure, build, and install all third-party libraries (bsonio, bsonui, EJDB, YAML-CPP, and pugixml) in build/{debug,release}/thirdparty.

After this, headers and libraries of the third-party libraries can be found in build-auto-test/{debug,release}/thirdparty/{include,lib}. The .pro file of master project has already been adjusted to find these dependencies.

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


### Simple API example ###

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