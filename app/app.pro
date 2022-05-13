TARGET = wpanda

TEMPLATE = app

include(../config.pri)
include(install.pri)

win32 {
    RC_FILE = windows.rc
    DISTFILES += windows.rc
}

SOURCES += main.cpp

DISTFILES += resources/postinst

TRANSLATIONS += resources/wpanda_pt_BR.ts
