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
    /// csvText() is comma-separated, so it keeps the raw four-state Status ints (-1 unknown,
    /// 2 error) -- the encoding create_waveform declares over MCP -- where its separator-less
    /// sibling must spend exactly one character. Both are pinned so a "make these consistent"
    /// pass has to confront the reason they differ.
    void testCsvTextKeepsFourStateIntsOnOutputRows();
};
