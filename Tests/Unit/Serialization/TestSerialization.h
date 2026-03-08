// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QByteArray>
#include <QObject>
#include <QVersionNumber>

class WorkSpace;
class GraphicElement;

class TestSerialization : public QObject
{
    Q_OBJECT

private slots:
    // Lifecycle
    void init();

    // Basic round-trip tests
    void testSingleElementRoundTrip();
    void testMultipleElementsRoundTrip();
    void testConnectionPersistence();
    void testElementPropertiesPreserved_data();
    void testElementPropertiesPreserved();

    // Complex circuits
    void testComplexCircuitSaveLoad();
    void testICSerializationBasic();

    // File format
    void testFileFormatHeader();
    void testDolphinFilenamePreserved();
    void testSceneRectPreserved();

    // Error handling
    void testInvalidMagicHeader();
    void testCorruptedStream();

    // Property persistence
    void testRotationPreserved_data();
    void testRotationPreserved();
    void testLabelPreserved_data();
    void testLabelPreserved();
    void testInputSizePreserved();
    void testOutputSizePreserved();

    // Special cases
    void testEmptyCircuit();

    void testLargeCircuitWith100Elements();
    void testLargeCircuitWith500Elements();

    void testUnicodeInLabels();
    void testSpecialCharactersInLabels();
    void testExtendedAsciiCharacters();

    void testMissingRequiredAttributes();
    void testInvalidElementTypeInStream();
    void testNegativePositionValues();
    void testExtremelyLargeValues();
    void testNullElementHandling();

    void testOldVersionHeaderFormat();
    void testMissingVersionHeader();

    void testTruncatedFileRecovery();
    void testInvalidUTF8InLabels();
    void testMismatchedElementCount();
    void testMultiGateChainSerialization();
    void testExtremelyLongLabels();
    void testStreamPositionValidation();
    void testConnectionWithDeletedPorts();
    void testMalformedConnectionData();

private:
    // Helper methods
    QByteArray saveToMemory(WorkSpace &workspace);
    void loadFromMemory(WorkSpace &workspace, const QByteArray &data);
    void verifyElementEquality(GraphicElement *original, GraphicElement *loaded);
};

