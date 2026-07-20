// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestSelectionCapabilities.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/UI/SelectionCapabilities.h"
#include "Tests/Common/TestUtils.h"

void TestSelectionCapabilities::testSelectionCapabilities()
{
    SelectionCapabilities caps = computeCapabilities({});
    QVERIFY(!caps.hasElements);
    QCOMPARE(caps.elementType, ElementType::Unknown);
}

void TestSelectionCapabilities::testDemuxSelectionCannotChangeInputSize()
{
    // Demux derives its input size from its own output size, so the generic input-size
    // combobox must be hidden even when the selection's min/max input range would otherwise
    // allow it.
    auto *demux1 = ElementFactory::buildElement(ElementType::Demux);
    auto *demux2 = ElementFactory::buildElement(ElementType::Demux);

    SelectionCapabilities caps = computeCapabilities({demux1, demux2});

    QVERIFY(caps.hasSameType);
    QCOMPARE(caps.elementType, ElementType::Demux);
    QVERIFY2(!caps.canChangeInputSize, "Demux selections must never allow changing input size directly");

    delete demux1;
    delete demux2;
}
