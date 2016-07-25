#-------------------------------------------------
#
# Project created by QtCreator 2014-10-22T13:17:08
#
#-------------------------------------------------

TARGET = wpanda

TEMPLATE = app

include(../includes.pri)
include(install.pri)

win32{
RC_FILE = windows.rc
DISTFILES += windows.rc
}

SOURCES += main.cpp

DISTFILES += resources/postinst

RESOURCES += \
    resources/translations.qrc

TRANSLATIONS = app/resources/wpanda_en.ts  app/resources/wpanda_pt.ts
