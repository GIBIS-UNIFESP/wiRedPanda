#-------------------------------------------------
#
# Project created by QtCreator 2014-10-22T13:17:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WPanda
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    label.cpp \
    editor.cpp \
    graphicelement.cpp \
    nodes/qneport.cpp \
    nodes/qneconnection.cpp \
    nodes/qneblock.cpp \
    elementfactory.cpp \
    element/inputbutton.cpp \
    element/led.cpp \
    element/and.cpp \
    element/or.cpp \
    element/clock.cpp \
    element/inputswitch.cpp

HEADERS  += mainwindow.h \
    label.h \
    editor.h \
    graphicelement.h \
    nodes/qneport.h \
    nodes/qneconnection.h \
    nodes/qneblock.h \
    elementfactory.h \
    element/inputbutton.h \
    element/led.h \
    element/and.h \
    element/or.h \
    element/clock.h \
    element/inputswitch.h

FORMS    += mainwindow.ui

mac {
TEMPLATE = app
QMAKE_MAC_SDK = macosx10.10
QMAKE_LFLAGS += -mmacosx-version-min=10.10 -v
QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
QMAKE_CXXFLAGS_DEBUG = -O
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
LIBS += -stdlib=libc++
CONFIG += app_bundle
CONFIG += c++11
}

RESOURCES += \
    resources.qrc
