TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

#QMAKE_CXXFLAGS += -std=c++1y
QMAKE_CXXFLAGS += -std=c++11

CONFIG += c++11
QT += widgets

TARGET = interfaceTest

DEFINES += IPMGEMPLUGIN
#DEFINES += NODEARRAYLEVEL
DEFINES += NOPARTICLEARRAY

ThermoFun_TEST_CPP = ./src
ThermoFun_CPP   =  ../../../thermofun/src

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

LIBS += -ljsonio -lyaml-cpp -lejdb -lpugixml
#-lReaktoro
LIBS += -lthrift -lboost_regex -lboost_system -lboost_filesystem
LIBS +=  -lcurl  -lvelocypack
!macx-clang:LIBS += -llua5.2
macx-clang:LIBS += -llua

include($$ThermoFun_TEST_CPP/ThermoFun-test.pri)
include($$ThermoFun_CPP/ThermoFun.pri)


