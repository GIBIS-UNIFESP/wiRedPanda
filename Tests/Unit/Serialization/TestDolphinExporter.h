// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDolphinExporter : public QObject
{
    Q_OBJECT

private slots:

    void testExportToPdfThrowsWhenPrinterCannotOpen();

    /// writeTruthTableText() writes cells with NO separator, so every cell must be exactly one
    /// character. A raw Status::Unknown (-1) prints as two and silently shifts the column count,
    /// making the row unparseable -- routinely reachable, since oscillating regions canonicalise
    /// to Unknown and propagate it to their readers.
    void testTruthTableTextWritesOneCharacterPerThreeStateCell();
};
