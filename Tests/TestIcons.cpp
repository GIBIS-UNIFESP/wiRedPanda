// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/TestIcons.h"

#include <QApplication>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/RegisterTypes.h"

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
        QVERIFY2(!pixmap.isNull(), (text + " pixmap not found.").toUtf8());
    }
}

