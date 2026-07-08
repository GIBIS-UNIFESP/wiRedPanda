// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestMCPProcessor.h"

#include <QTest>

#include "MCP/Server/Core/MCPProcessor.h"

void TestMCPProcessor::testExtractStdinLinesSplitsCompleteLines()
{
    QByteArray buffer;
    const QStringList lines = MCPProcessor::extractStdinLines(buffer, "abc\ndef\n");

    QCOMPARE(lines, QStringList({"abc", "def"}));
    QVERIFY(buffer.isEmpty());
}

void TestMCPProcessor::testExtractStdinLinesHandlesPartialLineAcrossCalls()
{
    QByteArray buffer;

    const QStringList firstResult = MCPProcessor::extractStdinLines(buffer, "ab");
    QVERIFY(firstResult.isEmpty());
    QCOMPARE(buffer, QByteArray("ab"));

    const QStringList secondResult = MCPProcessor::extractStdinLines(buffer, "c\n");
    QCOMPARE(secondResult, QStringList({"abc"}));
    QVERIFY(buffer.isEmpty());
}

void TestMCPProcessor::testExtractStdinLinesKeepsIncompleteLineAtExactCap()
{
    // No newline anywhere — the whole thing stays buffered, waiting for more data.
    // Exactly at the cap must NOT be treated as an overflow.
    QByteArray buffer;
    const QByteArray chunk(MCPProcessor::kMaxStdinLineBytes, 'x');

    const QStringList lines = MCPProcessor::extractStdinLines(buffer, chunk);

    QVERIFY(lines.isEmpty());
    QCOMPARE(buffer.size(), MCPProcessor::kMaxStdinLineBytes);
}

void TestMCPProcessor::testExtractStdinLinesDropsIncompleteLineOverCap()
{
    // One byte past the cap, still no newline: a client that never sends '\n' must not be
    // allowed to grow the buffer without bound.
    QByteArray buffer;
    const QByteArray chunk(MCPProcessor::kMaxStdinLineBytes + 1, 'x');

    const QStringList lines = MCPProcessor::extractStdinLines(buffer, chunk);

    QVERIFY(lines.isEmpty());
    QVERIFY2(buffer.isEmpty(), "Buffer must be dropped once it exceeds the cap with no newline");
}
