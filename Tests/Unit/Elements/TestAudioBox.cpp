// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestAudioBox.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QKeySequence>
#include <QPointF>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
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
    Element audioBox{ElementType::AudioBox};

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
    QVERIFY(audioBox.offersParam(ParamId::AudioFile));
}

// ============================================================================
// Audio Accessor Tests
// ============================================================================

void TestAudioBox::testAudioPath()
{
    Element audioBox{ElementType::AudioBox};

    // Audio path should be the default audio file after construction
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testAudioPathEmpty()
{
    Element audioBox{ElementType::AudioBox};

    // Setting empty path should be a no-op
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(""));

    // Path should remain the default
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

// ============================================================================
// Audio Configuration Tests
// ============================================================================

void TestAudioBox::testSetAudioWithQRCPath()
{
    Element audioBox{ElementType::AudioBox};

    // Set audio with QRC resource path
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(":/Components/Output/Audio/wiredpanda.wav"));

    // Audio path should be stored regardless of device availability
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testAResourcePathIsNeverReportedAsAFileThatFailed()
{
    // A resource is no file of the user's: one the build does not hold is not reported when a file opens, whatever
    // plays, as a sound file the user can restore would be
    Element audioBox{ElementType::AudioBox};
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(":/Components/Output/Audio/no-such-sound.wav"));
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/no-such-sound.wav"));
    try {
        audioBox.reportFailedFiles();
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("a resource path was reported: %1").arg(e.what())));
    }
    QVERIFY(audioBox.externalFiles().isEmpty());
}

void TestAudioBox::testSetAudioWithEmptyPath()
{
    Element audioBox{ElementType::AudioBox};

    // Set audio with empty path - should be a no-op
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(""));

    // Path should remain the default
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testSetAudioWithValidPath()
{
    Element audioBox{ElementType::AudioBox};

    // Create a temporary audio file
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();

    // Set audio to temporary file
    audioBox.setParam(ParamId::AudioFile, tempFile.fileName());

    // Audio path should be stored
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), tempFile.fileName());
}

void TestAudioBox::testAudioPathPreservedAfterMultipleSets()
{
    Element audioBox{ElementType::AudioBox};

    // Set audio with empty path multiple times — should be a no-op each time
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(""));
    QString path1 = audioBox.param<QString>(ParamId::AudioFile);

    audioBox.setParam(ParamId::AudioFile, QStringLiteral(""));
    QString path2 = audioBox.param<QString>(ParamId::AudioFile);

    // Both paths should remain the default
    QCOMPARE(path1, path2);
    QCOMPARE(path1, QString(":/Components/Output/Audio/wiredpanda.wav"));
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestAudioBox::testSaveAudioPath()
{
    Element audioBox{ElementType::AudioBox};
    audioBox.setParam(ParamId::AudioFile, QStringLiteral(":/Components/Output/Audio/wiredpanda.wav"));

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
    auto audioBox1 = std::make_unique<Element>(ElementType::AudioBox);
    audioBox1->setParam(ParamId::AudioFile, QStringLiteral(":/Components/Output/Audio/wiredpanda.wav"));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load and verify the audio path round-trips correctly
    auto audioBox2 = std::make_unique<Element>(ElementType::AudioBox);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, {}};
    audioBox2->load(loadStream, context);

    // Audio path should be loaded correctly
    QCOMPARE(audioBox2->param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testLoadVersionNew()
{
    // Create audio box and save it
    auto audioBox1 = std::make_unique<Element>(ElementType::AudioBox);
    audioBox1->setParam(ParamId::AudioFile, QStringLiteral(":/Components/Output/Audio/wiredpanda.wav"));

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1->save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Create new audio box and load with current version
    auto audioBox2 = std::make_unique<Element>(ElementType::AudioBox);

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, {}};
    audioBox2->load(loadStream, context);

    // Audio path should be loaded
    QCOMPARE(audioBox2->param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
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

    Element audioBox{ElementType::AudioBox};
    audioBox.setParam(ParamId::AudioFile, QFileInfo(tempFile.fileName()).absoluteFilePath());

    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QFileInfo(tempFile.fileName()).absoluteFilePath());
}

void TestAudioBox::testSetAudioWithNonExistentPathKeepsItAndPlaysTheDefault()
{
    // A sound that is gone keeps its path and plays the built-in sound; the sound comes back when its file does, and
    // only a file's report names it
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("gone.wav");

    Element audioBox{ElementType::AudioBox};
    try {
        audioBox.setParam(ParamId::AudioFile, path);
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("setAudio() threw: %1").arg(e.what())));
    }
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), path);
    QCOMPARE(audioBox.externalFiles(), QStringList{path});

    QString message;
    try {
        audioBox.reportFailedFiles();
    } catch (const Pandaception &e) {
        message = QString::fromUtf8(e.what());
    }
    QVERIFY2(message.contains("gone.wav") && message.contains("does not exist"), qPrintable(message));

    // What plays in its place is the audio service's (TestAudioService::testAnAudioBoxPlaysItsFileOrTheBuiltInSound())
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("DUMMY AUDIO DATA");
    file.close();
    try {
        audioBox.reportFailedFiles();
    } catch (const Pandaception &e) {
        QFAIL(qPrintable(QString("a sound that came back is still reported: %1").arg(e.what())));
    }
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

    Element audioBox1{ElementType::AudioBox};
    audioBox1.setParam(ParamId::AudioFile, tempInfo.absoluteFilePath());

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    Element audioBox2{ElementType::AudioBox};
    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, tempInfo.absolutePath()};
    audioBox2.load(loadStream, context);

    QCOMPARE(audioBox2.param<QString>(ParamId::AudioFile), tempInfo.absoluteFilePath());
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

    Element audioBox1{ElementType::AudioBox};
    audioBox1.setParam(ParamId::AudioFile, tempInfo.absoluteFilePath());

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

    Element audioBox2{ElementType::AudioBox};
    QDataStream loadStream(rewritten);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile, tempInfo.absolutePath()};
    audioBox2.load(loadStream, context);

    QCOMPARE(audioBox2.param<QString>(ParamId::AudioFile), tempInfo.absoluteFilePath());
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

    Element audioBox{ElementType::AudioBox};
    audioBox.setParam(ParamId::AudioFile, tempInfo.absoluteFilePath());

    // Verify internal state stores the full resolved path
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), tempInfo.absoluteFilePath());

    // Save and read back the serialized audio path
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    audioBox.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Skip past the Element base data to reach the AudioBox map.
    // Element::save writes: QMap (properties), QList<QMap> (inputs),
    // QList<QMap> (outputs), QList<QMap> (appearances). Then AudioBox::save adds its QMap.
    QDataStream loadStream(data);
    QMap<QString, QVariant> map;
    QList<QMap<QString, QVariant>> listMap;

    loadStream >> map;      // Element's properties map
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
    auto *audioBox = ElementFactory::buildElement(ElementType::AudioBox);
    workspace.scene()->addItem(audioBox);
    const int id = audioBox->id();

    QVERIFY(audioBox->externalFiles().isEmpty()); // starts on the default (resource) audio

    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        audioBox->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    audioBox->setParam(ParamId::AudioFile, audioPath);
    QCOMPARE(audioBox->externalFiles(), QStringList{audioPath});

    workspace.scene()->undoStack()->push(new UpdateCommand({audioBox}, oldData, workspace.scene()));

    // undo() must restore the default (resource) audio, not just leave the custom one
    workspace.scene()->undoStack()->undo();
    QVERIFY(dynamic_cast<Element *>(workspace.scene()->itemById(id))->externalFiles().isEmpty());

    // redo() reapplies the custom audio path, full and unstripped
    workspace.scene()->undoStack()->redo();
    QCOMPARE(dynamic_cast<Element *>(workspace.scene()->itemById(id))->externalFiles(), QStringList{audioPath});
}

void TestAudioBox::testLoadBeforeAudioExisted()
{
    // AudioBox was introduced at V_2_4 (see VersionInfo::hasAudio) -- no real pre-2.4 .panda
    // file can ever contain one, so this guard clause is otherwise unreachable by any fixture.
    Element audioBox{ElementType::AudioBox};

    AudioElementTestHelpers::testLoadBeforeAudioExisted(audioBox);

    // load() must have returned before touching anything AudioBox-specific.
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), QString(":/Components/Output/Audio/wiredpanda.wav"));
}

void TestAudioBox::testLoadOldFormatBareAudioPath()
{
    // No bundled backward-compatibility fixture happens to contain an AudioBox (unlike
    // Buzzer's "notes.panda"), so the v2.4-4.0 bare-QString load branch is hand-built here,
    // mirroring ElementSerializer::loadOldFormat()'s exact old-format field order.
    //
    // A real (non-resource) absolute path is used, not a ":/..." resource string: per
    // ExternalFilePath::forReading(), a resource path always resolves to std::nullopt for
    // PortableFile loads, which would leave audio() at its unchanged default and make this
    // test pass without ever actually reaching the setAudio() call it's meant to cover.
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("DUMMY AUDIO DATA");
    tempFile.close();
    const QString absolutePath = QFileInfo(tempFile.fileName()).absoluteFilePath();

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QPointF(5.0, 5.0);     // loadPos
    writeStream << 0.0;                   // loadRotation (qreal)
    writeStream << QString("old label");  // loadLabel (hasLabels since V_1_2)
    writeStream << quint64(1) << quint64(1) << quint64(1) << quint64(1); // loadPortsSize (V_1_3)
    writeStream << QKeySequence();        // loadTrigger (V_1_9)
    writeStream << quint64(0);            // loadInputPorts: inputSize = 0
    writeStream << quint64(0);            // loadOutputPorts: outputSize = 0
    // hasAppearanceNames() is V_2_7 -- false at V_2_4, so nothing further is written there.
    writeStream << absolutePath;           // bare audio path

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_2_4, SerializationPurpose::PortableFile};

    Element audioBox{ElementType::AudioBox};
    QDataStream readStream(data);
    audioBox.load(readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(audioBox.param<QString>(ParamId::AudioFile), absolutePath);
}
