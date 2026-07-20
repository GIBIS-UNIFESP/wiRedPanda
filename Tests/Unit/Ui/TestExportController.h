// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestExportController : public QObject
{
    Q_OBJECT

private slots:

    void testExportToArduinoGuardsMissingTab();
    void testExportToArduinoThrowsOnEmptyFileName();
    void testExportToArduinoThrowsOnEmptyScene();
    void testExportToArduinoAppendsExtensionAndGenerates();

    void testExportToSystemVerilogGuardsMissingTab();
    void testExportToSystemVerilogThrowsOnEmptyFileName();
    void testExportToSystemVerilogThrowsOnEmptyScene();
    void testExportToSystemVerilogAppendsExtensionAndGenerates();

    void testExportToWaveFormFileThrowsOnEmptyFileName();
    void testExportToWaveFormFileGeneratesAndPrints();
    void testExportToWaveFormTerminalReadsStdinAndPrints();

    void testExportArduinoDialogGuardsMissingTab();
    void testExportSystemVerilogDialogGuardsMissingTab();

    void testExportPdfDialogGuardsMissingTab();
    void testExportPdfDialogCancelledIsNoOp();
    void testExportPdfDialogAppendsExtensionAndOpensFile();

    void testExportImageDialogGuardsMissingTab();
    void testExportImageDialogCancelledIsNoOp();
    void testExportImageDialogAppendsExtensionAndOpensFile();
};
