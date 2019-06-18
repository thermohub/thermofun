TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt

#QMAKE_CXXFLAGS += -std=c++1y
#QMAKE_CXXFLAGS += -std=c++11

#CONFIG += -std=c++11
CONFIG += c++11

TARGET = dbclientTest

DEFINES += FROM_SRC

#DEFINES += IPMGEMPLUGIN
##DEFINES += NODEARRAYLEVEL
#DEFINES += NOPARTICLEARRAY

ThermoFun_TEST_CPP = ./src
ThermoDBClient_CPP = ../../DBClient
ThermoFun_CPP   =  ../../thermofun

ThermoFun_TEST_H   = $$ThermoFun_TEST_CPP
ThermoDBClient_H = $$ThermoDBClient_CPP
ThermoFun_H     =   $$ThermoFun_CPP

DEPENDPATH   += $$ThermoFun_TEST_H
DEPENDPATH   += $$ThermoFun_H
DEPENDPATH   += $$ThermoDBClient_H

INCLUDEPATH  += $$ThermoFun_TEST_H
INCLUDEPATH  += $$ThermoFun_H
INCLUDEPATH  += $$ThermoDBClient_H

QMAKE_LFLAGS +=
OBJECTS_DIR = obj

# Define the directory where the gui, third-party libs, resources are located
BUILD_DIR = $$OUT_PWD/..

#INCLUDEPATH   += "/usr/local/include"
#DEPENDPATH   += "/usr/local/include"
#LIBPATH += "/usr/local/lib/"

# Define the directory where the third-party libraries have been installed
#THIRDPARTY_DIR = $$BUILD_DIR/thirdparty/debug
CONFIG(release, debug|release): THIRDPARTY_DIR = $$BUILD_DIR/release/thirdparty
CONFIG(debug, debug|release): THIRDPARTY_DIR = $$BUILD_DIR/debug/thirdparty
# Define the directories where the headers of the third-party libraries have been installed
THIRDPARTY_INCLUDE_DIR = $$THIRDPARTY_DIR/include
# Define the directories where the THIRDPARTY libraries have been installed
THIRDPARTY_LIBRARY_DIR1 = $$THIRDPARTY_DIR/lib

DEPENDPATH   += $$THIRDPARTY_INCLUDE_DIR
INCLUDEPATH   += $$THIRDPARTY_INCLUDE_DIR
LIBS += -L$$THIRDPARTY_LIBRARY_DIR1


LIBS += -ljsonio
#-lReaktoro
LIBS += -lboost_regex -lboost_system -lboost_filesystem
LIBS +=  -lcurl  -lvelocypack

include($$ThermoFun_TEST_CPP/ThermoFun-test.pri)
include($$ThermoFun_CPP/ThermoFun.pri)
include($$ThermoDBClient_CPP/ThermoDBClient.pri)
#include(Modules/GEMS4R/GEMS4R.pri)


