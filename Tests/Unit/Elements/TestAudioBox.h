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

    // Hardware-detection seam tests (deterministic, environment-independent)
    void testMuteWithoutOutputDevice();
    void testSetAudioWithoutOutputDeviceSkipsHardwareSetup();
    void testSetAudioRestartsPlaybackWhenPlaying();

    // Serialization tests
    void testSaveAudioPath();
    void testLoadVersionOld();
    void testLoadVersionNew();
    void testLoadBeforeAudioExisted();
    void testLoadOldFormatBareAudioPath();

    // Path resolution (mechanics tested directly in Tests/Unit/Core/TestExternalFilePath.cpp;
    // these confirm AudioBox itself is wired to it correctly)
    void testSetAudioWithAbsolutePath();
    void testSetAudioWithNonExistentPathThrows();
    void testLoadResolvesBareFilenameAgainstContextDir();
    void testLoadResolvesForeignPathViaBareFilenameFallback();
    void testSaveStoresFilenameOnlyInProjectDir();

    // UpdateCommand undo/redo round-trip
    void testUpdateCommandUndoRedoPreservesFullAudioPath();
};
