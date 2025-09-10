// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testicons.h"

#include "common.h"
#include "elementfactory.h"
#include "registertypes.h"

#include <QApplication>
#include <QTest>

int main(int argc, char **argv)
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif

    registerTypes();

    Comment::setVerbosity(-1);

    QApplication app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    TestIcons testIcons;
    return QTest::qExec(&testIcons, argc, argv);
}

void TestIcons::testIcons()
{
    for (auto type = ElementType::InputButton; type < ElementType::Demux; ++type) {
        if ((type == ElementType::JKLatch) || (type == ElementType::IC)) {
            continue;
        }

        const auto pixmap = ElementFactory::pixmap(type);
        const auto text = ElementFactory::typeToText(type);
        QVERIFY2(!pixmap.isNull(), QString(text + " pixmap not found.").toUtf8());
    }
}
