// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/CodeGen/TestCodeGenUtils.h"

#include "App/CodeGen/CodeGenUtils.h"

void TestCodeGenUtils::testSanitizeCommentStripsLineBreaks()
{
    // Regression: a label containing an embedded newline, written raw into a "// IC: <label>"
    // comment by ArduinoCodeGen/SystemVerilogCodeGen, could break out of the comment and
    // inject an arbitrary line into the generated file. sanitizeComment() must neutralize any
    // line-break character so the label can never span more than the one comment line.
    const QString malicious = QStringLiteral("Evil\ndigitalWrite(13, HIGH); //");
    const QString sanitized = CodeGenUtils::sanitizeComment(malicious);

    QVERIFY(!sanitized.contains(QLatin1Char('\n')));
    QVERIFY(!sanitized.contains(QLatin1Char('\r')));

    const QString withCarriageReturn = QStringLiteral("Evil\r\ninjected();");
    QVERIFY(!CodeGenUtils::sanitizeComment(withCarriageReturn).contains(QLatin1Char('\n')));
    QVERIFY(!CodeGenUtils::sanitizeComment(withCarriageReturn).contains(QLatin1Char('\r')));
}

void TestCodeGenUtils::testSanitizeCommentLeavesCleanTextUnchanged()
{
    const QString clean = QStringLiteral("Half Adder (4-bit)");
    QCOMPARE(CodeGenUtils::sanitizeComment(clean), clean);
}
