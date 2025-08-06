include(../config.pri)

# Ensure this project is built through the main WPanda.pro subdirs project
!exists($$OUT_PWD/../lib) {
    error("This project must be built through WPanda.pro, not directly. The static library dependency is missing.")
}

# Link with the static library instead of including sources  
LIBS += -L$$OUT_PWD/../lib -lwiredpanda_lib

# macOS test-specific settings
mac {
    greaterThan(QT_MAJOR_VERSION, 5): QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

QT += testlib

CONFIG += testcase

TARGET = wiredpanda-test

DEFINES += CURRENTDIR=\"$$_PRO_FILE_PWD_\"

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
