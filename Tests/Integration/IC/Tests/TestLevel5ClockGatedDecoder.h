// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5ClockGatedDecoder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // Clock Gated Decoder Tests
    void testClockGatedDecoder_data();
    void testClockGatedDecoder();
};

