// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testicons.h"

#include "elementfactory.h"

#include <QTest>

void TestIcons::testIcons()
{
    for (auto type = ElementType::Button; type < ElementType::Demux; ++type) {
        const auto pixmap = ElementFactory::getPixmap(type);
        const auto text = ElementFactory::typeToText(type);
        QVERIFY2(!pixmap.isNull(), QString(text + " pixmap not found.").toUtf8());
    }
}
