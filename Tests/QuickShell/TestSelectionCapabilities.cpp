// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSelectionCapabilities.h"

#include "App/Element/SelectionCapabilities.h"

void TestSelectionCapabilities::testSelectionCapabilities()
{
    SelectionCapabilities caps = computeCapabilities({});
    QVERIFY(!caps.hasElements);
    QCOMPARE(caps.elementType, ElementType::Unknown);
}
