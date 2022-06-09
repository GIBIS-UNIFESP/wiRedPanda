include(../config.pri)

QT += testlib

CONFIG += testcase

TARGET = WPanda-test

DEFINES += CURRENTDIR=\\\"$$_PRO_FILE_PWD_\\\"

SOURCES += \
    testmain.cpp \
    testelements.cpp \
    testsimulationcontroller.cpp \
    testfiles.cpp \
    testcommands.cpp \
    testwaveform.cpp \
    testicons.cpp \
    testlogicelements.cpp

HEADERS += \
    testelements.h \
    testsimulationcontroller.h \
    testfiles.h \
    testcommands.h \
    testwaveform.h \
    testicons.h \
    testlogicelements.h
