// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestAudioService : public QObject
{
    Q_OBJECT

private slots:
    void testThePushPlaysAndStopsABuzzer();
    void testThePushTellsTheHostWhatItStartedAndStopped();
    void testALoadLeavesAPlayingVoiceAsItIs();
    void testAnAudioBoxPlaysItsFileOrTheBuiltInSound();
    void testMuteIsPerDocument();
    void testAnElementAddedToAMutedDocumentStartsMuted();
    void testAPauseMutesAndAResumeTakesTheUsersMute();
    void testAParameterChangedReachesTheVoice();
    void testAnElementRemovedStopsAndOneBackPlays();
    void testADestroyedElementDropsItsVoice();
    void testOnlyAnAudioElementThatPlayedGetsAVoice();
    void testTheHardwareFollowsTheVoice();
};
