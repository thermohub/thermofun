TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

#QMAKE_CXXFLAGS += -std=c++1y
QMAKE_CXXFLAGS += -std=c++11

CONFIG += c++11

TARGET = interfaceTest

DEFINES += IPMGEMPLUGIN
#DEFINES += NODEARRAYLEVEL
DEFINES += NOPARTICLEARRAY

ThermoFun_TEST_CPP = ./src
ThermoFun_CPP   =  ../../ThermoFun

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

LIBS += -lChemicalFun

INCLUDEPATH   += "/home/dmiron/miniconda3/envs/thermofun/include"
DEPENDPATH   += "/home/dmiron/miniconda3/envs/thermofun/include"
LIBPATH += "/home/dmiron/miniconda3/envs/thermofun/lib/"



include($$ThermoFun_TEST_CPP/ThermoFun-test.pri)
include($$ThermoFun_CPP/ThermoFun.pri)



unix:!macx: LIBS += -L$$PWD/../../../../../miniconda3/envs/thermofun/lib/ -lfmt

INCLUDEPATH += $$PWD/../../../../../miniconda3/envs/thermofun/include
DEPENDPATH += $$PWD/../../../../../miniconda3/envs/thermofun/include
