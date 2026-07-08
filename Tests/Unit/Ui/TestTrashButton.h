// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestTrashButton : public QObject
{
    Q_OBJECT

private slots:

    void testDragEnterEvent();
    void testDropEvent();
    void testEmptyDrop();

    // Regression: a crafted drag payload with an implausible icFileName/blobName
    // length must be rejected, not crash or hang the drop handler.
    void testDropEventRejectsOversizedPayload();
};
