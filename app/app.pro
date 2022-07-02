TARGET = wpanda

TEMPLATE = app

include(../config.pri)
include(install.pri)

win32 {
    QMAKE_TARGET_COMPANY = GIBIS-UNIFESP
    QMAKE_TARGET_PRODUCT = WiRedPanda
    QMAKE_TARGET_DESCRIPTION = Logic Circuit Simulator
    QMAKE_TARGET_COPYRIGHT = Davi Morales, Fábio Cappabianco, Lucas Lellis, Rodrigo Torres and Vinícius Miguel

    RC_ICONS = resources/wpanda.ico
}

SOURCES += main.cpp

DISTFILES += resources/postinst

TRANSLATIONS += \
    resources/translations/wpanda_en.ts \
    resources/translations/wpanda_pt_BR.ts
