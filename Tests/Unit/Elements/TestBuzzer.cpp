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
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Unit/Elements/TestAudioElementHelpers.h"

// ============================================================================
// Constructor Tests
// ============================================================================

void TestBuzzer::testConstructorInitialization()
{
    Buzzer buzzer;

    QCOMPARE(buzzer.elementType(), ElementType::Buzzer);
    QCOMPARE(buzzer.elementGroup(), ElementGroup::Output);
    QCOMPARE(buzzer.inputSize(), 1);
    QCOMPARE(buzzer.outputSize(), 0);
    QVERIFY(buzzer.hasFrequency());
    QVERIFY(buzzer.hasVolume());
    QVERIFY(buzzer.hasLabel());
    QVERIFY(!buzzer.isRotatable());
    QVERIFY(buzzer.canChangeAppearance());
}

void TestBuzzer::testCopyConstructor()
{
    Buzzer original;
    original.setFrequency(440.0);
    Buzzer copy(original);
    AudioElementTestHelpers::testCopyConstructor(original, copy);
}

// ============================================================================
// Frequency Tests
// ============================================================================

void TestBuzzer::testDefaultFrequency()
{
    Buzzer buzzer;
    QCOMPARE(buzzer.frequency(), 1047.0);
}

void TestBuzzer::testFrequency()
{
    Buzzer buzzer;

    buzzer.setFrequency(440.0);
    QCOMPARE(buzzer.frequency(), 440.0);
}

void TestBuzzer::testSetFrequency()
{
    Buzzer buzzer;

    buzzer.setFrequency(880.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(2000.0);
    QCOMPARE(buzzer.frequency(), 2000.0);
}

void TestBuzzer::testSetAudioBackwardCompat()
{
    Buzzer buzzer;

    // setAudio("C6") should set frequency to 1047
    buzzer.setAudio("C6");
    QCOMPARE(buzzer.frequency(), 1047.0);

    // setAudio("A7") should set frequency to 3520
    buzzer.setAudio("A7");
    QCOMPARE(buzzer.frequency(), 3520.0);
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

void TestBuzzer::testSaveFrequency()
{
    Buzzer buzzer;
    buzzer.setFrequency(440.0);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    buzzer.save(stream);

    QVERIFY(data.size() > 0);
}

void TestBuzzer::testLoadVersionOld()
{
    // Load an actual old format file (v4.0.0-rc2 uses version < 4.1 format)
    QString filePath = TestUtils::backwardCompatibilityDir() + "v4.0.0-rc2/notes.panda";

    QFileInfo fileInfo(filePath);

    QFile pandaFile(filePath);
    QVERIFY2(pandaFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Failed to open %1: %2")
                       .arg(pandaFile.fileName(), pandaFile.errorString())));

    WorkSpace workspace;
    QDataStream stream(&pandaFile);

    try {
        QVersionNumber version = Serialization::readPandaHeader(stream);
        workspace.load(stream, version, fileInfo.absolutePath());

        const auto elements = workspace.scene()->elements();
        bool foundBuzzer = false;

        for (GraphicElement *elem : elements) {
            if (elem->elementType() == ElementType::Buzzer) {
                auto *buzzer = dynamic_cast<Buzzer *>(elem);
                if (buzzer) {
                    foundBuzzer = true;
                    QVERIFY(buzzer->elementType() == ElementType::Buzzer);
                    // Old files stored note names — verify they converted to valid frequencies
                    QVERIFY(buzzer->frequency() > 0.0);
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
    // Create buzzer and save it with a custom frequency
    auto buzzer1 = std::make_unique<Buzzer>();
    buzzer1->setFrequency(440.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    buzzer1->save(saveStream);

    // Create new buzzer and load with current version
    auto buzzer2 = std::make_unique<Buzzer>();

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, FileVersion::current, {}};

    buzzer2->load(loadStream, context);

    QCOMPARE(buzzer2->frequency(), 440.0);
}

