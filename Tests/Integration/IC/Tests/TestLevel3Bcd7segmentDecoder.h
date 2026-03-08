// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel3BCD7SegmentDecoder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // BCD to 7-Segment Decoder Tests
    void testBCD7SegmentDecoder_data();
    void testBCD7SegmentDecoder();
};
