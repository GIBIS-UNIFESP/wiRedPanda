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
<<<<<<< HEAD

RESOURCES +=

FORMS +=

HEADERS +=

TRANSLATIONS = wpanda_en.ts \
               wpanda_pt.ts
=======
>>>>>>> master
