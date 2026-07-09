// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestTruthTable.h"

#include <limits>

#include <QDataStream>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QTest>

#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Constructor and Configuration Tests
// ============================================================================

void TestTruthTable::testConstructorInitialization()
{
    TruthTable truthTable;

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

    // Verify can change appearance
    QVERIFY(truthTable.canChangeAppearance());

    // Verify has label
    QVERIFY(truthTable.hasLabel());
}

void TestTruthTable::testInputPortNames()
{
    TruthTable truthTable;

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
    TruthTable truthTable;

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
    TruthTable truthTable;

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
    TruthTable truthTable;

    // Verify key is initialized to a QBitArray
    QBitArray &key = truthTable.key();
    QVERIFY(key.size() > 0);

    // Default key should be filled with 0s
    for (int i = 0; i < key.size(); ++i) {
        QVERIFY(key.at(i) == false);
    }
}

void TestTruthTable::testSetKey()
{
    TruthTable truthTable;

    // Create a test key
    QBitArray testKey(16);
    testKey.setBit(0, true);
    testKey.setBit(5, true);
    testKey.setBit(15, true);

    // Set the key
    truthTable.setkey(testKey);

    // Verify the supplied bits were kept; the key itself is padded back to
    // the 2048-bit class invariant (see testSetKeyEnforcesSizeInvariant).
    QBitArray &retrievedKey = truthTable.key();
    QCOMPARE(retrievedKey.size(), 2048);
    QVERIFY(retrievedKey.at(0) == true);
    QVERIFY(retrievedKey.at(5) == true);
    QVERIFY(retrievedKey.at(15) == true);
}

void TestTruthTable::testSetKeyEnforcesSizeInvariant()
{
    // Regression test (F2): updateLogic() indexes the key at 256*output + row
    // (up to bit 2047) and ToggleTruthTableOutputCommand toggles bits in
    // place, so the key must always hold exactly 2048 bits. A crafted/corrupt
    // .panda file can carry a shorter (or longer) array — setkey() is the
    // single mutation point and must restore the invariant, padding with
    // zeros and preserving the supplied bits.
    TruthTable truthTable;

    QBitArray shortKey(16);
    shortKey.setBit(0, true);
    shortKey.setBit(15, true);
    truthTable.setkey(shortKey);

    QCOMPARE(truthTable.key().size(), 2048);
    QVERIFY(truthTable.key().at(0));
    QVERIFY(truthTable.key().at(15));
    for (int i = 16; i < 2048; ++i) {
        QVERIFY(!truthTable.key().at(i));
    }

    QBitArray longKey(4096, true);
    truthTable.setkey(longKey);
    QCOMPARE(truthTable.key().size(), 2048);
}

void TestTruthTable::testKeyAccess()
{
    TruthTable truthTable;

    // Verify key() returns reference we can modify
    QBitArray &key = truthTable.key();
    key.setBit(0, true);
    key.setBit(10, true);

    // Verify changes persist
    QBitArray &key2 = truthTable.key();
    QVERIFY(key2.at(0) == true);
    QVERIFY(key2.at(10) == true);
}

void TestTruthTable::testKeyBitArray()
{
    TruthTable truthTable;

    QBitArray &key = truthTable.key();

    // Verify we can work with the bit array
    int bitsToSet = 5;
    for (int i = 0; i < bitsToSet; ++i) {
        key.setBit(i * 100, true);
    }

    // Verify bits were set
    for (int i = 0; i < bitsToSet; ++i) {
        QVERIFY(key.at(i * 100) == true);
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
    TruthTable truthTable;

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
    TruthTable truthTable;

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
    TruthTable truthTable;

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
    TruthTable truthTable;

    // Set a specific key
    QBitArray testKey(256);
    testKey.setBit(0, true);
    testKey.setBit(100, true);
    testKey.setBit(255, true);
    truthTable.setkey(testKey);

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
    TruthTable truthTable1;

    QBitArray testKey(256);
    testKey.setBit(50, true);
    testKey.setBit(150, true);
    truthTable1.setkey(testKey);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    truthTable1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load into new truth table with version 4.2
    TruthTable truthTable2;

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    SerializationContext context = {portMap, QVersionNumber(4, 2), SerializationPurpose::PortableFile, {}};
    truthTable2.load(loadStream, context);

    // Verify key was loaded correctly
    QBitArray &loadedKey = truthTable2.key();
    QVERIFY(loadedKey.at(50) == true);
    QVERIFY(loadedKey.at(150) == true);
}

void TestTruthTable::testLoadKeyOldVersion()
{
    // Test loading with version < 4.2 (should skip key loading)
    TruthTable truthTable1;

    QBitArray testKey(256);
    testKey.setBit(75, true);
    truthTable1.setkey(testKey);

    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    truthTable1.save(saveStream, {.purpose = SerializationPurpose::PortableFile});

    // Load with old version (< 4.2)
    TruthTable truthTable2;

    QDataStream loadStream(data);
    QHash<quint64, Port *> portMap;
    // Load with version 4.1 (less than 4.2)
    SerializationContext context = {portMap, QVersionNumber(4, 1), SerializationPurpose::PortableFile, {}};
    truthTable2.load(loadStream, context);

    // Key should remain at default (all zeros) for old versions
    // because load() returns early when version < 4.2
    QCOMPARE(truthTable2.key().count(true), 0);
}

// ============================================================================
// Rendering Under Rotation Tests
// ============================================================================

void TestTruthTable::testRotationKeepsSizingPixmap()
{
    // TruthTable::drawBody() sizes its body from pixmap().rect() — the sizing pixmap
    // installed by generatePixmap(), which grows with the port count. Rotating must not
    // swap it back for the fixed 64×64 base skin, or the body is drawn at the wrong size.
    // pixmapCenter() is the public window onto that pixmap's geometry.
    TruthTable truthTable;
    truthTable.setInputSize(8);

    // Inputs sit at x=0 and outputs at x=64 at any port count, so the body stays symmetric
    // about (32,32) on both axes — the centre point can't distinguish "grew past the base
    // skin" from "still the base skin"; height can.
    QVERIFY(truthTable.boundingRect().height() > 64);

    const QPointF footprintCenter = truthTable.pixmapCenter();

    truthTable.setRotation(90);
    QCOMPARE(truthTable.pixmapCenter(), footprintCenter);

    truthTable.setRotation(0);
    QCOMPARE(truthTable.pixmapCenter(), footprintCenter);
}

namespace {

/// Renders \a elm's scene footprint into an image; \a logoCenterOut receives the centre of the
/// element's body (where drawBody() centres its icon) in image coordinates.
QImage renderTruthTableElement(QGraphicsScene *scene, GraphicElement *elm, QPoint &logoCenterOut)
{
    // Render 1:1 from an integer-aligned source rect so every orientation rasterises scene
    // content at the same scale and sub-pixel phase — a fractional source origin would shift
    // antialiasing between renders and defeat pixel comparison.
    const QRect source = elm->sceneBoundingRect().toAlignedRect();
    QImage image(source.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    scene->render(&painter, QRectF(image.rect()), source);
    painter.end();

    // pixmapCenter() is already an absolute point in the element's local frame (it now derives
    // from boundingRect().center() for procedural-render elements like TruthTable) — no topLeft
    // offset needed on top of it.
    const QPointF logoScene = elm->mapToScene(elm->pixmapCenter());
    logoCenterOut = (logoScene - source.topLeft()).toPoint();
    return image;
}

/// Compares \a halfSize-radius crops of \a a around \a ca and \a b around \a cb, trying small
/// alignment offsets so sub-pixel crop rounding can't fail the comparison. Returns the smallest
/// count of pixels whose channels differ by more than \a tolerance.
int truthTableAlignedMismatch(const QImage &a, const QPoint ca, const QImage &b, const QPoint cb,
                               const int halfSize, const int tolerance)
{
    int best = std::numeric_limits<int>::max();

    for (int oy = -2; oy <= 2; ++oy) {
        for (int ox = -2; ox <= 2; ++ox) {
            int count = 0;
            for (int dy = -halfSize; dy < halfSize; ++dy) {
                for (int dx = -halfSize; dx < halfSize; ++dx) {
                    const QColor pa = a.pixelColor(ca + QPoint(dx, dy));
                    const QColor pb = b.pixelColor(cb + QPoint(dx + ox, dy + oy));
                    if (qAbs(pa.red() - pb.red()) > tolerance
                        || qAbs(pa.green() - pb.green()) > tolerance
                        || qAbs(pa.blue() - pb.blue()) > tolerance) {
                        ++count;
                    }
                }
            }
            best = qMin(best, count);
        }
    }

    return best;
}

} // namespace

void TestTruthTable::testRotatedIconStaysUpright()
{
    // The table icon is decoration, like the baked-in SVG pin text: it must read upright at any
    // element orientation. Its rendering around the icon centre must therefore be identical at
    // 0° and 180°; the icon's dot-grid rows sit asymmetrically about the centre, so an icon
    // that rotates with the body produces a large mismatch. (180° rather than 90° so the
    // fractional footprint centre maps back onto the pixel grid — a 90° mapping lands on
    // half-pixels and antialiasing legitimately differs.)
    QGraphicsScene scene;
    auto *truthTable = new TruthTable(); // the scene takes ownership
    scene.addItem(truthTable);

    QPoint uprightCenter;
    const QImage upright = renderTruthTableElement(&scene, truthTable, uprightCenter);

    truthTable->setRotation(180);
    QPoint rotatedCenter;
    const QImage rotated = renderTruthTableElement(&scene, truthTable, rotatedCenter);

    const int mismatch = truthTableAlignedMismatch(upright, uprightCenter, rotated, rotatedCenter, 10, 32);
    QVERIFY2(mismatch <= 20,
             qPrintable(QString("Icon is not upright after a 180° rotation: %1 mismatching pixels")
                            .arg(mismatch)));
}

void TestTruthTable::testBigPivotsAtBoundingRectCenter()
{
    // Direct regression: a big TruthTable's rotation/flip pivot must be the element's actual
    // footprint centre, not the (0,0)-anchored raw pixmap centre that only happens to
    // coincide with it for small instances.
    QGraphicsScene scene;
    auto *truthTable = new TruthTable(); // the scene takes ownership
    scene.addItem(truthTable);
    truthTable->setInputSize(8); // max input size -> tallest body

    QVERIFY2(truthTable->boundingRect().topLeft() != QPointF(0, 0),
             "Test TruthTable isn't actually 'big' — boundingRect() didn't grow past the 64x64 body");
    QCOMPARE(truthTable->transformOriginPoint(), truthTable->boundingRect().center());
}

void TestTruthTable::testBigRotationDoesNotDriftInScene()
{
    QGraphicsScene scene;
    auto *truthTable = new TruthTable();
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
    auto *truthTable = new TruthTable();
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
