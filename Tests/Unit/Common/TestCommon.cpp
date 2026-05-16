// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestCommon.h"

#include <QTest>

#include "App/Core/Common.h"

// Exception Tests

void TestCommon::testPandaceptionMessage()
{
    // Test: Pandaception exception with translated message
    QString translatedMsg = "Translated error message";
    QString englishMsg = "English error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Verify translated message is in what() (std::runtime_error)
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // Verify both messages are accessible
    QCOMPARE(ex.englishMessage(), englishMsg);
}

void TestCommon::testPandaceptionEnglishMessage()
{
    // Test: Pandaception keeps English message separate
    QString translatedMsg = "Mensaje de error traducido";  // Spanish
    QString englishMsg = "Error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Translated message is in what()
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // English message is in englishMessage()
    QCOMPARE(ex.englishMessage(), englishMsg);

    // Both are different
    QVERIFY(QString::fromStdString(ex.what()) != ex.englishMessage());
}
