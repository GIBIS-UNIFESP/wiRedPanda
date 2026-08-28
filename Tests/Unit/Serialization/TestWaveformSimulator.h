// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestWaveformSimulator : public QObject
{
    Q_OBJECT

private slots:

    void testCaptureAndRestoreSinglePortInputs();
    void testCaptureAndRestoreMultiPortInput();
    /// A four-state Status must survive the capture/restore round trip. Status::Unknown is -1
    /// and Status::Error is 2, so casting one to bool makes BOTH true and brings an undefined
    /// input port back driven HIGH -- a sweep silently mutating the live circuit.
    void testRestorePreservesNonDefiniteStatus();
};
