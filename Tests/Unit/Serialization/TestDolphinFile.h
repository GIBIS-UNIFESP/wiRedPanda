// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDolphinFile : public QObject
{
    Q_OBJECT

private slots:

    // save()/load() CSV round trip
    void testSaveAndLoadCSVRoundTrip();

    // save() error paths
    void testSaveOpenFailureThrows();

    // load() error paths
    void testLoadMissingFileThrows();
    void testLoadOpenFailureThrows();
    void testLoadUnsupportedFormatThrows();

    // parseTerminal()
    void testParseTerminalValidInput();
    void testParseTerminalClampsRowsToMaxInputPorts();
    void testParseTerminalRejectsShortHeader();
    void testParseTerminalRejectsNonPositiveRows();
    void testParseTerminalRejectsColumnCountTooLow();
    void testParseTerminalRejectsColumnCountTooHigh();
    void testParseTerminalRejectsShortDataRow();
};
