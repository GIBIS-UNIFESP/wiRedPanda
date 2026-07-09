// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICDropZone : public QObject
{
    Q_OBJECT

private slots:

    void testICDropEvent();
    void testICMimeData();

    // Regression: a crafted drag payload with an implausible icFileName/blobName
    // length must be rejected, not crash or hang the drop handler.
    void testDropEventRejectsOversizedPayload();

    // The drop hint appears only while a compatible IC is dragged over the zone.
    void testHintShownOnCompatibleDragEnter();
};
