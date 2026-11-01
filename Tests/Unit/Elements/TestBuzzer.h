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

    // Frequency tests
    void testFrequency();
    void testDefaultFrequency();
    void testSetFrequency();
    void testSetNote();

    // Regression (B1): the tone picker presents C6..C7 as an ascending scale, so
    // noteToFrequency() must rise monotonically across it — A6/B6 (not the octave-high
    // A7/B7) are the correct sixth/seventh degrees, while A7/B7 stay resolvable as aliases.
    void testToneScaleIsMonotonic();

    // Regression: setFrequency() must reject non-finite/non-positive values (the buzzer's frequency
    // spec refuses them, as the clock's frequency spec in Clock.cpp refuses the same untrusted-value class).
    void testSetFrequencyRejectsNaN();
    void testSetFrequencyRejectsInfinity();
    void testSetFrequencyRejectsZeroAndNegative();

    void testSetNoteWithEmptyNoteIsNoOp();
    void testSetNoteOffTheScaleIsC6();

    // Serialization tests
    void testSaveFrequency();
    void testLoadVersionOld();
    void testLoadVersionNew();
    void testLoadBeforeAudioExisted();
};
