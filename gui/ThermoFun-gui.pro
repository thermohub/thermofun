#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T16:01:53
#
#-------------------------------------------------

TARGET = thermofun-gui
TEMPLATE = app

CONFIG += thread
CONFIG += c++11

QT  += widgets svg webkitwidgets
QT  += core gui printsupport concurrent
DEFINES += DBCLIENT_H

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
LIBPATH += "/usr/local/lib/"

# Define the directory where source code is located
GUI_CPP   =    ./src
#ThermoFun_CPP =  ../src
GUI_H     =  $$GUI_CPP
#ThermoFun_H   =  $$ThermoFun_CPP

DEPENDPATH   += $$GUI_H
#DEPENDPATH   += $$CORPT_H

INCLUDEPATH   += $$GUI_H
#INCLUDEPATH   += $$CORPT_H

LIBS += -lbsonui -lbsonio -lyaml-cpp -lejdb -lpugixml -lqwt
LIBS += -lthrift -lboost_regex
LIBS += -lthermofun
!macx-clang:LIBS += -llua5.2
LIBS += -lapp-static -lhunspell -lmarkdown

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

