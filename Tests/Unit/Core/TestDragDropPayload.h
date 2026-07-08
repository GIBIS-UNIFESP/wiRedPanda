// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDragDropPayload : public QObject
{
    Q_OBJECT

private slots:
    void testRoundTrip();

    // Regression: readDragDropPayload() used Qt's raw, unbounded QString
    // deserialization for icFileName/blobName — a crafted drag-and-drop MIME
    // payload (the drag source is not a trusted process) could trigger an
    // OOM-sized QString::resize() before any validation.
    void testRejectsOversizedIcFileName();
    void testRejectsOversizedBlobName();
    void testMissingOptionalFieldsDefaultGracefully();
};
