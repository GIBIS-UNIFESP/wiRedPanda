// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestBuzzer.h"

#include <limits>
#include <memory>

#include <QDataStream>
#include <QFile>
#include <QTest>

#include "App/Element/AudioOutput.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Unit/Elements/TestAudioElementHelpers.h"

// ============================================================================
// Constructor Tests
// ============================================================================

void TestBuzzer::testConstructorInitialization()
{
    Element buzzer{ElementType::Buzzer};

    QCOMPARE(buzzer.elementType(), ElementType::Buzzer);
    QCOMPARE(buzzer.elementGroup(), ElementGroup::Output);
    QCOMPARE(buzzer.inputSize(), 1);
    QCOMPARE(buzzer.outputSize(), 0);
    QVERIFY(buzzer.hasFrequency());
    QVERIFY(buzzer.hasVolume());
    QVERIFY(buzzer.hasLabel());
    QVERIFY(!buzzer.rotatesGraphic());
    QVERIFY(buzzer.canChangeAppearance());
}

// ============================================================================
// Frequency Tests
// ============================================================================

void TestBuzzer::testDefaultFrequency()
{
    Element buzzer{ElementType::Buzzer};
    QCOMPARE(buzzer.frequency(), 1047.0);
}

void TestBuzzer::testFrequency()
{
    Element buzzer{ElementType::Buzzer};

    buzzer.setFrequency(440.0);
    QCOMPARE(buzzer.frequency(), 440.0);
}

void TestBuzzer::testSetFrequency()
{
    Element buzzer{ElementType::Buzzer};

    buzzer.setFrequency(880.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(2000.0);
    QCOMPARE(buzzer.frequency(), 2000.0);
}

void TestBuzzer::testSetFrequencyRejectsNaN()
{
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(880.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    // NaN must be rejected, not stored — ToneGenerator::readData() casts a NaN-derived
    // sample to qint16, which is undefined behaviour.
    buzzer.setFrequency(std::numeric_limits<double>::quiet_NaN());
    QCOMPARE(buzzer.frequency(), 880.0);
}

void TestBuzzer::testSetFrequencyRejectsInfinity()
{
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(880.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(std::numeric_limits<double>::infinity());
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(-std::numeric_limits<double>::infinity());
    QCOMPARE(buzzer.frequency(), 880.0);
}

void TestBuzzer::testSetFrequencyRejectsZeroAndNegative()
{
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(880.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(0.0);
    QCOMPARE(buzzer.frequency(), 880.0);

    buzzer.setFrequency(-100.0);
    QCOMPARE(buzzer.frequency(), 880.0);
}

void TestBuzzer::testSetNote()
{
    Element buzzer{ElementType::Buzzer};

    // setNote("C6") should set frequency to 1047
    AudioOutput::setNote(buzzer, "C6");
    QCOMPARE(buzzer.frequency(), 1047.0);

    // setNote("A7") should set frequency to 3520
    AudioOutput::setNote(buzzer, "A7");
    QCOMPARE(buzzer.frequency(), 3520.0);
}

void TestBuzzer::testToneScaleIsMonotonic()
{
    // The tone picker presents these eight notes as an ascending scale; every step
    // must rise in pitch (regression guard for the A7/B7 octave-jump bug).
    const QStringList scale = {"C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7"};
    int previous = 0;
    for (const QString &note : scale) {
        const int freq = AudioOutput::noteToFrequency(note);
        QVERIFY2(freq > previous,
                 qPrintable(QString("Note %1 (%2 Hz) does not ascend above the previous note (%3 Hz)")
                            .arg(note).arg(freq).arg(previous)));
        previous = freq;
    }

    // The corrected sixth/seventh degrees sit in the same octave as their neighbours.
    QCOMPARE(AudioOutput::noteToFrequency("A6"), 1760);
    QCOMPARE(AudioOutput::noteToFrequency("B6"), 1976);

    // Legacy note names remain resolvable to their true (higher-octave) pitches.
    QCOMPARE(AudioOutput::noteToFrequency("A7"), 3520);
    QCOMPARE(AudioOutput::noteToFrequency("B7"), 3951);
}

void TestBuzzer::testSetNoteWithEmptyNoteIsNoOp()
{
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(880.0);

    AudioOutput::setNote(buzzer, "");
    QCOMPARE(buzzer.frequency(), 880.0);
}

void TestBuzzer::testSetNoteOffTheScaleIsC6()
{
    // A name not on the scale, which an older file can hold, is C6's tone
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(880.0);

    AudioOutput::setNote(buzzer, "A4");
    QCOMPARE(buzzer.frequency(), 1047.0);
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestBuzzer::testSaveFrequency()
{
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(440.0);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    buzzer.save(stream, {.purpose = SerializationPurpose::PortableFile});

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

        for (Element *elem : elements) {
            if (elem->elementType() == ElementType::Buzzer) {
                foundBuzzer = true;
                // Old files stored note names — verify they converted to valid frequencies
                QVERIFY(elem->frequency() > 0.0);
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
    auto buzzer1 = std::make_unique<Element>(ElementType::Buzzer);
    buzzer1->setFrequency(440.0);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    buzzer1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Create new buzzer and load with current version
    auto buzzer2 = std::make_unique<Element>(ElementType::Buzzer);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, {}};

    buzzer2->load(loadStream, context);

    QCOMPARE(buzzer2->frequency(), 440.0);
}

void TestBuzzer::testLoadBeforeAudioExisted()
{
    // Buzzer was introduced at V_2_4 (see VersionInfo::hasAudio) -- no real pre-2.4 .panda
    // file can ever contain one, so this guard clause is otherwise unreachable by any fixture.
    Element buzzer{ElementType::Buzzer};
    buzzer.setFrequency(1234.0);

    AudioElementTestHelpers::testLoadBeforeAudioExisted(buzzer);

    // load() must have returned before touching anything Buzzer-specific.
    QCOMPARE(buzzer.frequency(), 1234.0);
}
