// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testenums.h"

#include "enums.h"
#include <QTest>
#include <QDataStream>
#include <QBuffer>

void TestEnums::testNextElmType()
{
    // Test basic next element type functionality
    QCOMPARE(Enums::nextElmType(ElementType::And), ElementType::Or);
    QCOMPARE(Enums::nextElmType(ElementType::Or), ElementType::Nand);
    QCOMPARE(Enums::nextElmType(ElementType::Not), ElementType::Node);
    QCOMPARE(Enums::nextElmType(ElementType::Node), ElementType::Not);
    
    // Test input elements
    QCOMPARE(Enums::nextElmType(ElementType::InputVcc), ElementType::InputGnd);
    QCOMPARE(Enums::nextElmType(ElementType::InputButton), ElementType::InputSwitch);
    
    // Test flip-flops
    QCOMPARE(Enums::nextElmType(ElementType::DFlipFlop), ElementType::TFlipFlop);
    QCOMPARE(Enums::nextElmType(ElementType::JKFlipFlop), ElementType::SRFlipFlop);
    
    // Test outputs
    QCOMPARE(Enums::nextElmType(ElementType::Led), ElementType::Buzzer);
    QCOMPARE(Enums::nextElmType(ElementType::AudioBox), ElementType::Led);
}

void TestEnums::testPrevElmType()
{
    // Test basic previous element type functionality
    QCOMPARE(Enums::prevElmType(ElementType::Or), ElementType::And);
    QCOMPARE(Enums::prevElmType(ElementType::Nand), ElementType::Or);
    QCOMPARE(Enums::prevElmType(ElementType::Not), ElementType::Node);
    QCOMPARE(Enums::prevElmType(ElementType::Node), ElementType::Not);
    
    // Test input elements
    QCOMPARE(Enums::prevElmType(ElementType::InputGnd), ElementType::InputVcc);
    QCOMPARE(Enums::prevElmType(ElementType::InputSwitch), ElementType::InputButton);
    
    // Test flip-flops
    QCOMPARE(Enums::prevElmType(ElementType::TFlipFlop), ElementType::DFlipFlop);
    QCOMPARE(Enums::prevElmType(ElementType::SRFlipFlop), ElementType::JKFlipFlop);
    
    // Test outputs
    QCOMPARE(Enums::prevElmType(ElementType::Buzzer), ElementType::Led);
    QCOMPARE(Enums::prevElmType(ElementType::Led), ElementType::AudioBox);
}

void TestEnums::testGateTypeCycling()
{
    QVector<ElementType> gates = {
        ElementType::And, ElementType::Or, ElementType::Nand, 
        ElementType::Nor, ElementType::Xor, ElementType::Xnor
    };
    testCycleGroup(gates);
}

void TestEnums::testInputTypeCycling()
{
    QVector<ElementType> inputs = {
        ElementType::InputVcc, ElementType::InputGnd, ElementType::InputButton,
        ElementType::InputSwitch, ElementType::InputRotary, ElementType::Clock
    };
    testCycleGroup(inputs);
}

void TestEnums::testFlipFlopTypeCycling()
{
    // Test D/T flip-flop cycle
    QCOMPARE(Enums::nextElmType(ElementType::DFlipFlop), ElementType::TFlipFlop);
    QCOMPARE(Enums::nextElmType(ElementType::TFlipFlop), ElementType::DFlipFlop);
    QCOMPARE(Enums::prevElmType(ElementType::DFlipFlop), ElementType::TFlipFlop);
    QCOMPARE(Enums::prevElmType(ElementType::TFlipFlop), ElementType::DFlipFlop);
    
    // Test JK/SR flip-flop cycle
    QCOMPARE(Enums::nextElmType(ElementType::JKFlipFlop), ElementType::SRFlipFlop);
    QCOMPARE(Enums::nextElmType(ElementType::SRFlipFlop), ElementType::JKFlipFlop);
    QCOMPARE(Enums::prevElmType(ElementType::JKFlipFlop), ElementType::SRFlipFlop);
    QCOMPARE(Enums::prevElmType(ElementType::SRFlipFlop), ElementType::JKFlipFlop);
}

void TestEnums::testOutputTypeCycling()
{
    QVector<ElementType> outputs = {
        ElementType::Led, ElementType::Buzzer, ElementType::AudioBox
    };
    testCycleGroup(outputs);
}

void TestEnums::testNotNodeCycling()
{
    // Test Not/Node bidirectional cycle
    QCOMPARE(Enums::nextElmType(ElementType::Not), ElementType::Node);
    QCOMPARE(Enums::nextElmType(ElementType::Node), ElementType::Not);
    QCOMPARE(Enums::prevElmType(ElementType::Not), ElementType::Node);
    QCOMPARE(Enums::prevElmType(ElementType::Node), ElementType::Not);
}

void TestEnums::testUnknownElementType()
{
    // Test that unknown/unsupported types return Unknown
    QCOMPARE(Enums::nextElmType(ElementType::Unknown), ElementType::Unknown);
    QCOMPARE(Enums::prevElmType(ElementType::Unknown), ElementType::Unknown);
}

void TestEnums::testUnsupportedElementTypes()
{
    // Test types that are not in cycling groups
    QVector<ElementType> unsupported = {
        ElementType::IC, ElementType::Display7, ElementType::Display14,
        ElementType::Display16, ElementType::Mux, ElementType::Demux,
        ElementType::DLatch, ElementType::JKLatch, ElementType::SRLatch,
        ElementType::TruthTable, ElementType::Line, ElementType::Text
    };
    
    for (ElementType type : unsupported) {
        QCOMPARE(Enums::nextElmType(type), ElementType::Unknown);
        QCOMPARE(Enums::prevElmType(type), ElementType::Unknown);
    }
}

void TestEnums::testCyclingConsistency()
{
    // Test that next->prev and prev->next operations are consistent
    QVector<ElementType> testTypes = {
        ElementType::And, ElementType::Or, ElementType::Nand, ElementType::Nor,
        ElementType::Xor, ElementType::Xnor, ElementType::Not, ElementType::Node,
        ElementType::InputVcc, ElementType::InputGnd, ElementType::InputButton,
        ElementType::InputSwitch, ElementType::InputRotary, ElementType::Clock,
        ElementType::DFlipFlop, ElementType::TFlipFlop, ElementType::JKFlipFlop,
        ElementType::SRFlipFlop, ElementType::Led, ElementType::Buzzer,
        ElementType::AudioBox
    };
    
    for (ElementType type : testTypes) {
        ElementType next = Enums::nextElmType(type);
        ElementType prev = Enums::prevElmType(type);
        validateCycleConsistency(type, next, prev);
    }
}

void TestEnums::testElementTypeSerializationOperators()
{
    // Test stream output operator
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    
    ElementType testType = ElementType::And;
    outStream << testType;
    
    buffer.close();
    QVERIFY(data.size() > 0);
    
    // Verify the serialized value
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    quint64 serializedValue;
    inStream >> serializedValue;
    QCOMPARE(serializedValue, static_cast<quint64>(ElementType::And));
}

void TestEnums::testElementTypeDeserialization()
{
    // Test stream input operator
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    
    quint64 testValue = static_cast<quint64>(ElementType::Or);
    outStream << testValue;
    buffer.close();
    
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    ElementType deserializedType;
    inStream >> deserializedType;
    
    QCOMPARE(deserializedType, ElementType::Or);
}

void TestEnums::testSerializationRoundTrip()
{
    // Test all defined element types for serialization round-trip
    QVector<ElementType> allTypes = {
        ElementType::Unknown, ElementType::And, ElementType::Or, ElementType::Nand,
        ElementType::Nor, ElementType::Xor, ElementType::Xnor, ElementType::Not,
        ElementType::InputButton, ElementType::InputSwitch, ElementType::Led,
        ElementType::Clock, ElementType::InputVcc, ElementType::InputGnd,
        ElementType::Display7, ElementType::DLatch, ElementType::JKLatch,
        ElementType::DFlipFlop, ElementType::JKFlipFlop, ElementType::SRFlipFlop,
        ElementType::TFlipFlop, ElementType::InputRotary, ElementType::IC,
        ElementType::Node, ElementType::Mux, ElementType::Demux, ElementType::Buzzer,
        ElementType::Display14, ElementType::Text, ElementType::Line,
        ElementType::TruthTable, ElementType::AudioBox, ElementType::SRLatch,
        ElementType::Display16
    };
    
    for (ElementType type : allTypes) {
        testElementTypeSerialization(type);
    }
}

void TestEnums::testInvalidSerializationData()
{
    // Test deserialization of invalid/out-of-range values
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    
    // Write an invalid element type value
    quint64 invalidValue = 9999;
    outStream << invalidValue;
    buffer.close();
    
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    ElementType deserializedType;
    inStream >> deserializedType;
    
    // Should deserialize as the cast value, even if invalid
    QCOMPARE(static_cast<quint64>(deserializedType), invalidValue);
}

void TestEnums::testElementTypeIncrementOperator()
{
    // Test prefix increment operator
    ElementType type = ElementType::And;
    ElementType incremented = ++type;
    
    QCOMPARE(static_cast<int>(incremented), static_cast<int>(ElementType::And) + 1);
    QCOMPARE(type, incremented);
}

void TestEnums::testIncrementOverflow()
{
    // Test increment with high values
    ElementType highType = static_cast<ElementType>(100);
    ElementType incremented = ++highType;
    
    QCOMPARE(static_cast<int>(incremented), 101);
}

void TestEnums::testAllDefinedElementTypes()
{
    // Verify all element types defined in enum are handled
    QVector<int> definedValues = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
    };
    
    for (int value : definedValues) {
        ElementType type = static_cast<ElementType>(value);
        
        // Test that cycling functions handle all defined types
        ElementType next = Enums::nextElmType(type);
        ElementType prev = Enums::prevElmType(type);
        
        // Should not crash and should return valid ElementType values
        QVERIFY(static_cast<int>(next) >= 0);
        QVERIFY(static_cast<int>(prev) >= 0);
    }
}

void TestEnums::testElementTypeBoundaries()
{
    // Test boundary conditions for element type values
    ElementType minType = static_cast<ElementType>(0);
    ElementType maxDefinedType = static_cast<ElementType>(33);
    
    // Should handle boundary values without crashing
    Enums::nextElmType(minType);
    Enums::prevElmType(minType);
    Enums::nextElmType(maxDefinedType);
    Enums::prevElmType(maxDefinedType);
    
    QVERIFY(true); // If we reach here, no crashes occurred
}

void TestEnums::testSerializationDataIntegrity()
{
    // Test that serialization preserves exact values
    QVector<ElementType> criticalTypes = {
        ElementType::Unknown, ElementType::And, ElementType::Or,
        ElementType::InputButton, ElementType::Led, ElementType::Clock
    };
    
    for (ElementType type : criticalTypes) {
        QByteArray data;
        QBuffer buffer(&data);
        
        // Serialize
        buffer.open(QIODevice::WriteOnly);
        QDataStream outStream(&buffer);
        outStream << type;
        buffer.close();
        
        // Deserialize
        buffer.open(QIODevice::ReadOnly);
        QDataStream inStream(&buffer);
        ElementType deserializedType;
        inStream >> deserializedType;
        
        // Verify exact match
        QCOMPARE(static_cast<int>(deserializedType), static_cast<int>(type));
    }
}

// Helper methods

void TestEnums::testCycleGroup(const QVector<ElementType> &group)
{
    if (group.size() < 2) return;
    
    // Test that each element in the group cycles to the next
    for (int i = 0; i < group.size(); ++i) {
        ElementType current = group[i];
        ElementType expected_next = group[(i + 1) % group.size()];
        ElementType expected_prev = group[(i - 1 + group.size()) % group.size()];
        
        QCOMPARE(Enums::nextElmType(current), expected_next);
        QCOMPARE(Enums::prevElmType(current), expected_prev);
    }
}

void TestEnums::validateCycleConsistency(ElementType start, ElementType next, ElementType prev)
{
    if (next != ElementType::Unknown && prev != ElementType::Unknown) {
        // If both operations return valid results, they should be inverses
        if (start != next) {
            QCOMPARE(Enums::prevElmType(next), start);
        }
        if (start != prev) {
            QCOMPARE(Enums::nextElmType(prev), start);
        }
    }
}

void TestEnums::testElementTypeSerialization(ElementType type)
{
    QByteArray data;
    QBuffer buffer(&data);
    
    // Serialize
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    outStream << type;
    buffer.close();
    
    // Deserialize
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    ElementType deserializedType;
    inStream >> deserializedType;
    
    // Verify round-trip integrity
    QCOMPARE(deserializedType, type);
}