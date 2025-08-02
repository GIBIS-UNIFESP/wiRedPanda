TARGET = wiredpanda

TEMPLATE = app

# Compile translations to QM files
CONFIG += lrelease

# Custom lupdate target with PANDACEPTION alias to match CMake functionality
lupdate.target = lupdate
lupdate.commands = lupdate -tr-function-alias tr+=PANDACEPTION $$PWD/../app.pro
QMAKE_EXTRA_TARGETS += lupdate

include(../config.pri)
include(install.pri)

win32 {
    QMAKE_TARGET_COMPANY = GIBIS-UNIFESP
    QMAKE_TARGET_PRODUCT = wiRedPanda - Logic Circuit Simulator
    QMAKE_TARGET_DESCRIPTION = wiRedPanda - Logic Circuit Simulator
    QMAKE_TARGET_COPYRIGHT = GIBIS-UNIFESP and the wiRedPanda contributors

    RC_ICONS = resources/wpanda.ico
}

SOURCES += main.cpp

DISTFILES += resources/postinst

TRANSLATIONS += \
    resources/translations/wpanda_ar.ts \
    resources/translations/wpanda_de.ts \
    resources/translations/wpanda_en.ts \
    resources/translations/wpanda_es.ts \
    resources/translations/wpanda_fr.ts \
    resources/translations/wpanda_hi.ts \
    resources/translations/wpanda_id.ts \
    resources/translations/wpanda_it.ts \
    resources/translations/wpanda_ja.ts \
    resources/translations/wpanda_ko.ts \
    resources/translations/wpanda_nl.ts \
    resources/translations/wpanda_pl.ts \
    resources/translations/wpanda_pt_BR.ts \
    resources/translations/wpanda_ro.ts \
    resources/translations/wpanda_ru.ts \
    resources/translations/wpanda_tr.ts \
    resources/translations/wpanda_uk.ts \
    resources/translations/wpanda_vi.ts \
    resources/translations/wpanda_zh_Hans.ts
