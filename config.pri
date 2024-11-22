equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 12) {
    error("Use Qt 5.12 or newer")
}

equals(QT_MAJOR_VERSION, 6) : !versionAtLeast(QT_VERSION, 6.2.0) {
    error("For Qt6 the minimum version is 6.2.0")
}

!qtHaveModule(multimedia) {
    error("QtMultimedia is not installed. Please install with Qt Maintenance Tool or with system repository")
}

VERSION = 4.2
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += core gui printsupport multimedia widgets svg

CONFIG += c++17 warn_on strict_c strict_c++
QMAKE_LFLAGS += -sASYNCIFY -Os
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += QT_MESSAGELOGCONTEXT

linux {
    MOLD_BIN = $$system(which mold)

    !isEmpty(MOLD_BIN) {
        QMAKE_LFLAGS += -fuse-ld=mold
    }
}

linux-g++ {
    contains(CONFIG, coverage) {
        QMAKE_CXXFLAGS += --coverage
        QMAKE_LFLAGS += --coverage
    }
}

linux-clang {
    contains(CONFIG, MemorySanitizer) {
        QMAKE_CXXFLAGS += -fsanitize=memory -fPIE
        QMAKE_LFLAGS += -fsanitize=memory -fPIE
    }

    contains(CONFIG, UBSan) {
        QMAKE_CXXFLAGS += -fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability
        QMAKE_LFLAGS += -fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability
    }

    contains(CONFIG, AddressSanitizer) {
        QMAKE_CXXFLAGS += -fsanitize=address
        QMAKE_LFLAGS += -fsanitize=address
    }

    contains(CONFIG, ThreadSanitizer) {
        QMAKE_CXXFLAGS += -fsanitize=thread
        QMAKE_LFLAGS += -fsanitize=thread
    }
}

unix: QMAKE_RPATHDIR += ${ORIGIN}/lib

linux {
    CCACHE_BIN = $$system(which ccache)

    !isEmpty(CCACHE_BIN) {
        QMAKE_CC = ccache $$QMAKE_CC
        QMAKE_CXX = ccache $$QMAKE_CXX
    } else {
        PRECOMPILED_HEADER = $$PWD/pch.h
        CONFIG += precompile_header
    }
}

!linux {
    PRECOMPILED_HEADER = $$PWD/pch.h
    CONFIG += precompile_header
}

msvc {
    QMAKE_CXXFLAGS_WARN_ON ~= s/-W3/-W4
    QMAKE_CXXFLAGS += /permissive- /external:I $$[QT_INSTALL_PREFIX] /external:W0
    QMAKE_CXXFLAGS_DEBUG += /Ob1
    QMAKE_CXXFLAGS_RELEASE += /GL
    QMAKE_LFLAGS_RELEASE += /LTCG
}

*-g++ {
    QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-deprecated-enum-enum-conversion # can be removed after migrating to Qt6
}

*-clang {
    QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion # can be removed after migrating to Qt6
}

mac {
    CONFIG += sdk_no_version_check
}

MOC_DIR        = build_files/moc
UI_DIR         = build_files/ui
UI_HEADERS_DIR = build_files/ui
UI_SOURCES_DIR = build_files/ui
OBJECTS_DIR    = build_files/obj
RCC_DIR        = build_files/rcc

include(app/element/element.pri)
include(app/logicelement/logicelement.pri)

SOURCES += \
    $$PWD/app/application.cpp \
    $$PWD/app/arduino/codegenerator.cpp \
    $$PWD/app/bewaveddolphin.cpp \
    $$PWD/app/clockdialog.cpp \
    $$PWD/app/commands.cpp \
    $$PWD/app/common.cpp \
    $$PWD/app/elementeditor.cpp \
    $$PWD/app/elementfactory.cpp \
    $$PWD/app/elementlabel.cpp \
    $$PWD/app/elementmapping.cpp \
    $$PWD/app/enums.cpp \
    $$PWD/app/graphicelement.cpp \
    $$PWD/app/graphicsview.cpp \
    $$PWD/app/ic.cpp \
    $$PWD/app/itemwithid.cpp \
    $$PWD/app/lengthdialog.cpp \
    $$PWD/app/logicelement.cpp \
    $$PWD/app/mainwindow.cpp \
    $$PWD/app/nodes/qneconnection.cpp \
    $$PWD/app/nodes/qneport.cpp \
    $$PWD/app/recentfiles.cpp \
    $$PWD/app/scene.cpp \
    $$PWD/app/serialization.cpp \
    $$PWD/app/settings.cpp \
    $$PWD/app/simulation.cpp \
    $$PWD/app/simulationblocker.cpp \
    $$PWD/app/thememanager.cpp \
    $$PWD/app/trashbutton.cpp \
    $$PWD/app/workspace.cpp

HEADERS += \
    $$PWD/app/application.h \
    $$PWD/app/arduino/codegenerator.h \
    $$PWD/app/bewaveddolphin.h \
    $$PWD/app/clockdialog.h \
    $$PWD/app/commands.h \
    $$PWD/app/common.h \
    $$PWD/app/elementeditor.h \
    $$PWD/app/elementfactory.h \
    $$PWD/app/elementlabel.h \
    $$PWD/app/elementmapping.h \
    $$PWD/app/enums.h \
    $$PWD/app/globalproperties.h \
    $$PWD/app/graphicelement.h \
    $$PWD/app/graphicelementinput.h \
    $$PWD/app/graphicsview.h \
    $$PWD/app/ic.h \
    $$PWD/app/itemwithid.h \
    $$PWD/app/lengthdialog.h \
    $$PWD/app/logicelement.h \
    $$PWD/app/mainwindow.h \
    $$PWD/app/nodes/qneconnection.h \
    $$PWD/app/nodes/qneport.h \
    $$PWD/app/recentfiles.h \
    $$PWD/app/scene.h \
    $$PWD/app/serialization.h \
    $$PWD/app/settings.h \
    $$PWD/app/simulation.h \
    $$PWD/app/simulationblocker.h \
    $$PWD/app/thememanager.h \
    $$PWD/app/trashbutton.h \
    $$PWD/app/workspace.h

INCLUDEPATH += \
    $$PWD/app \
    $$PWD/app/arduino \
    $$PWD/app/element \
    $$PWD/app/logicelement \
    $$PWD/app/nodes

FORMS += \
    $$PWD/app/bewaveddolphin.ui \
    $$PWD/app/clockdialog.ui \
    $$PWD/app/elementeditor.ui \
    $$PWD/app/lengthdialog.ui \
    $$PWD/app/mainwindow.ui

RESOURCES += \
    $$PWD/app/resources/basic/basic.qrc \
    $$PWD/app/resources/dolphin/dolphin.qrc \
    $$PWD/app/resources/input/input.qrc\
    $$PWD/app/resources/memory/dark/memory_dark.qrc \
    $$PWD/app/resources/memory/light/memory_light.qrc \
    $$PWD/app/resources/misc/misc.qrc \
    $$PWD/app/resources/output/output.qrc\
    $$PWD/app/resources/toolbar/toolbar.qrc \
    $$PWD/app/resources/translations/translations.qrc

DISTFILES +=
