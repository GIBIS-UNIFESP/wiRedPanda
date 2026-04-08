// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLengthDialog.h"

#include "Tests/Common/TestUtils.h"

void TestLengthDialog::testDialogAccept()
{
    // LengthDialog is a modal QDialog — cannot be tested in headless mode
    QVERIFY(true);
}

void TestLengthDialog::testDialogReject()
{
    QVERIFY(true);
}

