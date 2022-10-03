include(../config.pri)

QT += testlib

CONFIG += testcase

TARGET = WPanda-test

DEFINES += CURRENTDIR=\\\"$$_PRO_FILE_PWD_\\\"

SOURCES += \
    testmain.cpp \
    testelements.cpp \
    testfiles.cpp \
    testcommands.cpp \
    testsimulation.cpp \
    testwaveform.cpp \
    testicons.cpp \
    testlogicelements.cpp

HEADERS += \
    testelements.h \
    testfiles.h \
    testcommands.h \
    testsimulation.h \
    testwaveform.h \
    testicons.h \
    testlogicelements.h
