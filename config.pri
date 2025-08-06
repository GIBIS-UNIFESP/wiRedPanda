equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 15) {
    error("Use Qt 5.15 or newer")
}

equals(QT_MAJOR_VERSION, 6) : !versionAtLeast(QT_VERSION, 6.2.0) {
    error("For Qt6 the minimum version is 6.2.0")
}

!qtHaveModule(multimedia) {
    error("QtMultimedia is not installed. Please install with Qt Maintenance Tool or with system repository")
}

VERSION = 4.2.6
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += core gui printsupport multimedia widgets svg

CONFIG += warn_on strict_c strict_c++

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
DEFINES += QT_MESSAGELOGCONTEXT


wasm {
    QMAKE_LFLAGS += -sASYNCIFY -Os
}

linux {
    MOLD_BIN = $$system(which mold)

    !isEmpty(MOLD_BIN) {
        QMAKE_LFLAGS += -fuse-ld=mold
    }
}

linux-g++ {
    !isEmpty(COVERAGE) {
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

linux | mac {
    CCACHE_BIN = $$system(which ccache)
}

win32 {
    !msvc {
        CCACHE_BIN = $$system(where ccache)
    }
}

isEmpty(CCACHE_BIN) {
    PRECOMPILED_HEADER = $$PWD/pch.h
    CONFIG += precompile_header
} else {
    QMAKE_CC = ccache $$QMAKE_CC
    QMAKE_CXX = ccache $$QMAKE_CXX
}

CXX_STANDARD = c++20

msvc {
    QMAKE_CXXFLAGS_WARN_ON ~= s/-W3/-W4
    QMAKE_CXXFLAGS += /external:I $$[QT_INSTALL_PREFIX] /external:W0
    QMAKE_CXXFLAGS += /permissive- /Zc:preprocessor /std:$$CXX_STANDARD
    QMAKE_CXXFLAGS_DEBUG += /Ob1
    QMAKE_CXXFLAGS_RELEASE += /GL /Zi
    QMAKE_LFLAGS_RELEASE += /LTCG /DEBUG
} else {
    QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic -std=$$CXX_STANDARD
}

linux | mac | win32-g++ {
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE += -g
}

mac {
    CONFIG += sdk_no_version_check

    # Override Qt's mkspecs -std=c++11 flag with C++20
    # Qt 5.15.2 mkspecs inject -std=c++11 after config.pri processing
    # Use QMAKE_CXXFLAGS_WARN_ON which gets processed late to override it
    QMAKE_CXXFLAGS_WARN_ON += -std=c++20

    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_LFLAGS_RELEASE += -Wl,-dead_strip_dylibs -g

    greaterThan(QT_MAJOR_VERSION, 5): QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

MOC_DIR        = build_files/moc
UI_DIR         = build_files/ui
UI_HEADERS_DIR = build_files/ui
UI_SOURCES_DIR = build_files/ui
OBJECTS_DIR    = build_files/obj
RCC_DIR        = build_files/rcc

INCLUDEPATH += \
    $$PWD/app \
    $$PWD/app/arduino \
    $$PWD/app/element \
    $$PWD/app/logicelement \
    $$PWD/app/nodes
