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

    // Audio accessor tests
    void testAudioNote();
    void testAudioNoteEmpty();

    // Audio configuration tests
    void testSetAudioWithValidNote();
    void testSetAudioWithEmptyNote();
    void testAudioPersistence();

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
    void testSaveNote();
    void testLoadVersionOld();
    void testLoadVersionNew();
};
