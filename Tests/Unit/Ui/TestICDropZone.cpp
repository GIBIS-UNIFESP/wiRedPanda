// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestICDropZone.h"

#include "App/UI/ICDropZone.h"
#include "Tests/Common/TestUtils.h"

void TestICDropZone::testICDropEvent()
{
    ICDropZone dropZone(ICDropZone::Section::FileBased);
    QVERIFY(dropZone.acceptDrops());
}

void TestICDropZone::testICMimeData()
{
    ICDropZone dropZone(ICDropZone::Section::Embedded);
    QVERIFY(dropZone.acceptDrops());
}

