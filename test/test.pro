
include(../includes.pri)

QT += testlib
TARGET = WPanda-test
SOURCES += \
      main.cpp \
      testpriorityqueue.cpp \
    testelements.cpp \
    testsimulationcontroller.cpp

HEADERS += \
      testpriorityqueue.h \
    testelements.h \
    testsimulationcontroller.h

DEFINES += CURRENTDIR=\\\"$$PWD\\\"
