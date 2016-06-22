TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#QMAKE_CXXFLAGS += -std=c++1y
QMAKE_CXXFLAGS += -std=c++11

#CONFIG += -std=c++11

TARGET = HKF-test

TCORRPT_TEST_CPP = ./src
TCORRPT_CPP   =  ../../../tcorrpt/src

TCORRPT_TEST_H   = $$TCORRPT_TEST_CPP
TCORRPT_H     =   $$TCORRPT_CPP

DEPENDPATH   += $$TCORRPT_TEST_H
DEPENDPATH   += $$TCORRPT_H

INCLUDEPATH  += $$TCORRPT_TEST_H
INCLUDEPATH  += $$TCORRPT_H

QMAKE_LFLAGS +=
OBJECTS_DIR = obj

# Define the directory where the gui, third-party libs, resources are located
BUILD_DIR = $$OUT_PWD/..
THRIFT_DIR    = ./thrift

# Define the directory where the third-party libraries have been installed
#THIRDPARTY_DIR = $$BUILD_DIR/thirdparty/debug
CONFIG(release, debug|release): THIRDPARTY_DIR = $$BUILD_DIR/release/thirdparty
CONFIG(debug, debug|release): THIRDPARTY_DIR = $$BUILD_DIR/debug/thirdparty
# Define the directories where the headers of the third-party libraries have been installed
THIRDPARTY_INCLUDE_DIR = $$THIRDPARTY_DIR/include

# Define the directories where the THIRDPARTY libraries have been installed
THIRDPARTY_LIBRARY_DIR1 = $$THIRDPARTY_DIR/lib
THIRDPARTY_LIBRARY_DIR2 = $$THIRDPARTY_DIR/lib/x86_64-linux-gnu

DEPENDPATH    += $$THIRDPARTY_INCLUDE_DIR
INCLUDEPATH   += $$THIRDPARTY_INCLUDE_DIR

LIBS += -L$$THIRDPARTY_LIBRARY_DIR1
LIBS += -L$$THIRDPARTY_LIBRARY_DIR2
LIBS += -lbsonio -lyaml-cpp -lejdb -lpugixml
LIBS += -lthrift

include($$TCORRPT_TEST_CPP/tcorrpt-test.pri)
include($$TCORRPT_CPP/tcorrpt.pri)


