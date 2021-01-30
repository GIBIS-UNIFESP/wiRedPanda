// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testicons.h"
#include "elementfactory.h"

void TestIcons::testIcons()
{
    for (int type = static_cast<int>(ElementType::BUTTON); type < static_cast<int>(ElementType::DEMUX); ++type) {
        QVERIFY2(!ElementFactory::getPixmap(static_cast<ElementType>(type)).isNull(),
                 QString(ElementFactory::typeToText(static_cast<ElementType>(type)) + " pixmap not found.").toUtf8());
    }
}