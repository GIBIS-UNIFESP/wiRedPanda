#-------------------------------------------------
#
# Project created by QtCreator 2014-10-22T13:17:08
#
#-------------------------------------------------

TARGET = wpanda
VERSION = 1.9

TEMPLATE = app

include(../includes.pri)
include(install.pri)

win32{
RC_FILE = windows.rc
DISTFILES += windows.rc
}

SOURCES += main.cpp


DEFINES += APP_VERSION=\\\"$$VERSION\\\"

DISTFILES += \
    resources/postinst


