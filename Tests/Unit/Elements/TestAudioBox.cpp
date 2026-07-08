// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestAudioBox.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputGND.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
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
    QVERIFY(audioBox.rotatesGraphic());

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
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();

    // Set audio to temporary file
    audioBox.setAudio(tempFile.fileName());

    // Audio path should be stored
    QCOMPARE(audioBox.audio(), tempFile.fileName());
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

    audioBox.save(stream, {.purpose = SerializationPurpose::PortableFile});

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
    audioBox1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load and verify the audio path round-trips correctly
    auto audioBox2 = std::make_unique<AudioBox>();

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, {}};
    audioBox2->load(loadStream, context);

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
    audioBox1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Create new audio box and load with current version
    auto audioBox2 = std::make_unique<AudioBox>();

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, {}};
    audioBox2->load(loadStream, context);

    // Audio path should be loaded
    QCOMPARE(audioBox2->audio(), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testSetAudioWithAbsolutePath()
{
    // setAudio() takes a directly-usable path as-is -- no contextDir resolution of its
    // own (that now happens once, at load time, via ExternalFilePath::forReading(); see
    // testLoadResolvesBareFilenameAgainstContextDir() below and Tests/Unit/Core/TestExternalFilePath.cpp
    // for the resolution mechanics themselves).
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();

    AudioBox audioBox;
    audioBox.setAudio(QFileInfo(tempFile.fileName()).absoluteFilePath());

    QCOMPARE(audioBox.audio(), QFileInfo(tempFile.fileName()).absoluteFilePath());
}

void TestAudioBox::testSetAudioWithNonExistentPathThrows()
{
    AudioBox audioBox;

    bool threw = false;
    try {
        audioBox.setAudio("/some/empty/directory/nonexistent_audio_12345.wav");
    } catch (const Pandaception &) {
        threw = true;
    }

    QVERIFY2(threw, "setAudio should throw when the given path does not exist");
}

void TestAudioBox::testLoadResolvesBareFilenameAgainstContextDir()
{
    // Integration check that AudioBox::load() is wired to ExternalFilePath::forReading():
    // a PortableFile save() strips a non-resource path to a bare filename; loading it
    // back with contextDir pointing at that file's directory must resolve the full path.
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();
    const QFileInfo tempInfo(tempFile.fileName());

    AudioBox audioBox1;
    audioBox1.setAudio(tempInfo.absoluteFilePath());

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    AudioBox audioBox2;
    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, tempInfo.absolutePath()};
    audioBox2.load(loadStream, context);

    QCOMPARE(audioBox2.audio(), tempInfo.absoluteFilePath());
}

void TestAudioBox::testLoadResolvesForeignPathViaBareFilenameFallback()
{
    // A stored Windows-style absolute path (saved on a different OS/machine) doesn't
    // exist locally; load() must fall back to a bare-filename lookup in contextDir.
    // See Tests/Unit/Core/TestExternalFilePath.cpp for the exhaustive separator matrix --
    // this just confirms AudioBox::load() actually exercises that fallback.
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();
    const QFileInfo tempInfo(tempFile.fileName());

    AudioBox audioBox1;
    audioBox1.setAudio(tempInfo.absoluteFilePath());

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Rewrite the stored "audiobox" path to a foreign, locally-nonexistent Windows-style
    // absolute path with the same bare filename, so it's load()'s fallback -- not the
    // original real path -- that has to resolve it.
    QMap<QString, QVariant> propsMap;
    QList<QMap<QString, QVariant>> inputPorts, outputPorts, appearances;
    QMap<QString, QVariant> audioMap;
    {
        QDataStream readStream(data);
        readStream >> propsMap >> inputPorts >> outputPorts >> appearances >> audioMap;
    }
    audioMap["audiobox"] = "C:\\Users\\alice\\project\\" + tempInfo.fileName();

    QByteArray rewritten;
    {
        QDataStream writeStream(&rewritten, QIODevice::WriteOnly);
        writeStream << propsMap << inputPorts << outputPorts << appearances << audioMap;
    }

    AudioBox audioBox2;
    QDataStream loadStream(rewritten);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, tempInfo.absolutePath()};
    audioBox2.load(loadStream, context);

    QCOMPARE(audioBox2.audio(), tempInfo.absoluteFilePath());
}

void TestAudioBox::testSaveStoresFilenameOnlyInProjectDir()
{
    // When the audio file lives inside the project directory, save() should
    // store only the bare filename for portability
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();

    const QFileInfo tempInfo(tempFile.fileName());

    AudioBox audioBox;
    audioBox.setAudio(tempInfo.absoluteFilePath());

    // Verify internal state stores the full resolved path
    QCOMPARE(audioBox.audio(), tempInfo.absoluteFilePath());

    // Save and read back the serialized audio path
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Skip past the GraphicElement base data to reach the AudioBox map.
    // GraphicElement::save writes: QMap (properties), QList<QMap> (inputs),
    // QList<QMap> (outputs), QList<QMap> (appearances). Then AudioBox::save adds its QMap.
    QDataStream loadStream(data);
    QMap<QString, QVariant> map;
    QList<QMap<QString, QVariant>> listMap;

    loadStream >> map;      // GraphicElement's properties map
    loadStream >> listMap;  // input ports
    loadStream >> listMap;  // output ports
    loadStream >> listMap;  // appearances
    loadStream >> map;      // AudioBox's map

    QVERIFY2(map.contains("audiobox"), "Saved data should contain 'audiobox' key");
    const QString savedPath = map.value("audiobox").toString();
    QCOMPARE(savedPath, tempInfo.fileName());
}

void TestAudioBox::testUpdateCommandUndoRedoPreservesFullAudioPath()
{
    // UpdateCommand snapshots are InMemorySnapshot serialization, which must round-trip
    // the full audio path losslessly (mirrors TestSceneUndoredo's equivalent Led test),
    // with no contextDir ever having been set.
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();
    const QString audioPath = QFileInfo(tempFile.fileName()).absoluteFilePath();

    WorkSpace workspace;
    auto *audioBox = new AudioBox();
    workspace.scene()->addItem(audioBox);
    const int id = audioBox->id();
    auto *audioElm = dynamic_cast<GraphicElement *>(audioBox);

    QVERIFY(audioElm->externalFiles().isEmpty()); // starts on the default (resource) audio

    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        audioElm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    audioBox->setAudio(audioPath);
    QCOMPARE(audioElm->externalFiles(), QStringList{audioPath});

    workspace.scene()->undoStack()->push(new UpdateCommand({audioElm}, oldData, workspace.scene()));

    // undo() must restore the default (resource) audio, not just leave the custom one
    workspace.scene()->undoStack()->undo();
    QVERIFY(dynamic_cast<GraphicElement *>(workspace.scene()->itemById(id))->externalFiles().isEmpty());

    // redo() reapplies the custom audio path, full and unstripped
    workspace.scene()->undoStack()->redo();
    QCOMPARE(dynamic_cast<GraphicElement *>(workspace.scene()->itemById(id))->externalFiles(), QStringList{audioPath});
}
