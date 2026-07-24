// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestGraphicElementSerializer.h"

#include <QDataStream>
#include <QKeySequence>
#include <QPointF>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementSerializer.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/IC.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestGraphicElementSerializer::testReadPortListCountReadFailureThrows()
{
    QByteArray data; // empty: the very first read (the count) runs past the end
    QDataStream stream(data);

    bool threw = false;
    try {
        GraphicElementSerializer::readPortList(stream, "test");
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an empty stream must fail reading the port-list count");
}

void TestGraphicElementSerializer::testReadPortListEntryReadFailureThrows()
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
        GraphicElementSerializer::readPortList(stream, "test");
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "a truncated entry must fail, not silently return a partial list");
}

void TestGraphicElementSerializer::testRemovePortFromMapErasesMatchingEntries()
{
    And gate;
    Port *target = gate.inputPort(0);
    Port *other = gate.inputPort(1);

    QHash<quint64, Port *> portMap;
    portMap[1] = target;
    portMap[2] = other;
    portMap[3] = target; // a port may be registered under more than one legacy key

    GraphicElementSerializer::removePortFromMap(target, portMap);

    QCOMPARE(portMap.size(), 1);
    QCOMPARE(portMap.value(2), other);
}

void TestGraphicElementSerializer::testLoadOldFormatDrainsUnusedPriorityField()
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
    GraphicElementSerializer::loadOldFormat(gate, readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
}

void TestGraphicElementSerializer::testLoadNewFormatPropertiesStreamErrorThrows()
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
        GraphicElementSerializer::loadNewFormat(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "a truncated properties map must fail, not silently proceed with a partial map");
}

void TestGraphicElementSerializer::testLoadNewFormatAppearanceIndexOutOfRangeThrows()
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
        GraphicElementSerializer::loadNewFormat(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "more appearance entries than the element's own slot count must be rejected");
}

void TestGraphicElementSerializer::testLoadInputPortsImplausibleCountThrows()
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
        GraphicElementSerializer::loadInputPorts(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format input port count must be rejected before allocating");
}

void TestGraphicElementSerializer::testLoadOutputPortsImplausibleCountThrows()
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
        GraphicElementSerializer::loadOutputPorts(gate, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format output port count must be rejected before allocating");
}

void TestGraphicElementSerializer::testLoadPixmapAppearanceNamesImplausibleCountThrows()
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
        GraphicElementSerializer::loadPixmapAppearanceNames(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "an implausible old-format appearance-name count must be rejected before allocating");
}

void TestGraphicElementSerializer::testLoadPixmapAppearanceNamesIndexOutOfRangeThrows()
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
        GraphicElementSerializer::loadPixmapAppearanceNames(line, readStream, context);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "more appearance names than the element's own slot count must be rejected");
}

void TestGraphicElementSerializer::testLoadPixmapAppearanceNamesAppliesResolvedPath()
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
    GraphicElementSerializer::loadPixmapAppearanceNames(line, readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);
    QCOMPARE(line.externalFiles(), QStringList{path});
}

void TestGraphicElementSerializer::testLoadInputOutputPortRenamesExistingIcPorts()
{
    // loadInputPort()/loadOutputPort()'s "port < inputs().size()" branch (IC-only rename) is
    // only reachable when the target element *already has* ports at load time -- true for IC
    // specifically when an already-populated IC object is reloaded in place (e.g. an
    // UpdateCommand undo/redo snapshot), not for a freshly constructed one (IC::load() populates
    // its own ports from the sub-circuit blob *after* calling GraphicElement::load(), so a fresh
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
    GraphicElementSerializer::loadInputPorts(*ic, inputRead, context);
    QCOMPARE(ic->inputPort(0)->name(), QString("renamed_in"));

    QByteArray outputData;
    QDataStream outputWrite(&outputData, QIODevice::WriteOnly);
    outputWrite << quint64(1);
    outputWrite << quint64(2) << QString("renamed_out") << 0;
    QDataStream outputRead(outputData);
    GraphicElementSerializer::loadOutputPorts(*ic, outputRead, context);
    QCOMPARE(ic->outputPort(0)->name(), QString("renamed_out"));
}
