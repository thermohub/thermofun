#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T16:01:53
#
#-------------------------------------------------

TARGET = ThermoFunDemoGUI
TEMPLATE = app

CONFIG += thread
CONFIG += c++11

QT  += widgets svg
QT  += core gui printsupport concurrent
QT  += charts webenginewidgets
#QT  += webkitwidgets

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
  LIBS += -llua
}

INCLUDEPATH   += "/usr/local/include"
DEPENDPATH   += "/usr/local/include"
#LIBPATH += "/usr/local/lib/"

# Define the directory where source code is located
#THRIFT_DIR    = ./thrift
FUN_DIR       = ./src
FUN_CPP       = $$FUN_DIR
FUN_H         = $$FUN_DIR
GUI_DIR       = ./DemoGui
GUI_CPP       = $$GUI_DIR
GUI_H         = $$GUI_DIR
#GUI_CPP       =  ./GUI
#GUI_H         =  $$GUI_CPP

DEPENDPATH   += $$GUI_H
DEPENDPATH   += $$GUI_DIR

INCLUDEPATH   += $$GUI_H
INCLUDEPATH   += $$GUI_DIR

DEPENDPATH   += $$FUN_H
DEPENDPATH   += $$FUN_DIR

INCLUDEPATH   += $$FUN_H
INCLUDEPATH   += $$FUN_DIR

#temporary
BUILD_DIR = $$OUT_PWD/..
#THIRDPARTY_DIR = $$BUILD_DIR/debug/thirdparty/thirdparty
#THERMOFUN_SRC =  $$THIRDPARTY_DIR/src/THERMOFUN/src
#DEPENDPATH   += $$THERMOFUN_SRC
#INCLUDEPATH   += $$THERMOFUN_SRC
#include($$THERMOFUN_SRC/ThermoFun.pri)

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


LIBS += -ljsonui -ljsonio -lyaml-cpp -lpugixml
#LIBS += -lthrift -lboost_regex
#LIBS += -lthermofun
!macx-clang:LIBS += -llua5.3
#LIBS += -lapp-static -lhunspell -lmarkdown
#LIBS += -lfuerte
LIBS += -lpthread -lssl -lcrypto -lcurl -lboost_thread -lvelocypack

MOC_DIR = tmp
UI_DIR        = $$MOC_DIR
UI_SOURSEDIR  = $$MOC_DIR
UI_HEADERDIR  = $$MOC_DIR
OBJECTS_DIR   = obj

#include($$THRIFT_DIR/thrift.pri)
#include($$GEN_DIR/generator.pri)
include($$GUI_DIR/ThermoFunGUI.pri)
include($$FUN_DIR/ThermoFun.pri)

RESOURCES += \
    ThermoFunDemoGUI.qrc

#thrift -r -v --gen cpp
