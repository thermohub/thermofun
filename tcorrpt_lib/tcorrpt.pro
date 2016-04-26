##########################################################################################
#      qmake project file for generation of a dynamically linked TCORRPT library         #
#  configure release version:	            qmake "CONFIG += release" tcorrpt.pro        #
#  configure debug version:	            qmake "CONFIG += debug" tcorrpt.pro          #
#  Project created by QtCreator 2016-04-13T13:12:21 D. Miron                             #
##########################################################################################

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= tcorrpt
VERSION		= 1.0.0

CONFIG		-= qt
CONFIG		+= warn_on
CONFIG		+= console

QMAKE_CC	= gcc
QMAKE_CXX	= g++


  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CFLAGS += -std=g++11

DEFINES       += TCORRPT_LIBRARY

!win32 {
  DEFINES += __unix
}

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}

TCORRPT_CPP = ../src
TCORRPT_H   = $$TCORRPT_CPP

DEPENDPATH += $$TCORRPT_H

INCLUDEPATH +=
INCLUDEPATH += .
INCLUDEPATH += $$TCORRPT_H

QMAKE_LFLAGS +=
OBJECTS_DIR = obj

include($$TCORRPT_CPP/tcorrpt.pri)


