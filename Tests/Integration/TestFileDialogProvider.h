// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

/// Tests for file-dialog-dependent GUI paths using the StubFileDialogProvider.
///
/// Verifies that MainWindow, WorkSpace, and ElementEditor correctly use
/// FileDialogs::provider() and that the dialog parameters (caption, filter,
/// starting directory) are correct.
class TestFileDialogProvider : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // --- Provider infrastructure ---

    void testDefaultProviderIsNotNull();
    void testSetProviderSwapsAndRestores();
    void testScopedStubRestoresOnDestruction();

    // --- MainWindow: Open ---

    void testOpenActionUsesProvider();
    void testOpenActionCancelledDoesNotLoad();

    // --- MainWindow: Save / Save As ---

    void testSaveAsUsesProvider();
    void testSaveAsAppendsExtension();
    void testSaveFirstTimeUsesProvider();

    // --- MainWindow: Exports ---

    void testExportArduinoUsesProvider();
    void testExportSystemVerilogUsesProvider();
    void testExportPdfUsesProvider();
    void testExportImageUsesProvider();

    // --- MainWindow: IC embed/extract ---

    void testEmbedICFromFileUsesProvider();

    // --- WorkSpace: Save dialog ---

    void testWorkspaceSaveDialogUsesProvider();

    // --- ElementEditor: Audio ---

    void testAudioFileSelectionUsesProvider();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};

