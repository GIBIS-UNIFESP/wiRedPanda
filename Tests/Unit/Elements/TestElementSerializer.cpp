// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementSerializer.h"

#include <memory>

#include <QDataStream>
#include <QFile>
#include <QKeySequence>
#include <QMap>
#include <QPointF>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <QVariant>

#include "App/Core/Common.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementSerializer.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace TestElementSerializerHelpers {

/// Loads \a element from a current-format stream listing \a inputs and \a outputs unnamed ports; false if refused.
bool loadsWithPorts(Element &element, const int inputs, const int outputs)
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint32(0); // properties map: empty
    for (const int count : {inputs, outputs}) {
        writeStream << quint32(count);
        for (int i = 0; i < count; ++i) {
            writeStream << quint32(0); // each port entry is an (empty) metadata map
        }
    }
    writeStream << quint32(0); // appearances: none
    writeStream << quint32(0); // a truth table's own map: empty; other types leave it unread

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};
    QDataStream readStream(data);
    try {
        element.load(readStream, context);
    } catch (const Pandaception &) {
        return false;
    }
    return true;
}

using AppearanceEntries = QList<QMap<QString, QVariant>>;

/// Loads \a element from a current-format stream holding its own ports and \a appearances, for \a purpose, resolving
/// file names against \a contextDir; false if refused.
bool loadsAppearances(Element &element, const AppearanceEntries &appearances, const SerializationPurpose purpose,
                      const QString &contextDir = {})
{
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QMap<QString, QVariant>();
    writeStream << AppearanceEntries(element.inputSize()) << AppearanceEntries(element.outputSize());
    writeStream << appearances;
    writeStream << QMap<QString, QVariant>(); // a truth table's own map: empty; other types leave it unread

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, purpose};
    context.contextDir = contextDir;
    QDataStream readStream(data);
    try {
        element.load(readStream, context);
    } catch (const Pandaception &) {
        return false;
    }
    return true;
}

/// Returns the appearance entries \a element saves for \a purpose.
AppearanceEntries savedAppearances(const Element &element, const SerializationPurpose purpose)
{
    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        element.save(stream, {.purpose = purpose});
    }
    QDataStream stream(data);
    QMap<QString, QVariant> properties;
    AppearanceEntries inputs;
    AppearanceEntries outputs;
    AppearanceEntries appearances;
    stream >> properties >> inputs >> outputs >> appearances;
    return appearances;
}

/// Returns the bytes \a element saves for \a purpose.
QByteArray savedBytes(const Element &element, const SerializationPurpose purpose)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    element.save(stream, {.purpose = purpose});
    return data;
}

/// Writes a 64x64 picture to \a path and returns whether it was written.
bool writePicture(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly)
        && (file.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"/>") > 0);
}

} // namespace TestElementSerializerHelpers

using TestElementSerializerHelpers::AppearanceEntries;
using TestElementSerializerHelpers::loadsAppearances;
using TestElementSerializerHelpers::loadsWithPorts;
using TestElementSerializerHelpers::savedAppearances;
using TestElementSerializerHelpers::savedBytes;
using TestElementSerializerHelpers::writePicture;

void TestElementSerializer::testLoadRefusesPortCountsAboveTheTypeMaximum()
{
    // A truth table's key holds exactly its maximum rows and columns, so one port more reads past it when simulated
    TruthTable atMaximum;
    QVERIFY(loadsWithPorts(atMaximum, atMaximum.maxInputSize(), atMaximum.maxOutputSize()));
    QCOMPARE(atMaximum.inputSize(), 8);
    QCOMPARE(atMaximum.outputSize(), 8);

    TruthTable oneInputMore;
    QVERIFY(!loadsWithPorts(oneInputMore, oneInputMore.maxInputSize() + 1, oneInputMore.maxOutputSize()));
    TruthTable oneOutputMore;
    QVERIFY(!loadsWithPorts(oneOutputMore, oneOutputMore.maxInputSize(), oneOutputMore.maxOutputSize() + 1));
}

void TestElementSerializer::testLoadRefusesDemuxInputsItsOutputsDoNotNeed()
{
    // A Demux has its data input and the select lines its outputs need: 2 inputs for 2 outputs, 3 for 4, 4 for 8
    Demux eightOutputs;
    QVERIFY(loadsWithPorts(eightOutputs, 4, 8));
    Demux fourOutputs;
    QVERIFY(loadsWithPorts(fourOutputs, 3, 4));

    Demux missingSelectLines;
    QVERIFY(!loadsWithPorts(missingSelectLines, 2, 8));
    Demux surplusInputs;
    QVERIFY(!loadsWithPorts(surplusInputs, 4, 2));
}

void TestElementSerializer::testReadPortListCountReadFailureThrows()
{
    QByteArray data; // empty: the very first read (the count) runs past the end
    QDataStream stream(data);

    bool threw = false;
    try {
        ElementSerializer::readPortList(stream, "test");
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an empty stream must fail reading the port-list count");
}

void TestElementSerializer::testReadPortListEntryReadFailureThrows()
{
    // The entry itself is a nested readBoundedMetadata() map. Its own implausible-count guard
    // rejects a declared count with too few remaining bytes (< 9/entry) outright -- to reach the
    // *genuine* truncated-read path (readBoundedVariant() running out of bytes and silently
    // leaving the stream status bad, not throwing itself), the sub-map's own count and key must
    // both read successfully, with nothing left for the value.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint32(1);       // readPortList's own count: 1 entry
    writeStream << quint32(1);       // that entry's nested metadata-map count: 1 key/value pair
    writeStream << QString("key12"); // a real key, comfortably over the 9-byte-per-entry minimum
    // No value bytes follow: readBoundedVariant() falls back to a plain `stream >> v`, which
    // fails silently (bad status, no throw of its own) rather than reading a full value.

    QDataStream stream(data);
    bool threw = false;
    try {
        ElementSerializer::readPortList(stream, "test");
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "a truncated entry must fail, not silently return a partial list");
}

void TestElementSerializer::testRemovePortFromMapErasesMatchingEntries()
{
    And gate;
    Port *target = gate.inputPort(0);
    Port *other = gate.inputPort(1);

    QHash<quint64, Port *> portMap;
    portMap[1] = target;
    portMap[2] = other;
    portMap[3] = target; // a port may be registered under more than one legacy key

    ElementSerializer::removePortFromMap(target, portMap);

    QCOMPARE(portMap.size(), 1);
    QCOMPARE(portMap.value(2), other);
}

void TestElementSerializer::testLoadOldFormatDrainsUnusedPriorityField()
{
    // hasUnusedPriority() is V_4_0_1-only (removed in 4.1's QMap format); no version both old-format
    // *and* carrying this field is reachable via the public load() dispatch table's boundary tests
    // elsewhere, so this drives loadOldFormat() directly with that exact version.
    And gate;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QPointF(0.0, 0.0);
    writeStream << 0.0;
    writeStream << QString("label");
    writeStream << quint64(2) << quint64(8) << quint64(1) << quint64(1); // portsSize
    writeStream << QKeySequence();     // trigger
    writeStream << quint64(0xDEADBEEF); // the unused-priority field itself
    writeStream << quint64(0);         // inputPorts: 0 (keep gate's existing 2)
    writeStream << quint64(0);         // outputPorts: 0
    writeStream << quint64(0);         // appearance names: 0

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_0_1, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    ElementSerializer::loadOldFormat(gate, readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
}

void TestElementSerializer::testLoadNewFormatPropertiesStreamErrorThrows()
{
    And gate;

    // Same reasoning as testReadPortListEntryReadFailureThrows(): the count and key must both
    // read successfully (past readBoundedMetadata()'s own implausible-count guard) so the value
    // read is what genuinely runs out of bytes, leaving the stream status bad without throwing.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint32(1);       // properties metadata-map count: 1 key/value pair
    writeStream << QString("key12"); // a real key, comfortably over the 9-byte-per-entry minimum

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadNewFormat(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "a truncated properties map must fail, not silently proceed with a partial map");
}

void TestElementSerializer::testLoadNewFormatAppearanceIndexOutOfRangeThrows()
{
    // Line has exactly 1 appearance slot; declare 2 entries in the stream.
    Line line;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint32(0);  // properties map: empty
    writeStream << quint32(0);  // input ports: none
    writeStream << quint32(0);  // output ports: none
    writeStream << quint32(2);  // appearances: 2 entries, but Line only has 1 slot
    for (int i = 0; i < 2; ++i) {
        writeStream << quint32(0); // each appearance entry is itself an (empty) metadata map
    }

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadNewFormat(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "more appearance entries than the element's own slot count must be rejected");
}

void TestElementSerializer::testLoadInputPortsImplausibleCountThrows()
{
    And gate;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(100000); // far past the 1024-port ceiling

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadInputPorts(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format input port count must be rejected before allocating");
}

void TestElementSerializer::testLoadOutputPortsImplausibleCountThrows()
{
    And gate;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(100000);

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadOutputPorts(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format output port count must be rejected before allocating");
}

void TestElementSerializer::testLoadPixmapAppearanceNamesImplausibleCountThrows()
{
    Line line;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(100000);

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_0_1, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadPixmapAppearanceNames(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format appearance-name count must be rejected before allocating");
}

void TestElementSerializer::testLoadPixmapAppearanceNamesIndexOutOfRangeThrows()
{
    // Line has exactly 1 appearance slot; declare 2 names in the stream.
    Line line;

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(2);
    writeStream << QString("first.svg");
    writeStream << QString("second.svg");

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_0_1, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    bool threw = false;
    try {
        ElementSerializer::loadPixmapAppearanceNames(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "more appearance names than the element's own slot count must be rejected");
}

void TestElementSerializer::testLoadPixmapAppearanceNamesAppliesResolvedPath()
{
    Line line;

    QTemporaryFile tempFile("XXXXXX.svg");
    QVERIFY(tempFile.open());
    tempFile.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"/>");
    tempFile.close();
    const QString path = tempFile.fileName();

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(1);
    writeStream << path;

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_0_1, SerializationPurpose::PortableFile};

    QDataStream readStream(data);
    ElementSerializer::loadPixmapAppearanceNames(line, readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(line.externalFiles(), QStringList{path});
}

void TestElementSerializer::testLoadInputOutputPortRenamesExistingIcPorts()
{
    // loadInputPort()/loadOutputPort()'s "port < inputs().size()" branch (IC-only rename) is
    // only reachable when the target element *already has* ports at load time -- true for IC
    // specifically when an already-populated IC object is reloaded in place (e.g. an
    // UpdateCommand undo/redo snapshot), not for a freshly constructed one (IC::loadState() populates
    // its own ports from the sub-circuit blob *after* calling Element::loadState(), so a fresh
    // IC always has 0 ports at the moment loadInputPort()/loadOutputPort() run).
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    WorkSpace subWorkspace;
    CircuitBuilder subBuilder(subWorkspace.scene());
    InputSwitch sw1, sw2;
    And andGate;
    Led led;
    subBuilder.add(&sw1, &sw2, &andGate, &led);
    subBuilder.connect(&sw1, 0, &andGate, 0);
    subBuilder.connect(&sw2, 0, &andGate, 1);
    subBuilder.connect(&andGate, 0, &led, 0);

    const QString subPath = subDir.path() + "/reload_probe.panda";
    QCOMPARE(subWorkspace.save(subPath), WorkSpace::SaveOutcome::Saved);

    auto ic = std::make_unique<IC>();
    ic->loadFile(subPath, subDir.path());
    QVERIFY(ic->inputSize() >= 1);
    QVERIFY(ic->outputSize() >= 1);

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, FormatRev::current, SerializationPurpose::PortableFile};

    // Old-format input-port entry: ptr=1, a new name, flags=0. Reusing loadInputPorts() directly
    // on the already-populated ic exercises the "port < size()" branch for its existing port 0.
    QByteArray inputData;
    QDataStream inputWrite(&inputData, QIODevice::WriteOnly);
    inputWrite << quint64(1); // 1 input port entry, matching the IC's existing single input
    inputWrite << quint64(1) << QString("renamed_in") << 0;
    QDataStream inputRead(inputData);
    ElementSerializer::loadInputPorts(*ic, inputRead, context);
    QCOMPARE(ic->inputPort(0)->name(), QString("renamed_in"));

    QByteArray outputData;
    QDataStream outputWrite(&outputData, QIODevice::WriteOnly);
    outputWrite << quint64(1);
    outputWrite << quint64(2) << QString("renamed_out") << 0;
    QDataStream outputRead(outputData);
    ElementSerializer::loadOutputPorts(*ic, outputRead, context);
    QCOMPARE(ic->outputPort(0)->name(), QString("renamed_out"));
}

void TestElementSerializer::testAFileSavesACustomSlotByKeyAtItsPosition()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("lit.svg");
    QVERIFY(writePicture(path));
    InputSwitch input;
    input.setOn(true);
    input.setAppearance(false, path);

    // Only the custom slot, named by its key, at the position a version reading by position reads it at; an empty
    // map holds the default slot's place before it
    const AppearanceEntries saved = savedAppearances(input, SerializationPurpose::PortableFile);
    QCOMPARE(saved.size(), 2);
    QVERIFY(saved.at(0).isEmpty());
    QCOMPARE(saved.at(1), (QMap<QString, QVariant>{{"key", "on"}, {"skinName", "lit.svg"}}));

    // A file of defaults saves no entry
    const InputSwitch plain;
    QVERIFY(savedAppearances(plain, SerializationPurpose::PortableFile).isEmpty());
}

void TestElementSerializer::testASnapshotSavesEverySlotByKey()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("lit.svg");
    QVERIFY(writePicture(path));
    InputSwitch input;
    input.setOn(true);
    input.setAppearance(false, path);

    // Every slot, the default's resource path too, so restoring the snapshot restores the defaults
    const AppearanceEntries saved = savedAppearances(input, SerializationPurpose::InMemorySnapshot);
    QCOMPARE(saved, (AppearanceEntries{
                        {{"key", "off"}, {"skinName", input.appearance().defaultAppearances().at(0)}},
                        {{"key", "on"}, {"skinName", path}},
                    }));
}

void TestElementSerializer::testAKeyedEntryIsReadByItsKey()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writePicture(dir.filePath("lit.svg")));

    // The key names the slot wherever the entry sits: first in the list, it is still the on slot's
    InputSwitch input;
    QVERIFY(loadsAppearances(input, {{{"key", "on"}, {"skinName", "lit.svg"}}}, SerializationPurpose::PortableFile,
                             dir.path()));
    QCOMPARE(input.appearance().alternativeAppearances(),
             QStringList({input.appearance().defaultAppearances().at(0), dir.filePath("lit.svg")}));
}

void TestElementSerializer::testAKeyNoSlotHasIsSkipped()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writePicture(dir.filePath("lit.svg")));

    // A key the type has no slot for, a slot dropped since or one a newer version added, is skipped, wherever it sits
    InputSwitch input;
    QVERIFY(loadsAppearances(input,
                             {{{"key", "flashing"}, {"skinName", "lit.svg"}}, {}, {{"key", "dimmed"}, {"skinName", "lit.svg"}}},
                             SerializationPurpose::PortableFile, dir.path()));
    QCOMPARE(input.appearance().alternativeAppearances(), input.appearance().defaultAppearances());
}

void TestElementSerializer::testAnOlderFilesPositionsReadAsTheSlotsTheyNamed()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(writePicture(dir.filePath("first.svg")));
    QVERIFY(writePicture(dir.filePath("second.svg")));

    // A list with no keys reads each position as the slot it named: a switch's second is its on slot
    InputSwitch input;
    QVERIFY(loadsAppearances(input, {{}, {{"skinName", "second.svg"}}}, SerializationPurpose::PortableFile, dir.path()));
    QCOMPARE(input.appearance().alternativeAppearances(),
             QStringList({input.appearance().defaultAppearances().at(0), dir.filePath("second.svg")}));

    // Text's second position named a picture nothing showed, a slot since dropped: it is skipped, and the first
    // position is still Text's one slot
    const std::unique_ptr<Element> text(ElementFactory::buildElement(ElementType::Text));
    QVERIFY(loadsAppearances(*text, {{{"skinName", "first.svg"}}, {{"skinName", "second.svg"}}},
                             SerializationPurpose::PortableFile, dir.path()));
    QCOMPARE(text->appearance().alternativeAppearances(), QStringList({dir.filePath("first.svg")}));

    // So does the rotary's second, its arrow
    const std::unique_ptr<Element> rotary(ElementFactory::buildElement(ElementType::InputRotary));
    QVERIFY(loadsAppearances(*rotary, {{}, {{"skinName", "second.svg"}}}, SerializationPurpose::PortableFile, dir.path()));
    QCOMPARE(rotary->appearance().alternativeAppearances(), rotary->appearance().defaultAppearances());

    // A position past those the type ever had is refused, as a corrupt stream
    QVERIFY(!loadsAppearances(*text, {{}, {}, {{"skinName", "first.svg"}}}, SerializationPurpose::PortableFile, dir.path()));
}

void TestElementSerializer::testOlderAppearanceNamesReadAsTheSlotsTheyNamed()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString first = dir.filePath("first.svg");
    QVERIFY(writePicture(first));

    // A file older than 4.1 lists a name at each position: Text's second, a slot since dropped, is skipped
    const std::unique_ptr<Element> text(ElementFactory::buildElement(ElementType::Text));
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << quint64(2) << first << QString(":/Components/Misc/text.png");

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_4_0_1, SerializationPurpose::PortableFile};
    QDataStream readStream(data);
    ElementSerializer::loadPixmapAppearanceNames(*text, readStream, context);
    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(text->appearance().alternativeAppearances(), QStringList({first}));
}

void TestElementSerializer::testAppearancesSaveAgainByteForByte_data()
{
    QTest::addColumn<int>("purpose");
    QTest::newRow("file") << static_cast<int>(SerializationPurpose::PortableFile);
    QTest::newRow("snapshot") << static_cast<int>(SerializationPurpose::InMemorySnapshot);
}

void TestElementSerializer::testAppearancesSaveAgainByteForByte()
{
    QFETCH(int, purpose);
    const auto how = static_cast<SerializationPurpose>(purpose);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("custom.svg");
    QVERIFY(writePicture(path));

    // A switch showing the file in its on slot, and an LED in its lit red slot: what each saves, read back into a new
    // element, saves again byte for byte
    InputSwitch input;
    input.setOn(true);
    input.setAppearance(false, path);
    Led led;
    led.setColor("Red");
    led.inputPort(0)->setStatus(Status::Active);
    led.refresh();
    led.setAppearance(false, path);
    QCOMPARE(led.appearance().alternativeAppearances().indexOf(path), 3);

    for (const Element *element : {static_cast<const Element *>(&input), static_cast<const Element *>(&led)}) {
        const QByteArray first = savedBytes(*element, how);
        const std::unique_ptr<Element> copy(ElementFactory::buildElement(element->elementType()));
        QHash<quint64, Port *> portMap;
        SerializationContext context = {portMap, FormatRev::current, how};
        context.contextDir = dir.path();
        QDataStream readStream(first);
        copy->load(readStream, context);
        QCOMPARE(readStream.status(), QDataStream::Ok);
        QCOMPARE(copy->appearance().alternativeAppearances(), element->appearance().alternativeAppearances());
        QCOMPARE(savedBytes(*copy, how), first);
    }
}
