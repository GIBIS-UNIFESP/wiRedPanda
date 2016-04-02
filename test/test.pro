
include(../includes.pri)

QT += testlib
TARGET = WPanda-test
SOURCES += \
      main.cpp \
    testelements.cpp \
    testsimulationcontroller.cpp

HEADERS += \
    testelements.h \
    testsimulationcontroller.h

DEFINES += CURRENTDIR=\\\"$$PWD\\\"
