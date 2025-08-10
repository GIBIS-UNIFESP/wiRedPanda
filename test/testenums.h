// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include "enums.h"

class TestEnums : public QObject
{
    Q_OBJECT

private slots:
    // Basic enum cycling tests
    void testNextElmType();
    void testPrevElmType();
    
    // Comprehensive element type cycling tests
    void testGateTypeCycling();
    void testInputTypeCycling();
    void testFlipFlopTypeCycling();
    void testOutputTypeCycling();
    void testNotNodeCycling();
    
    // Edge cases and special values
    void testUnknownElementType();
    void testUnsupportedElementTypes();
    void testCyclingConsistency();
    
    // Serialization operator tests
    void testElementTypeSerializationOperators();
    void testElementTypeDeserialization();
    void testSerializationRoundTrip();
    void testInvalidSerializationData();
    
    // Increment operator tests
    void testElementTypeIncrementOperator();
    void testIncrementOverflow();
    
    // Comprehensive coverage tests
    void testAllDefinedElementTypes();
    void testElementTypeBoundaries();
    void testSerializationDataIntegrity();
    
private:
    // Helper methods
    void testCycleGroup(const QVector<ElementType> &group);
    void validateCycleConsistency(ElementType start, ElementType next, ElementType prev);
    void testElementTypeSerialization(ElementType type);
};