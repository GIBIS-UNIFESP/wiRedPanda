
include(../includes.pri)

QT += testlib
TARGET = WPanda-test
SOURCES += \
      main.cpp \
    testelements.cpp \
    testsimulationcontroller.cpp \
    testfiles.cpp

HEADERS += \
    testelements.h \
    testsimulationcontroller.h \
    testfiles.h

DEFINES += CURRENTDIR=\\\"$$PWD\\\"
