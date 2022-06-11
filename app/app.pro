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

TRANSLATIONS += \
    resources/translations/wpanda_en.ts \
    resources/translations/wpanda_pt_BR.ts
