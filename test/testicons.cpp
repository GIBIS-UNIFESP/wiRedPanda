// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testicons.h"

#include "elementfactory.h"

#include <QTest>

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
