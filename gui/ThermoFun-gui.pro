#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T16:01:53
#
#-------------------------------------------------

TARGET = ThermoFun-gui
TEMPLATE = app

CONFIG += thread
CONFIG += c++11

QT  += widgets svg
QT  += core gui printsupport concurrent

!win32 {
  DEFINES += __unix
}

macx-g++ {
  DEFINES += __APPLE__
}

macx-clang {
  DEFINES += __APPLE__
  CONFIG -= warn_on
  CONFIG += warn_off
  INCLUDEPATH   += "/usr/local/include"
  DEPENDPATH   += "/usr/local/include"
  LIBPATH += "/usr/local/lib/"
  LIBS += -llua
}
else {

# Define the directory where the gui, third-party libs, resources are located
BUILD_DIR = $$OUT_PWD/..

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
}

# Define the directory where source code is located
GUI_CPP   =    ./src
ThermoFun_CPP =  ../src
GUI_H     =  $$GUI_CPP
ThermoFun_H   =  $$ThermoFun_CPP

DEPENDPATH   += $$GUI_H
DEPENDPATH   += $$CORPT_H
DEPENDPATH   += $$THIRDPARTY_INCLUDE_DIR

INCLUDEPATH   += $$GUI_H
INCLUDEPATH   += $$CORPT_H
INCLUDEPATH   += $$THIRDPARTY_INCLUDE_DIR

LIBS += -lbsonui -lbsonio -lyaml-cpp -lejdb -lpugixml -lqwt
LIBS += -lthrift -lboost_regex
LIBS += -lthermofun
!macx-clang:LIBS += -llua5.2

MOC_DIR = tmp
UI_DIR        = $$MOC_DIR
UI_SOURSEDIR  = $$MOC_DIR
UI_HEADERDIR  = $$MOC_DIR
OBJECTS_DIR   = obj

include($$GUI_H/gui.pri)
#include($$ThermoFun_H/ThermoFun.pri)

RESOURCES += \
    ThermoFun.qrc

SOURCES += \
    test/main.cpp \
    test/TestMainWindow.cpp

HEADERS += \
    test/TestMainWindow.h

FORMS += \
    test/TestMainWindow.ui

