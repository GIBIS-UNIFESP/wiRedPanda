// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestBuzzer.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Buzzer.h"
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

void TestBuzzer::testConstructorInitialization()
{
    Buzzer buzzer;

    // Verify element type
    QCOMPARE(buzzer.elementType(), ElementType::Buzzer);

    // Verify element group is Output
    QCOMPARE(buzzer.elementGroup(), ElementGroup::Output);

    // Verify has one input port
    QCOMPARE(buzzer.inputSize(), 1);

    // Verify has no output ports
    QCOMPARE(buzzer.outputSize(), 0);

    // Verify has audio property
    QVERIFY(buzzer.hasAudio());

    // Verify has label
    QVERIFY(buzzer.hasLabel());

    // Verify cannot rotate
    QVERIFY(!buzzer.isRotatable());

    // Verify can change skin
    QVERIFY(buzzer.canChangeSkin());
}

void TestBuzzer::testCopyConstructor()
{
    Buzzer original;
    original.setAudio("C6");
    Buzzer copy(original);
    AudioElementTestHelpers::testCopyConstructor(original, copy);
}

// ============================================================================
// Audio Accessor Tests
// ============================================================================

void TestBuzzer::testAudioNote()
{
    Buzzer buzzer;

    // Set a note and verify it's stored
    buzzer.setAudio("E6");
    QCOMPARE(buzzer.audio(), QString("E6"));
}

void TestBuzzer::testAudioNoteEmpty()
{
    Buzzer buzzer;

    // Default constructed buzzer should have empty note
    QCOMPARE(buzzer.audio(), QString{});
}

// ============================================================================
// Audio Configuration Tests
// ============================================================================

void TestBuzzer::testSetAudioWithValidNote()
{
    Buzzer buzzer;

    // Set audio with valid note name
    buzzer.setAudio("C6");

    // Audio note should be set regardless of device availability
    QCOMPARE(buzzer.audio(), QString("C6"));
}

void TestBuzzer::testSetAudioWithEmptyNote()
{
    Buzzer buzzer;

    // Set audio with empty note - should return early without changing note
    buzzer.setAudio("");

    // Note should remain empty
    QCOMPARE(buzzer.audio(), QString{});
}

void TestBuzzer::testAudioPersistence()
{
    Buzzer buzzer;

    // Set audio multiple times and verify persistence
    buzzer.setAudio("D5");
    QString note1 = buzzer.audio();
    QCOMPARE(note1, QString("D5"));

    buzzer.setAudio("E6");
    QString note2 = buzzer.audio();
    QCOMPARE(note2, QString("E6"));
}

// ============================================================================
// Playback Control Tests
// ============================================================================

void TestBuzzer::testPlayStopTransition()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testPlayStopTransition(buzzer);
}

void TestBuzzer::testPlayIdempotency()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testPlayIdempotency(buzzer);
}

void TestBuzzer::testStopIdempotency()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testStopIdempotency(buzzer);
}

void TestBuzzer::testRefreshWithActiveInput()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testRefreshWithActiveInput(buzzer);
}

void TestBuzzer::testRefreshWithInactiveInput()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testRefreshWithInactiveInput(buzzer);
}

// ============================================================================
// Mute Control Tests
// ============================================================================

void TestBuzzer::testMute()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testMute(buzzer);
}

void TestBuzzer::testUnmute()
{
    Buzzer buzzer;
    AudioElementTestHelpers::testUnmute(buzzer);
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestBuzzer::testSaveNote()
{
    Buzzer buzzer;
    buzzer.setAudio("C6");

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    buzzer.save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestBuzzer::testLoadVersionOld()
{
    // Load an actual old format file (v4.0.0-rc2 uses version < 4.1 format)
    QString filePath = TestUtils::backwardCompatibilityDir() + "v4.0.0-rc2/notes.panda";

    QFileInfo fileInfo(filePath);
    GlobalProperties::currentDir = fileInfo.absolutePath();

    QFile pandaFile(filePath);
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(pandaFile.fileName(), pandaFile.errorString())));

    WorkSpace workspace;
    QDataStream stream(&pandaFile);

    try {
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version);

        // Verify that buzzers loaded successfully from old format
        const auto elements = workspace.scene()->elements();
        bool foundBuzzer = false;

        for (GraphicElement *elem : elements) {
            if (elem->elementType() == ElementType::Buzzer) {
                Buzzer *buzzer = dynamic_cast<Buzzer *>(elem);
                if (buzzer) {
                    foundBuzzer = true;
                    // Just verify that the buzzer loaded without errors
                    // and is a valid Buzzer object
                    QVERIFY(buzzer->elementType() == ElementType::Buzzer);
                }
            }
        }

        QVERIFY(foundBuzzer);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load backward compatibility file: %1").arg(e.what())));
    }

    pandaFile.close();
}

void TestBuzzer::testLoadVersionNew()
{
    // Create buzzer and save it
    auto buzzer1 = std::make_unique<Buzzer>();
    buzzer1->setAudio("E6");

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    buzzer1->save(saveStream);

    // Create new buzzer and load with current version
    auto buzzer2 = std::make_unique<Buzzer>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;

    buzzer2->load(loadStream, portMap, GlobalProperties::version);

    // Audio note should be loaded
    QCOMPARE(buzzer2->audio(), QString("E6"));
}

