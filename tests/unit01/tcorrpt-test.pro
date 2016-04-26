TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

TARGET = unit01

TCORRPT_TEST_CPP = ./src
TCORRPT_CPP   =  ../../../tcorrpt/src

TCORRPT_TEST_H   = $$TCORRPT_TEST_CPP
TCORRPT_H     =   $$TCORRPT_CPP

DEPENDPATH   += $$TCORRPT_TEST_H
DEPENDPATH   += $$TCORRPT_H

INCLUDEPATH += $$TCORRPT_TEST_H
INCLUDEPATH  += $$TCORRPT_H

QMAKE_LFLAGS +=
OBJECTS_DIR = obj

include($$TCORRPT_TEST_CPP/tcorrpt-test.pri)
include($$TCORRPT_CPP/tcorrpt.pri)


