TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

#QMAKE_CXXFLAGS += -std=c++1y
QMAKE_CXXFLAGS += -std=c++11

#CONFIG += -std=c++11

TARGET = dbclientTest

#DEFINES += IPMGEMPLUGIN
##DEFINES += NODEARRAYLEVEL
#DEFINES += NOPARTICLEARRAY

ThermoFun_TEST_CPP = ./src
ThermoFun_CPP   =  ../../src

ThermoFun_TEST_H   = $$ThermoFun_TEST_CPP
ThermoFun_H     =   $$ThermoFun_CPP

DEPENDPATH   += $$ThermoFun_TEST_H
DEPENDPATH   += $$ThermoFun_H

INCLUDEPATH  += $$ThermoFun_TEST_H
INCLUDEPATH  += $$ThermoFun_H

QMAKE_LFLAGS +=
OBJECTS_DIR = obj

# Define the directory where the gui, third-party libs, resources are located
BUILD_DIR = $$OUT_PWD/..
THRIFT_DIR    = ./thrift

INCLUDEPATH   += "/usr/local/include"
DEPENDPATH   += "/usr/local/include"
LIBPATH += "/usr/local/lib/"

LIBS += -lbsonio -lyaml-cpp -lejdb -lpugixml
#-lReaktoro
LIBS += -lthrift -lboost_regex
LIBS += -llua5.2

include($$ThermoFun_TEST_CPP/ThermoFun-test.pri)
include($$ThermoFun_CPP/ThermoFun.pri)
#include(Modules/GEMS4R/GEMS4R.pri)


