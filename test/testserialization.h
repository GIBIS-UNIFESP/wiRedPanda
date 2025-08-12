// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class QGraphicsItem;

class TestSerialization : public QObject
{
    Q_OBJECT

private slots:
    // Header operations tests
    void testWritePandaHeader();
    void testReadPandaHeader();
    void testWriteDolphinHeader();
    void testReadDolphinHeader();
    void testMagicHeaders();
    
    // Serialization core tests
    void testSerializeEmptyList();
    void testSerializeSingleElement();
    void testSerializeMultipleElements();
    void testSerializeConnections();
    
    // Deserialization core tests
    void testDeserializeEmptyStream();
    void testDeserializeSingleElement();
    void testDeserializeMultipleElements();
    void testDeserializeConnections();
    
    // Round-trip serialization tests
    void testSerializeDeserializeRoundTrip();
    void testSerializeDeserializeWithConnections();
    void testSerializeDeserializeComplexCircuit();
    
    // File format version tests
    void testVersionNumbers();
    void testBackwardsCompatibility();
    void testVersionHandling();
    
    // Utility functions tests
    void testTypeName();
    void testLoadRect();
    void testLoadDolphinFileName();
    
    // Error handling tests
    void testInvalidFileFormat();
    void testCorruptedData();
    void testInvalidMagicHeader();
    
    // Stream operations tests
    void testStreamPositioning();
    void testDataStreamVersion();
    void testStreamErrorHandling();
    
    // Integration tests
    void testSerializationIntegration();
    void testFileFormatConsistency();
    
private:
    QByteArray createTestStream();
    QByteArray createCorruptedStream();
    void validateDeserialization(const QList<QGraphicsItem *> &items);
};