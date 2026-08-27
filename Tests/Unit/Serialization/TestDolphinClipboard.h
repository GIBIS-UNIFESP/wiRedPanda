// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDolphinClipboard : public QObject
{
    Q_OBJECT

private slots:
    void testPasteRoundTrip();

    /// Input rows are two-state by definition -- the sweep reads them as `value != 0`. Output
    /// rows legitimately hold four-state Status, so copying an Unknown (-1)
    /// output cell into an input row made that input read HIGH, while the delegate rendered it
    /// as a grey Unknown bar and the exporter wrote 'x'. paste() bounds row/col but never the
    /// VALUE, and it explicitly treats the clipboard as untrusted.
    void testPasteClampsInputRowValues();

    // Regression: DolphinClipboard::paste() read its item count straight off the
    // system clipboard with no bound, so a crafted/corrupt payload could spin the
    // paste loop for eons instead of failing fast.
    void testPasteTruncatesImplausibleItemCount();
    void testPasteWithNoDataDoesNothing();

    void testPasteFromClipboardAcceptsLegacyMimeType();
    void testPasteFromClipboardReturnsFalseWhenEmpty();
};
