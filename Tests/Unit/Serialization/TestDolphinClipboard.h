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

    // Regression: DolphinClipboard::paste() read its item count straight off the
    // system clipboard with no bound, so a crafted/corrupt payload could spin the
    // paste loop for eons instead of failing fast.
    void testPasteTruncatesImplausibleItemCount();
    void testPasteWithNoDataDoesNothing();

    void testPasteFromClipboardAcceptsLegacyMimeType();
    void testPasteFromClipboardReturnsFalseWhenEmpty();
};
