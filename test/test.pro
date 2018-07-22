include(../includes.pri)

QT += testlib

TARGET = WPanda-test

SOURCES += \
    main.cpp \
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

DEFINES += CURRENTDIR=\\\"$$_PRO_FILE_PWD_\\\"
