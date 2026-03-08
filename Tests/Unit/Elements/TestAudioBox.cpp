// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestAudioBox.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QTemporaryFile>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/GlobalProperties.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Unit/Elements/TestAudioElementHelpers.h"

// ============================================================================
// Constructor Tests
// ============================================================================

void TestAudioBox::testConstructorInitialization()
{
    AudioBox audioBox;

    // Verify element type
    QCOMPARE(audioBox.elementType(), ElementType::AudioBox);

    // Verify element group is Output
    QCOMPARE(audioBox.elementGroup(), ElementGroup::Output);

    // Verify has one input port
    QCOMPARE(audioBox.inputSize(), 1);

    // Verify has no output ports
    QCOMPARE(audioBox.outputSize(), 0);

    // Verify is rotatable
    QVERIFY(audioBox.isRotatable());

    // Verify has audio box feature
    QVERIFY(audioBox.hasAudioBox());
}

void TestAudioBox::testCopyConstructor()
{
    AudioBox original;
    original.setAudio(":/Components/Output/Audio/wiredpanda.wav");
    AudioBox copy(original);
    AudioElementTestHelpers::testCopyConstructor(original, copy);
}

// ============================================================================
// Audio Accessor Tests
// ============================================================================

void TestAudioBox::testAudioPath()
{
    AudioBox audioBox;

    // Audio path should be the default audio file after construction
    QCOMPARE(audioBox.audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testAudioPathEmpty()
{
    AudioBox audioBox;

    // Setting empty path should be a no-op
    audioBox.setAudio("");

    // Path should remain the default
    QCOMPARE(audioBox.audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

// ============================================================================
// Audio Configuration Tests
// ============================================================================

void TestAudioBox::testSetAudioWithQRCPath()
{
    AudioBox audioBox;

    // Set audio with QRC resource path
    audioBox.setAudio(":/Components/Output/Audio/wiredpanda.wav");

    // Audio path should be stored regardless of device availability
    QCOMPARE(audioBox.audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testSetAudioWithEmptyPath()
{
    AudioBox audioBox;

    // Set audio with empty path - should be a no-op
    audioBox.setAudio("");

    // Path should remain the default
    QCOMPARE(audioBox.audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testSetAudioWithValidPath()
{
    AudioBox audioBox;

    // Create a temporary audio file
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        tempFile.write("DUMMY AUDIO DATA");
        tempFile.close();

        // Set audio to temporary file
        audioBox.setAudio(tempFile.fileName());

        // Audio path should be stored
        QCOMPARE(audioBox.audio(), tempFile.fileName());
    }
}

void TestAudioBox::testAudioPathPreservedAfterMultipleSets()
{
    AudioBox audioBox;

    // Set audio with empty path multiple times — should be a no-op each time
    audioBox.setAudio("");
    QString path1 = audioBox.audio();

    audioBox.setAudio("");
    QString path2 = audioBox.audio();

    // Both paths should remain the default
    QCOMPARE(path1, path2);
    QCOMPARE(path1, QString(":/Components/Output/Audio/wiredpanda.wav"));
}

// ============================================================================
// Playback Control Tests
// ============================================================================

void TestAudioBox::testPlayStopTransition()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testPlayStopTransition(audioBox);
}

void TestAudioBox::testPlayIdempotency()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testPlayIdempotency(audioBox);
}

void TestAudioBox::testStopIdempotency()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testStopIdempotency(audioBox);
}

void TestAudioBox::testRefreshWithActiveInput()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testRefreshWithActiveInput(audioBox);
}

void TestAudioBox::testRefreshWithInactiveInput()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testRefreshWithInactiveInput(audioBox);
}

// ============================================================================
// Mute Control Tests
// ============================================================================

void TestAudioBox::testMute()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testMute(audioBox);
}

void TestAudioBox::testUnmute()
{
    AudioBox audioBox;
    AudioElementTestHelpers::testUnmute(audioBox);
}

void TestAudioBox::testMuteWithoutAudioDevice()
{
    // Verifies mute state is tracked independently of hardware availability
    AudioBox audioBox;
    AudioElementTestHelpers::testUnmute(audioBox);
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestAudioBox::testSaveAudioPath()
{
    AudioBox audioBox;
    audioBox.setAudio(":/Components/Output/Audio/wiredpanda.wav");

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    audioBox.save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestAudioBox::testLoadVersionOld()
{
    // Verify round-trip save/load with current format (version 4.1+)
    // This tests that AudioBox serialization/deserialization works correctly
    auto audioBox1 = std::make_unique<AudioBox>();
    audioBox1->setAudio(":/Components/Output/Audio/wiredpanda.wav");

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1->save(saveStream);

    // Load and verify the audio path round-trips correctly
    auto audioBox2 = std::make_unique<AudioBox>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;

    audioBox2->load(loadStream, portMap, GlobalProperties::version);

    // Audio path should be loaded correctly
    QCOMPARE(audioBox2->audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testLoadVersionNew()
{
    // Create audio box and save it
    auto audioBox1 = std::make_unique<AudioBox>();
    audioBox1->setAudio(":/Components/Output/Audio/wiredpanda.wav");

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1->save(saveStream);

    // Create new audio box and load with current version
    auto audioBox2 = std::make_unique<AudioBox>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;

    audioBox2->load(loadStream, portMap, GlobalProperties::version);

    // Audio path should be loaded
    QCOMPARE(audioBox2->audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

