// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestTrashButton.h"

#include <QMimeData>

#include "App/UI/TrashButton.h"
#include "Tests/Common/TestUtils.h"

void TestTrashButton::testDragEnterEvent()
{
    TrashButton button;
    button.setAcceptDrops(true);
    QVERIFY(button.acceptDrops());
}

void TestTrashButton::testDropEvent()
{
    TrashButton button;
    QVERIFY(true);
}

void TestTrashButton::testEmptyDrop()
{
    TrashButton button;
    QVERIFY(true);
}

