QT       += core gui printsupport charts multimedia widgets
VERSION = 2.6.1

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++11

CONFIG(debug, debug|release) {
    CONFIG += console
}

*-g++{
#    QMAKE_CXXFLAGS *= -Wall -Wextra -Wpedantic -Wfloat-equal -Wnarrowing
#    QMAKE_CXXFLAGS *= -Wnull-dereference -Wold-style-cast -Wdouble-promotion -Wformat=2 -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wshadow=local

    QMAKE_CXXFLAGS += -Wl,-rpath,'${ORIGIN}/lib'
}

*-clang{
#    QMAKE_CXXFLAGS *= -Weverything -Wno-reserved-id-macro -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-undef -Wno-padded -Wno-sign-conversion -Wno-deprecated -Wno-covered-switch-default
#    QMAKE_CXXFLAGS *= -Wno-undefined-reinterpret-cast -Wno-weak-vtables -Wno-exit-time-destructors -Wno-used-but-marked-unused -Wno-inconsistent-missing-destructor-override -Wno-documentation-unknown-command
#    QMAKE_CXXFLAGS *= -Wno-shift-sign-overflow -Wno-non-virtual-dtor -Wno-conversion -Wno-global-constructors -Wno-switch-enum -Wno-missing-prototypes -Wno-shadow-field-in-constructor
#    QMAKE_CXXFLAGS *= -Wno-shadow -Wno-shadow-field
}

#mac {
#TEMPLATE = app
#QMAKE_MAC_SDK = macosx10.10
#QMAKE_LFLAGS += -mmacosx-version-min=10.10 -v
#QMAKE_CXXFLAGS += -stdlib=libc++
#QMAKE_CXXFLAGS += -mmacosx-version-min=10.10
#QMAKE_CXXFLAGS_DEBUG = -O
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
#LIBS += -stdlib=libc++
#CONFIG += app_bundle
#}

QMAKE_CXXFLAGS_DEBUG += -DDEBUG=1 -Wall

include($$PWD/app/element/element.pri)
include($$PWD/app/logicelement/logicelement.pri)

SOURCES += \
    $$PWD/app/arduino/codegenerator.cpp \
    $$PWD/app/elementeditor.cpp \
    $$PWD/app/elementfactory.cpp \
    $$PWD/app/box.cpp \
    $$PWD/app/boxnotfoundexception.cpp \
    $$PWD/app/commands.cpp \
    $$PWD/app/editor.cpp \
    $$PWD/app/globalproperties.cpp \
    $$PWD/app/graphicelement.cpp \
    $$PWD/app/graphicsview.cpp \
    $$PWD/app/graphicsviewzoom.cpp \
    $$PWD/app/label.cpp \
    $$PWD/app/listitemwidget.cpp \
    $$PWD/app/mainwindow.cpp \
    $$PWD/app/nodes/qneconnection.cpp \
    $$PWD/app/nodes/qneport.cpp \
    $$PWD/app/recentfilescontroller.cpp \
    $$PWD/app/scene.cpp \
    $$PWD/app/serializationfunctions.cpp \
    $$PWD/app/simulationcontroller.cpp \
    $$PWD/app/itemwithid.cpp \
    $$PWD/app/simplewaveform.cpp \
    $$PWD/app/thememanager.cpp \
    $$PWD/app/logicelement.cpp \
    $$PWD/app/boxmanager.cpp \
    $$PWD/app/boxprototype.cpp \
    $$PWD/app/boxfilehelper.cpp \
    $$PWD/app/boxprototypeimpl.cpp \
    $$PWD/app/elementmapping.cpp \
    $$PWD/app/boxmapping.cpp \
    $$PWD/app/common.cpp

HEADERS  +=  \
    $$PWD/app/common.h \
    $$PWD/app/graphicsviewzoom.h \
    $$PWD/app/arduino/codegenerator.h\
    $$PWD/app/box.h \
    $$PWD/app/boxnotfoundexception.h \
    $$PWD/app/commands.h \
    $$PWD/app/editor.h \
    $$PWD/app/elementeditor.h \
    $$PWD/app/elementfactory.h \
    $$PWD/app/globalproperties.h \
    $$PWD/app/graphicelement.h \
    $$PWD/app/graphicsview.h \
    $$PWD/app/label.h \
    $$PWD/app/listitemwidget.h \
    $$PWD/app/mainwindow.h \
    $$PWD/app/nodes/qneconnection.h \
    $$PWD/app/nodes/qneport.h \
    $$PWD/app/recentfilescontroller.h \
    $$PWD/app/scene.h \
    $$PWD/app/serializationfunctions.h \
    $$PWD/app/simulationcontroller.h \
    $$PWD/app/itemwithid.h \
    $$PWD/app/simplewaveform.h \
    $$PWD/app/thememanager.h \
    $$PWD/app/logicelement.h \
    $$PWD/app/boxmanager.h \
    $$PWD/app/boxprototype.h \
    $$PWD/app/boxfilehelper.cpp \
    $$PWD/app/boxprototypeimpl.h \
    $$PWD/app/elementmapping.h \
    $$PWD/app/boxmapping.h \

INCLUDEPATH += \
    $$PWD/app \
    $$PWD/app/element \
    $$PWD/app/nodes

FORMS    += \
    $$PWD/app/mainwindow.ui \
    $$PWD/app/elementeditor.ui \
    $$PWD/app/simplewaveform.ui

RESOURCES += \
    $$PWD/app/resources/resources.qrc\
    $$PWD/app/resources/basic/basic.qrc\
    $$PWD/app/resources/input/input.qrc\
    $$PWD/app/resources/output/output.qrc\
    $$PWD/app/resources/toolbar/toolbar.qrc \
    $$PWD/app/resources/memory/dark/memory_dark.qrc \
    $$PWD/app/resources/memory/light/memory_light.qrc

RESOURCES += $$PWD/app/resources/translations.qrc

TRANSLATIONS = $$PWD/app/resources/wpanda_en.ts  $$PWD/app/resources/wpanda_pt.ts

DISTFILES += \
    $$PWD/uncrustify.cfg
