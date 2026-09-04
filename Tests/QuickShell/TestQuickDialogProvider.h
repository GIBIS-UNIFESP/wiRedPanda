// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests QuickDialogProvider (App/QuickShell/Chrome/QuickDialogProvider) by driving its actual
/// QML-backed dialogs, rather than only through StubDialogProvider/StubFileDialogProvider.
///
/// Technique: QuickDialogProvider's dialogOpened(QObject*) signal is a purpose-built test hook,
/// emitted before the modal wait begins, so a test can drive the live QML dialog object directly
/// (set properties, invoke accept()/reject()/handleButtonClicked()) instead of needing real
/// synthetic mouse/window input (this project's own sandbox can't deliver that to a real Quick
/// window -- see project_xwayland_synthetic_input_broken.md). Every test here connects to
/// dialogOpened and drives the dialog via a deferred QTimer::singleShot(0, ...) -- calling
/// accept()/reject() synchronously inside dialogOpened() runs before execModal()'s own nested
/// QEventLoop starts, silently no-op'ing the close and hanging forever.
///
/// getSaveFileNameWithDirPathSetsCurrentFolder/WithFullFilePathPresetsSelectedFile cover the
/// QFileInfo(dir).isDir() branch in QuickDialogProvider::getSaveFileName(), which has no
/// Widgets-side equivalent -- RealFileDialogProvider::getSaveFileName() just forwards to
/// QFileDialog::getSaveFileName()'s own static, which handles this internally.
class TestQuickDialogProvider : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testChoiceReturnsClickedButton();
    void testChoiceReturnsDefaultButtonWhenDismissedWithoutClick();

    void testTextPromptReturnsEnteredValueOnAccept();
    void testTextPromptReturnsNulloptOnCancel();

    void testGetOpenFileNameReturnsSelectedPath();
    void testGetOpenFileNameReturnsEmptyOnCancel();

    void testGetSaveFileNameReturnsPathAndFilter();
    void testGetSaveFileNameReturnsEmptyOnCancel();
    void testGetSaveFileNameWithDirPathSetsCurrentFolder();
    void testGetSaveFileNameWithFullFilePathPresetsSelectedFile();

private:
    class QQmlEngine *m_engine = nullptr;
    class QQuickWindow *m_window = nullptr;
};
