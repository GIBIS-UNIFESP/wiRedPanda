// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestLengthDialog.h"

#include <QTimer>

#include "App/UI/LengthDialog.h"
#include "Tests/Common/TestUtils.h"

void TestLengthDialog::testDialogAccept()
{
    // length() execs the modal dialog; queue an accept so it returns the
    // pre-populated spin-box value
    LengthDialog dialog(32);
    QTimer::singleShot(0, &dialog, &QDialog::accept);
    QCOMPARE(dialog.length(), 32);
}

void TestLengthDialog::testDialogReject()
{
    // A dismissed dialog reports -1 so the caller can tell cancel apart
    // from a chosen length
    LengthDialog dialog(32);
    QTimer::singleShot(0, &dialog, &QDialog::reject);
    QCOMPARE(dialog.length(), -1);
}
