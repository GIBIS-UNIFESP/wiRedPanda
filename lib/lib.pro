TARGET = wiredpanda_lib
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = $$OUT_PWD

# macOS library-specific settings
mac {
    greaterThan(QT_MAJOR_VERSION, 5): QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

include(../config.pri)
include(wiredpanda_lib.pri)
