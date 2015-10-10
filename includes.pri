QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG += c++11
CONFIG(debug, debug|release) {
    CONFIG += console
}


SOURCES += \
    $$PWD/app/box.cpp \
    $$PWD/app/editor.cpp \
    $$PWD/app/element/and.cpp \
    $$PWD/app/element/clock.cpp \
    $$PWD/app/element/dflipflop.cpp \
    $$PWD/app/element/display.cpp \
    $$PWD/app/element/dlatch.cpp \
    $$PWD/app/elementeditor.cpp \
    $$PWD/app/elementfactory.cpp \
    $$PWD/app/element/inputbutton.cpp \
    $$PWD/app/element/inputgnd.cpp \
    $$PWD/app/element/inputswitch.cpp \
    $$PWD/app/element/inputvcc.cpp \
    $$PWD/app/element/jkflipflop.cpp \
    $$PWD/app/element/jklatch.cpp \
    $$PWD/app/element/led.cpp \
    $$PWD/app/element/nand.cpp \
    $$PWD/app/element/nor.cpp \
    $$PWD/app/element/not.cpp \
    $$PWD/app/element/or.cpp \
    $$PWD/app/element/srflipflop.cpp \
    $$PWD/app/element/tflipflop.cpp \
    $$PWD/app/element/tlatch.cpp \
    $$PWD/app/element/xnor.cpp \
    $$PWD/app/element/xor.cpp \
    $$PWD/app/globalproperties.cpp \
    $$PWD/app/graphicelement.cpp \
    $$PWD/app/graphicsview.cpp \
    $$PWD/app/label.cpp \
    $$PWD/app/mainwindow.cpp \
    $$PWD/app/nodes/qneblock.cpp \
    $$PWD/app/nodes/qneconnection.cpp \
    $$PWD/app/nodes/qneport.cpp \
    $$PWD/app/priorityelement.cpp \
    $$PWD/app/priorityqueue.cpp \
    $$PWD/app/scene.cpp \
    $$PWD/app/simulationcontroller.cpp \


HEADERS  +=  \
    $$PWD/app/box.h \
    $$PWD/app/editor.h \
    $$PWD/app/element/and.h \
    $$PWD/app/element/clock.h \
    $$PWD/app/element/dflipflop.h \
    $$PWD/app/element/display.h \
    $$PWD/app/element/dlatch.h \
    $$PWD/app/elementeditor.h \
    $$PWD/app/elementfactory.h \
    $$PWD/app/element/inputbutton.h \
    $$PWD/app/element/inputgnd.h \
    $$PWD/app/element/inputswitch.h \
    $$PWD/app/element/inputvcc.h \
    $$PWD/app/element/jkflipflop.h \
    $$PWD/app/element/jklatch.h \
    $$PWD/app/element/led.h \
    $$PWD/app/element/nand.h \
    $$PWD/app/element/nor.h \
    $$PWD/app/element/not.h \
    $$PWD/app/element/or.h \
    $$PWD/app/element/srflipflop.h \
    $$PWD/app/element/tflipflop.h \
    $$PWD/app/element/tlatch.h \
    $$PWD/app/element/xnor.h \
    $$PWD/app/element/xor.h \
    $$PWD/app/globalproperties.h \
    $$PWD/app/graphicelement.h \
    $$PWD/app/graphicsview.h \
    $$PWD/app/label.h \
    $$PWD/app/mainwindow.h \
    $$PWD/app/nodes/qneblock.h \
    $$PWD/app/nodes/qneconnection.h \
    $$PWD/app/nodes/qneport.h \
    $$PWD/app/priorityelement.h \
    $$PWD/app/priorityqueue.h \
    $$PWD/app/scene.h \
    $$PWD/app/simulationcontroller.h \

INCLUDEPATH += \
    $$PWD/app \
    $$PWD/app/element \
    $$PWD/app/nodes

FORMS    += \
    $$PWD/app/mainwindow.ui \
    $$PWD/app/elementeditor.ui

RESOURCES += \
    $$PWD/app/resources/resources.qrc\
    $$PWD/app/resources/basic/basic.qrc\
    $$PWD/app/resources/input/input.qrc\
    $$PWD/app/resources/memory/memory.qrc\
    $$PWD/app/resources/output/output.qrc\
    $$PWD/app/resources/toolbar/toolbar.qrc

debug{
DEFINES += DEBUG
}

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
}
