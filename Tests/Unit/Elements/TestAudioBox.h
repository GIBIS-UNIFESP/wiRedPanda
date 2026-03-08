// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestAudioBox : public QObject
{
    Q_OBJECT

private slots:
    // Constructor tests
    void testConstructorInitialization();
    void testCopyConstructor();

    // Audio accessor tests
    void testAudioPath();
    void testAudioPathEmpty();

    // Audio configuration tests
    void testSetAudioWithQRCPath();
    void testSetAudioWithEmptyPath();
    void testSetAudioWithValidPath();
    void testAudioPathPreservedAfterMultipleSets();

    // Playback control tests
    void testPlayStopTransition();
    void testPlayIdempotency();
    void testStopIdempotency();
    void testRefreshWithActiveInput();
    void testRefreshWithInactiveInput();

    // Mute control tests
    void testMute();
    void testUnmute();
    void testMuteWithoutAudioDevice();

    // Serialization tests
    void testSaveAudioPath();
    void testLoadVersionOld();
    void testLoadVersionNew();
};
