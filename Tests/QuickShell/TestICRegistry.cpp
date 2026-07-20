// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestICRegistry.h"

#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

void TestICRegistry::testICRegistration()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    QVERIFY(canvas.icRegistry()->blobMapRef().isEmpty());
}

void TestICRegistry::testICValidation()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    QVERIFY(!canvas.icRegistry()->hasBlob("nonexistent"));
}
