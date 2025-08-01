# wiRedPanda Library Sources
# This file contains all the shared application sources that should be compiled once
# and linked by both the main app and test executable

include(../app/element/element.pri)
include(../app/logicelement/logicelement.pri)

INCLUDEPATH += \
    $$PWD/../app \
    $$PWD/../app/arduino \
    $$PWD/../app/element \
    $$PWD/../app/logicelement \
    $$PWD/../app/nodes

SOURCES += \
    $$PWD/../app/application.cpp \
    $$PWD/../app/arduino/codegenerator.cpp \
    $$PWD/../app/bewaveddolphin.cpp \
    $$PWD/../app/clockdialog.cpp \
    $$PWD/../app/commands.cpp \
    $$PWD/../app/common.cpp \
    $$PWD/../app/elementeditor.cpp \
    $$PWD/../app/elementfactory.cpp \
    $$PWD/../app/elementlabel.cpp \
    $$PWD/../app/elementmapping.cpp \
    $$PWD/../app/enums.cpp \
    $$PWD/../app/graphicelement.cpp \
    $$PWD/../app/graphicsview.cpp \
    $$PWD/../app/ic.cpp \
    $$PWD/../app/itemwithid.cpp \
    $$PWD/../app/lengthdialog.cpp \
    $$PWD/../app/logicelement.cpp \
    $$PWD/../app/mainwindow.cpp \
    $$PWD/../app/nodes/qneconnection.cpp \
    $$PWD/../app/nodes/qneport.cpp \
    $$PWD/../app/recentfiles.cpp \
    $$PWD/../app/scene.cpp \
    $$PWD/../app/serialization.cpp \
    $$PWD/../app/settings.cpp \
    $$PWD/../app/simulation.cpp \
    $$PWD/../app/simulationblocker.cpp \
    $$PWD/../app/thememanager.cpp \
    $$PWD/../app/trashbutton.cpp \
    $$PWD/../app/workspace.cpp

HEADERS += \
    $$PWD/../app/application.h \
    $$PWD/../app/arduino/codegenerator.h \
    $$PWD/../app/bewaveddolphin.h \
    $$PWD/../app/clockdialog.h \
    $$PWD/../app/commands.h \
    $$PWD/../app/common.h \
    $$PWD/../app/elementeditor.h \
    $$PWD/../app/elementfactory.h \
    $$PWD/../app/elementlabel.h \
    $$PWD/../app/elementmapping.h \
    $$PWD/../app/enums.h \
    $$PWD/../app/globalproperties.h \
    $$PWD/../app/graphicelement.h \
    $$PWD/../app/graphicelementinput.h \
    $$PWD/../app/graphicsview.h \
    $$PWD/../app/ic.h \
    $$PWD/../app/itemwithid.h \
    $$PWD/../app/lengthdialog.h \
    $$PWD/../app/logicelement.h \
    $$PWD/../app/mainwindow.h \
    $$PWD/../app/nodes/qneconnection.h \
    $$PWD/../app/nodes/qneport.h \
    $$PWD/../app/recentfiles.h \
    $$PWD/../app/scene.h \
    $$PWD/../app/serialization.h \
    $$PWD/../app/settings.h \
    $$PWD/../app/simulation.h \
    $$PWD/../app/simulationblocker.h \
    $$PWD/../app/thememanager.h \
    $$PWD/../app/trashbutton.h \
    $$PWD/../app/workspace.h

FORMS += \
    $$PWD/../app/bewaveddolphin.ui \
    $$PWD/../app/clockdialog.ui \
    $$PWD/../app/elementeditor.ui \
    $$PWD/../app/lengthdialog.ui \
    $$PWD/../app/mainwindow.ui

RESOURCES += \
    $$PWD/../app/resources/basic/basic.qrc \
    $$PWD/../app/resources/dolphin/dolphin.qrc \
    $$PWD/../app/resources/input/input.qrc \
    $$PWD/../app/resources/memory/dark/memory_dark.qrc \
    $$PWD/../app/resources/memory/light/memory_light.qrc \
    $$PWD/../app/resources/misc/misc.qrc \
    $$PWD/../app/resources/output/output.qrc \
    $$PWD/../app/resources/toolbar/toolbar.qrc \
    $$PWD/../app/resources/translations/translations.qrc

