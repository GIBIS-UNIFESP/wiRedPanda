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

    // Regression: setFrequency() must reject non-finite/non-positive values (the base
    // GraphicElement::setFrequency() is a no-op, so Buzzer must guard itself, mirroring
    // Clock::setFrequency()'s identical guard for the same untrusted-value class).
    void testSetFrequencyRejectsNaN();
    void testSetFrequencyRejectsInfinity();
    void testSetFrequencyRejectsZeroAndNegative();

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
