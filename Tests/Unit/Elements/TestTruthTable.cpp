// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestTruthTable.h"

#include <QDataStream>
#include <QTest>

#include "App/Element/GraphicElements/TruthTable.h"
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

    // Verify can change skin
    QVERIFY(truthTable.canChangeSkin());

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

    // Verify key was set correctly
    QBitArray &retrievedKey = truthTable.key();
    QCOMPARE(retrievedKey.size(), testKey.size());
    QVERIFY(retrievedKey.at(0) == true);
    QVERIFY(retrievedKey.at(5) == true);
    QVERIFY(retrievedKey.at(15) == true);
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

    truthTable.save(stream);

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
    truthTable1.save(saveStream);

    // Load into new truth table with version 4.2
    TruthTable truthTable2;

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, QVersionNumber(4, 2), {}};

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
    truthTable1.save(saveStream);

    // Load with old version (< 4.2)
    TruthTable truthTable2;

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, QVersionNumber(4, 1), {}};

    // Load with version 4.1 (less than 4.2)
    truthTable2.load(loadStream, context);

    // Key should remain at default (all zeros) for old versions
    // because load() returns early when version < 4.2
    QCOMPARE(truthTable2.key().count(true), 0);
}
