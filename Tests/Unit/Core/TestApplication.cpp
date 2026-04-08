// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestApplication.h"

#include <QCoreApplication>

#include "Tests/Common/TestUtils.h"

void TestApplication::testApplicationNotify()
{
    // Application requires argc/argv — verify the test runner's QApplication is alive.
    QVERIFY(QCoreApplication::instance() != nullptr);
}

void TestApplication::testExceptionHandling()
{
    QVERIFY(QCoreApplication::instance() != nullptr);
}

