// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestTruthTable.h"

#include <QDataStream>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QStyleOptionGraphicsItem>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Constructor and Configuration Tests
// ============================================================================

void TestTruthTable::testConstructorInitialization()
{
    Element truthTable{ElementType::TruthTable};

    // Verify element type
    QCOMPARE(truthTable.elementType(), ElementType::TruthTable);

    // Verify element group is IC
    QCOMPARE(truthTable.elementGroup(), ElementGroup::IC);

    // Verify default port configuration (2-8 inputs, 1-8 outputs)
    QVERIFY(truthTable.inputSize() >= 2);
    QVERIFY(truthTable.inputSize() <= 8);
    QVERIFY(truthTable.outputSize() >= 1);
    QVERIFY(truthTable.outputSize() <= 8);

    // Verify has truth table flag
    QVERIFY(truthTable.hasTruthTable());

    // Its body is drawn in code, sized by its ports, so it offers no custom appearance
    QVERIFY(!truthTable.canChangeAppearance());

    // Verify has label
    QVERIFY(truthTable.hasLabel());
}

void TestTruthTable::testInputPortNames()
{
    Element truthTable{ElementType::TruthTable};

    // Verify input ports are named A, B, C, ... up to available ports
    for (int i = 0; i < truthTable.inputSize(); ++i) {
        auto *port = truthTable.inputPort(i);
        QVERIFY(port != nullptr);

        // Expected name is single letter A, B, C, ... H
        QString expectedName = QChar::fromLatin1(static_cast<char>('A' + i));
        QCOMPARE(port->name(), expectedName);
    }
}

void TestTruthTable::testOutputPortNames()
{
    Element truthTable{ElementType::TruthTable};

    // Verify output ports are named S0, S1, S2, ... based on index
    for (int i = 0; i < truthTable.outputSize(); ++i) {
        auto *port = truthTable.outputPort(i);
        QVERIFY(port != nullptr);

        // Expected name is S followed by index
        QString expectedName = "S" + QString::number(i);
        QCOMPARE(port->name(), expectedName);
    }
}

void TestTruthTable::testPortConfiguration()
{
    Element truthTable{ElementType::TruthTable};

    int inputCount = truthTable.inputSize();
    int outputCount = truthTable.outputSize();

    // Verify reasonable port counts
    QVERIFY(inputCount >= 2);
    QVERIFY(inputCount <= 8);
    QVERIFY(outputCount >= 1);
    QVERIFY(outputCount <= 8);

    // Verify all input ports exist
    for (int i = 0; i < inputCount; ++i) {
        QVERIFY(truthTable.inputPort(i) != nullptr);
    }

    // Verify all output ports exist
    for (int i = 0; i < outputCount; ++i) {
        QVERIFY(truthTable.outputPort(i) != nullptr);
    }

    // Verify min/max sizes
    QCOMPARE(truthTable.minInputSize(), 2);
    QCOMPARE(truthTable.maxInputSize(), 8);
    QCOMPARE(truthTable.minOutputSize(), 1);
    QCOMPARE(truthTable.maxOutputSize(), 8);
}

// ============================================================================
// Key Management Tests
// ============================================================================

void TestTruthTable::testKeyInitialization()
{
    Element truthTable{ElementType::TruthTable};

    // Verify key is initialized to a QBitArray
    const QBitArray &key = truthTable.param<QBitArray>(ParamId::Table);
    QVERIFY(key.size() > 0);

    // Default key should be filled with 0s
    for (int i = 0; i < key.size(); ++i) {
        QVERIFY(key.at(i) == false);
    }
}

void TestTruthTable::testSetKey()
{
    Element truthTable{ElementType::TruthTable};

    // Create a test key
    QBitArray testKey(16);
    testKey.setBit(0, true);
    testKey.setBit(5, true);
    testKey.setBit(15, true);

    // Set the key
    truthTable.setParam(ParamId::Table, testKey);

    // Verify the supplied bits were kept; the key itself is padded back to
    // the 2048-bit class invariant (see testSetKeyEnforcesSizeInvariant).
    const QBitArray &retrievedKey = truthTable.param<QBitArray>(ParamId::Table);
    QCOMPARE(retrievedKey.size(), 2048);
    QVERIFY(retrievedKey.at(0) == true);
    QVERIFY(retrievedKey.at(5) == true);
    QVERIFY(retrievedKey.at(15) == true);
}

void TestTruthTable::testSetKeyEnforcesSizeInvariant()
{
    // Regression test (F2): the kernel indexes the key at 256*output + row
    // (up to bit 2047) and ToggleTruthTableOutputCommand toggles bits in
    // place, so the key must always hold exactly 2048 bits. A crafted/corrupt
    // .panda file can carry a shorter (or longer) array — the parameter's spec
    // takes every value set and must restore the invariant, padding with
    // zeros and preserving the supplied bits.
    Element truthTable{ElementType::TruthTable};

    QBitArray shortKey(16);
    shortKey.setBit(0, true);
    shortKey.setBit(15, true);
    truthTable.setParam(ParamId::Table, shortKey);

    QCOMPARE(truthTable.param<QBitArray>(ParamId::Table).size(), 2048);
    QVERIFY(truthTable.param<QBitArray>(ParamId::Table).at(0));
    QVERIFY(truthTable.param<QBitArray>(ParamId::Table).at(15));
    for (int i = 16; i < 2048; ++i) {
        QVERIFY(!truthTable.param<QBitArray>(ParamId::Table).at(i));
    }

    QBitArray longKey(4096, true);
    truthTable.setParam(ParamId::Table, longKey);
    QCOMPARE(truthTable.param<QBitArray>(ParamId::Table).size(), 2048);
}

void TestTruthTable::testKeyAccess()
{
    Element truthTable{ElementType::TruthTable};

    // Verify bits set through the parameter persist
    QBitArray key = truthTable.param<QBitArray>(ParamId::Table);
    key.setBit(0, true);
    key.setBit(10, true);
    truthTable.setParam(ParamId::Table, key);

    const QBitArray &key2 = truthTable.param<QBitArray>(ParamId::Table);
    QVERIFY(key2.at(0) == true);
    QVERIFY(key2.at(10) == true);
}

void TestTruthTable::testKeyBitArray()
{
    Element truthTable{ElementType::TruthTable};

    QBitArray bits = truthTable.param<QBitArray>(ParamId::Table);

    // Verify we can work with the bit array
    int bitsToSet = 5;
    for (int i = 0; i < bitsToSet; ++i) {
        bits.setBit(static_cast<qsizetype>(i) * 100, true);
    }
    truthTable.setParam(ParamId::Table, bits);
    const QBitArray &key = truthTable.param<QBitArray>(ParamId::Table);

    // Verify bits were set
    for (int i = 0; i < bitsToSet; ++i) {
        QVERIFY(key.at(static_cast<qsizetype>(i) * 100) == true);
    }

    // Verify other bits remain false
    QVERIFY(key.at(1) == false);
    QVERIFY(key.at(50) == false);
}

// ============================================================================
// Port Properties Tests
// ============================================================================

void TestTruthTable::testUpdatePortsProperties()
{
    Element truthTable{ElementType::TruthTable};

    // Call update to refresh port properties
    truthTable.updatePortsProperties();

    // Verify all ports still exist after update
    QVERIFY(truthTable.inputSize() > 0);
    QVERIFY(truthTable.outputSize() > 0);

    // Verify ports have proper names
    for (int i = 0; i < truthTable.inputSize(); ++i) {
        QVERIFY(!truthTable.inputPort(i)->name().isEmpty());
    }

    for (int i = 0; i < truthTable.outputSize(); ++i) {
        QVERIFY(!truthTable.outputPort(i)->name().isEmpty());
    }
}

void TestTruthTable::testInputPortPositioning()
{
    Element truthTable{ElementType::TruthTable};

    // Verify input ports have valid positions
    for (int i = 0; i < truthTable.inputSize(); ++i) {
        auto *port = truthTable.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Input port %1 is null").arg(i)));
        QPointF pos = port->pos();

        // Input ports should be on the left side (x ~ 0)
        QVERIFY(pos.x() >= -5 && pos.x() <= 5);

        // Positions should be ordered vertically
        if (i > 0) {
            auto *prevPort = truthTable.inputPort(i - 1);
            QVERIFY2(prevPort != nullptr, qPrintable(QString("Input port %1 is null").arg(i - 1)));
            QVERIFY(port->pos().y() >= prevPort->pos().y());
        }
    }
}

void TestTruthTable::testOutputPortPositioning()
{
    Element truthTable{ElementType::TruthTable};

    // Verify output ports have valid positions
    for (int i = 0; i < truthTable.outputSize(); ++i) {
        auto *port = truthTable.outputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Output port %1 is null").arg(i)));
        QPointF pos = port->pos();

        // Output ports should be on the right side (x ~ 64)
        QVERIFY(pos.x() >= 59 && pos.x() <= 69);

        // Positions should be ordered vertically
        if (i > 0) {
            auto *prevPort = truthTable.outputPort(i - 1);
            QVERIFY2(prevPort != nullptr, qPrintable(QString("Output port %1 is null").arg(i - 1)));
            QVERIFY(port->pos().y() >= prevPort->pos().y());
        }
    }
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestTruthTable::testSaveKey()
{
    Element truthTable{ElementType::TruthTable};

    // Set a specific key
    QBitArray testKey(256);
    testKey.setBit(0, true);
    testKey.setBit(100, true);
    testKey.setBit(255, true);
    truthTable.setParam(ParamId::Table, testKey);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    truthTable.save(stream, {.purpose = SerializationPurpose::PortableFile});

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestTruthTable::testLoadKeyVersion42()
{
    // Create and save truth table with key
    Element truthTable1{ElementType::TruthTable};

    QBitArray testKey(256);
    testKey.setBit(50, true);
    testKey.setBit(150, true);
    truthTable1.setParam(ParamId::Table, testKey);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    truthTable1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load into new truth table with version 4.2
    Element truthTable2{ElementType::TruthTable};

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 2), SerializationPurpose::PortableFile, {}};
    truthTable2.load(loadStream, context);

    // Verify key was loaded correctly
    const QBitArray &loadedKey = truthTable2.param<QBitArray>(ParamId::Table);
    QVERIFY(loadedKey.at(50) == true);
    QVERIFY(loadedKey.at(150) == true);
}

void TestTruthTable::testLoadKeyOldVersion()
{
    // Test loading with version < 4.2 (should skip key loading)
    Element truthTable1{ElementType::TruthTable};

    QBitArray testKey(256);
    testKey.setBit(75, true);
    truthTable1.setParam(ParamId::Table, testKey);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    truthTable1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load with old version (< 4.2)
    Element truthTable2{ElementType::TruthTable};

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    // Load with version 4.1 (less than 4.2)
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};
    truthTable2.load(loadStream, context);

    // Key should remain at default (all zeros) for old versions
    // because load() returns early when version < 4.2
    QCOMPARE(truthTable2.param<QBitArray>(ParamId::Table).count(true), 0);
}

// ============================================================================
// Rendering Under Rotation Tests
// ============================================================================

void TestTruthTable::testRotationKeepsTheFootprint()
{
    // The body is the footprint, which grows with the port count, and turns about its centre. Rotating must not
    // measure it again as the fixed 64×64 picture, or the body is drawn at the wrong size.
    Element truthTable{ElementType::TruthTable};
    truthTable.setInputSize(8);

    // Inputs sit at x=0 and outputs at x=64 at any port count, so the body stays symmetric
    // about (32,32) on both axes — the centre point can't distinguish "grew past the base
    // skin" from "still the base skin"; height can.
    QVERIFY(truthTable.boundingRect().height() > 64);

    const BodyShape footprint = truthTable.body();
    QVERIFY(footprint.body.height() > 64);
    QCOMPARE(footprint.centre, footprint.body.center());

    truthTable.setRotation(90);
    QCOMPARE(truthTable.body(), footprint);

    truthTable.setRotation(0);
    QCOMPARE(truthTable.body(), footprint);
}

void TestTruthTable::testRotatedIconStaysUpright()
{
    // The table icon is decoration, like the baked-in SVG pin text: it must read upright at any
    // element orientation. Its rendering around the icon centre must therefore be identical at
    // 0° and 180°; the icon's dot-grid rows sit asymmetrically about the centre, so an icon
    // that rotates with the body produces a large mismatch. (180° rather than 90° so the
    // fractional footprint centre maps back onto the pixel grid — a 90° mapping lands on
    // half-pixels and antialiasing legitimately differs.)
    QGraphicsScene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable); // the scene takes ownership
    scene.addItem(truthTable);

    QPoint uprightCenter;
    const QImage upright = TestUtils::renderElementForComparison(&scene, truthTable, uprightCenter);

    truthTable->setRotation(180);
    QPoint rotatedCenter;
    const QImage rotated = TestUtils::renderElementForComparison(&scene, truthTable, rotatedCenter);

    const int mismatch = TestUtils::alignedMismatch(upright, uprightCenter, rotated, rotatedCenter, 10, 32);
    QVERIFY2(mismatch <= 20,
             qPrintable(QString("Icon is not upright after a 180° rotation: %1 mismatching pixels")
                            .arg(mismatch)));
}

void TestTruthTable::testPaintingSelected()
{
    QGraphicsScene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable);
    scene.addItem(truthTable);
    truthTable->setSelected(true);

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    truthTable->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "TruthTable paint() must draw the selection highlight when selected");
}

void TestTruthTable::testMouseDoubleClickRequestsEditor()
{
    // The scene opens a truth table's editor on a double-click of it, and edits no label
    Scene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable);
    scene.addItem(truthTable);
    QSignalSpy spy(&scene, &Scene::openTruthTableRequested);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
    event.setScenePos(truthTable->sceneBoundingRect().center());
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(&scene, &event);

    QCOMPARE(spy.count(), 1);
    QVERIFY(event.isAccepted());
    QVERIFY(!TestUtils::inlineLabelEditor(&scene));
}

void TestTruthTable::testDisconnectedInputsAreUnknown()
{
    Element truthTable{ElementType::TruthTable};
    TestUtils::initElm(truthTable);
    truthTable.updateLogic();

    for (int i = 0; i < truthTable.outputSize(); ++i) {
        QCOMPARE(truthTable.outputValue(i), Status::Unknown);
    }
}

void TestTruthTable::testBigPivotsAtBoundingRectCenter()
{
    // Direct regression: a big TruthTable's rotation/flip pivot must be the element's actual
    // footprint centre, not the (0,0)-anchored raw pixmap centre that only happens to
    // coincide with it for small instances.
    QGraphicsScene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable); // the scene takes ownership
    scene.addItem(truthTable);
    truthTable->setInputSize(8); // max input size -> tallest body

    QVERIFY2(truthTable->boundingRect().topLeft() != QPointF(0, 0),
             "Test TruthTable isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(truthTable->transformOriginPoint(), truthTable->boundingRect().center());
}

void TestTruthTable::testBigRotationDoesNotDriftInScene()
{
    QGraphicsScene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable);
    scene.addItem(truthTable);
    truthTable->setInputSize(8);

    const QPointF centerScene = truthTable->mapToScene(truthTable->boundingRect().center());

    for (const qreal angle : {90.0, 180.0, 270.0, 0.0}) {
        truthTable->setRotation(angle);
        QCOMPARE(truthTable->mapToScene(truthTable->boundingRect().center()), centerScene);
    }
}

void TestTruthTable::testBigFlipDoesNotDriftInScene()
{
    QGraphicsScene scene;
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable);
    scene.addItem(truthTable);
    truthTable->setInputSize(8);

    const QPointF centerScene = truthTable->mapToScene(truthTable->boundingRect().center());

    truthTable->setFlippedX(true);
    QCOMPARE(truthTable->mapToScene(truthTable->boundingRect().center()), centerScene);

    truthTable->setFlippedY(true);
    QCOMPARE(truthTable->mapToScene(truthTable->boundingRect().center()), centerScene);

    truthTable->setFlippedX(false);
    truthTable->setFlippedY(false);
    QCOMPARE(truthTable->mapToScene(truthTable->boundingRect().center()), centerScene);
}
