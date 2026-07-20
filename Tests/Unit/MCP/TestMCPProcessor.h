// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestMCPProcessor : public QObject
{
    Q_OBJECT

private slots:
    void testExtractStdinLinesSplitsCompleteLines();
    void testExtractStdinLinesHandlesPartialLineAcrossCalls();

    // Regression: stdin is not a trusted channel — a client that never sends '\n' must not
    // grow the pending-line buffer without bound.
    void testExtractStdinLinesKeepsIncompleteLineAtExactCap();
    void testExtractStdinLinesDropsIncompleteLineOverCap();

    void testProcessIncomingDataIgnoresBlankLine();
    void testProcessCommandRejectsInvalidJson();
    void testProcessCommandRejectsWrongJsonRpcVersion();
    void testProcessCommandRejectsSchemaViolation();
    void testProcessCommandRejectsUnknownMethod();
    void testProcessCommandDispatchesKnownMethodSuccessfully();

    // These drive the real QSocketNotifier -- MCPProcessor uses a dedicated StdinReader
    // thread on Windows instead (see MCPProcessor.h), so there's no m_stdinNotifier there.
#ifndef Q_OS_WIN
    void testStartStopProcessingTogglesNotifier();
    void testOnStdinReadableDispatchesRealData();
    void testOnStdinReadableQuitsOnEof();
#endif

    void testHeapAllocationDeletesCleanly();
};
