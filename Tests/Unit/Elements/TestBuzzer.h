// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestBuzzer : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorInitialization();
    void testCopyConstructor();

    // Frequency tests
    void testFrequency();
    void testDefaultFrequency();
    void testSetFrequency();
    void testSetAudioBackwardCompat();

    // Playback control tests
    void testPlayStopTransition();
    void testPlayIdempotency();
    void testStopIdempotency();
    void testRefreshWithActiveInput();
    void testRefreshWithInactiveInput();

    // Mute control tests
    void testMute();
    void testUnmute();

    // Serialization tests
    void testSaveFrequency();
    void testLoadVersionOld();
    void testLoadVersionNew();
};

