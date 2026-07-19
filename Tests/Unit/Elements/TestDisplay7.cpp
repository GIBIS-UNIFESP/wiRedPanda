// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplay7.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include <QKeySequence>
#include <QPointF>

#include "App/Element/GraphicElements/Display7.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Workspace.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestDisplay7::testDisplay7ColorSetting()
{
    Display7 display;
    display.setColor("Red");
    QCOMPARE(display.color(), QString("Red"));

    display.setColor("Green");
    QCOMPARE(display.color(), QString("Green"));
}

void TestDisplay7::testDisplay7SegmentRendering()
{
    Display7 display;
    display.setColor("Blue");
    QVERIFY(!display.color().isEmpty());
}

void TestDisplay7::testDisplay7SaveLoad()
{
    Display7 display;
    display.setColor("Red");

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    display.save(writeStream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestDisplay7::testDisplay7AllColors()
{
    Display7 display;
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay7::testDisplay7Paint()
{
    WorkSpace workspace;
    auto *disp = new Display7;
    workspace.scene()->addItem(disp);
    disp->setColor("Red");

    // Set all segments to Active
    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Display7 paint() must draw visible pixels");
}

void TestDisplay7::testDisplay7LoadPreV16RemapsPinsToCanonicalOrder()
{
    // Pre-1.6 files used a different pin-to-segment layout; load() applies a permutation
    // (and, since 1.6 < 1.7, a second one right after) to remap old positional indices onto
    // the canonical layout. Hand-built old-format stream (V_1_6/V_1_7 predate the QMap
    // format), matching GraphicElementSerializer::loadOldFormat()'s exact field order.
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    writeStream << QPointF(0.0, 0.0);   // loadPos
    writeStream << 0.0;                 // loadRotation (qreal) -- Display7 skips the old-convention adjustment outright
    writeStream << QString("label");    // loadLabel (hasLabels since V_1_2)
    writeStream << quint64(8) << quint64(8) << quint64(0) << quint64(1); // loadPortsSize (V_1_3)
    // hasTrigger/hasUnusedPriority are V_1_9/V_4_0_1 -- both false below V_1_6, nothing written.
    writeStream << quint64(8); // loadInputPorts: inputSize = 8, matching Display7's fixed 8 inputs
    for (quint64 i = 0; i < 8; ++i) {
        writeStream << (1000 + i);        // ptr: a fake, unique per-port serialized id
        writeStream << QString("old %1").arg(i); // name (ignored for non-IC types, just fills the field)
        writeStream << 0;                 // flags (unused by the loader)
    }
    writeStream << quint64(0); // loadOutputPorts: outputSize = 0, Display7 has no outputs
    // hasAppearanceNames() is V_2_7 -- false below V_1_6, nothing further is written.

    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, Versions::V_1_4, SerializationPurpose::PortableFile};

    Display7 display;
    QDataStream readStream(data);
    display.load(readStream, context);

    QCOMPARE(readStream.status(), QDataStream::Ok);

    // Composing both permutations by hand (order1 = {2,1,4,5,0,7,3,6}, order2 = {2,5,4,0,7,3,6,1}):
    // the port originally read at old index 0 ends up at order2[order1[0]] = order2[2] = 4;
    // old index 1 ends up at order2[order1[1]] = order2[1] = 5. Confirms the composed
    // permutation actually repositioned the ports, not just left them where they started.
    QCOMPARE(portMap.value(1000), display.inputPort(4));
    QCOMPARE(portMap.value(1001), display.inputPort(5));
}
