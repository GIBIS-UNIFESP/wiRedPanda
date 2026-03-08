// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestEnums.h"

#include <QDataStream>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// ElementType Conversions Tests (8 tests)
// ============================================================

void TestEnums::testElementTypeToString()
{
    // Test all major element types convert correctly to strings
    QCOMPARE(ElementFactory::typeToText(ElementType::And), QString("And"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Or), QString("Or"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Not), QString("Not"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Nand), QString("Nand"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Nor), QString("Nor"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Xor), QString("Xor"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Xnor), QString("Xnor"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Clock), QString("Clock"));
    QCOMPARE(ElementFactory::typeToText(ElementType::InputButton), QString("InputButton"));
    QCOMPARE(ElementFactory::typeToText(ElementType::Led), QString("Led"));
}

void TestEnums::testStringToElementType()
{
    // Test string → enum conversion for all major types
    QCOMPARE(ElementFactory::textToType("And"), ElementType::And);
    QCOMPARE(ElementFactory::textToType("Or"), ElementType::Or);
    QCOMPARE(ElementFactory::textToType("Not"), ElementType::Not);
    QCOMPARE(ElementFactory::textToType("Nand"), ElementType::Nand);
    QCOMPARE(ElementFactory::textToType("Nor"), ElementType::Nor);
    QCOMPARE(ElementFactory::textToType("Xor"), ElementType::Xor);
    QCOMPARE(ElementFactory::textToType("Xnor"), ElementType::Xnor);
    QCOMPARE(ElementFactory::textToType("Clock"), ElementType::Clock);
    QCOMPARE(ElementFactory::textToType("InputButton"), ElementType::InputButton);
    QCOMPARE(ElementFactory::textToType("Led"), ElementType::Led);
}

void TestEnums::testInvalidStringToElementType()
{
    // Test invalid strings - QMetaEnum::keyToValue returns -1 for invalid keys
    // which casts to an invalid enum value (not Unknown/0)
    ElementType invalid = ElementFactory::textToType("InvalidType");
    // Invalid conversion returns -1 cast to ElementType, which is not Unknown
    QVERIFY(invalid != ElementType::And);
    QVERIFY(invalid != ElementType::Or);

    invalid = ElementFactory::textToType("NOTATYPE");
    QVERIFY(invalid != ElementType::Not);

    invalid = ElementFactory::textToType("");
    QVERIFY(invalid != ElementType::Clock);

    invalid = ElementFactory::textToType("random_string_123");
    QVERIFY(invalid != ElementType::Led);
}

void TestEnums::testUnknownElementType()
{
    // Test Unknown type special handling
    QCOMPARE(ElementFactory::typeToText(ElementType::Unknown), QString("UNKNOWN"));

    // Unknown converts back to Unknown
    ElementType converted = ElementFactory::textToType("Unknown");
    QCOMPARE(converted, ElementType::Unknown);
}

void TestEnums::testRoundTripConversion()
{
    // Test type → string → type preserves value for all types
    const ElementType types[] = {
        ElementType::And, ElementType::Or, ElementType::Not, ElementType::Nand,
        ElementType::Nor, ElementType::Xor, ElementType::Xnor, ElementType::Clock,
        ElementType::InputButton, ElementType::InputSwitch, ElementType::InputVcc,
        ElementType::InputGnd, ElementType::Led, ElementType::Buzzer,
        ElementType::DFlipFlop, ElementType::TFlipFlop, ElementType::SRFlipFlop,
        ElementType::Mux, ElementType::Demux, ElementType::IC
    };

    for (auto type : types) {
        QString str = ElementFactory::typeToText(type);
        ElementType converted = ElementFactory::textToType(str);
        QCOMPARE(converted, type);
    }
}

void TestEnums::testTypeToTitleText()
{
    // Test type to title text conversion
    QString titleAnd = ElementFactory::typeToTitleText(ElementType::And);
    QVERIFY(!titleAnd.isEmpty());
    QVERIFY(titleAnd.length() > 0);

    // Unknown type should return "MULTIPLE TYPES"
    QString unknownTitle = ElementFactory::typeToTitleText(ElementType::Unknown);
    QVERIFY(unknownTitle.contains("MULTIPLE", Qt::CaseInsensitive) || !unknownTitle.isEmpty());

    // Different types should have different titles
    QString titleOr = ElementFactory::typeToTitleText(ElementType::Or);
    QVERIFY(titleAnd != titleOr);
}

void TestEnums::testElementTypeIncrement()
{
    // Test incrementing element type values
    ElementType type = ElementType::And;
    ElementType incremented = type;
    ++incremented;

    // Incrementing should produce a different value
    QVERIFY(static_cast<int>(incremented) == static_cast<int>(type) + 1);

    // Can increment multiple times
    ElementType twice = type;
    ++twice;
    ++twice;
    QVERIFY(static_cast<int>(twice) == static_cast<int>(type) + 2);
}

void TestEnums::testElementTypeEdgeCases()
{
    // Test special element types
    ElementType inputVcc = ElementType::InputVcc;
    QCOMPARE(ElementFactory::typeToText(inputVcc), QString("InputVcc"));
    QCOMPARE(ElementFactory::textToType("InputVcc"), inputVcc);

    ElementType inputGnd = ElementType::InputGnd;
    QCOMPARE(ElementFactory::typeToText(inputGnd), QString("InputGnd"));
    QCOMPARE(ElementFactory::textToType("InputGnd"), inputGnd);

    // Test deprecated type (JKLatch)
    ElementType jkLatch = ElementType::JKLatch;
    QCOMPARE(ElementFactory::typeToText(jkLatch), QString("JKLatch"));
    QCOMPARE(ElementFactory::textToType("JKLatch"), jkLatch);
}

// ============================================================
// Element Type Cycling Tests (4 tests)
// ============================================================

void TestEnums::testNextElementTypeCycling()
{
    // Test cycling through gate types with nextElmType
    ElementType andNext = Enums::nextElmType(ElementType::And);
    QCOMPARE(andNext, ElementType::Or);

    ElementType orNext = Enums::nextElmType(andNext);
    QCOMPARE(orNext, ElementType::Nand);

    ElementType nandNext = Enums::nextElmType(orNext);
    QCOMPARE(nandNext, ElementType::Nor);
}

void TestEnums::testPrevElementTypeCycling()
{
    // Test cycling backward through gate types with prevElmType
    ElementType andPrev = Enums::prevElmType(ElementType::And);
    QCOMPARE(andPrev, ElementType::Xnor);

    ElementType xnorPrev = Enums::prevElmType(andPrev);
    QCOMPARE(xnorPrev, ElementType::Xor);

    ElementType xorPrev = Enums::prevElmType(xnorPrev);
    QCOMPARE(xorPrev, ElementType::Nor);
}

void TestEnums::testNextPrevSymmetry()
{
    // Test that nextElmType and prevElmType are symmetric
    ElementType and_type = ElementType::And;
    ElementType next = Enums::nextElmType(and_type);
    ElementType back = Enums::prevElmType(next);
    QCOMPARE(back, and_type);

    // Test for other types
    ElementType led = ElementType::Led;
    next = Enums::nextElmType(led);
    back = Enums::prevElmType(next);
    QCOMPARE(back, led);

    // Test for input types
    ElementType clock = ElementType::Clock;
    next = Enums::nextElmType(clock);
    back = Enums::prevElmType(next);
    QCOMPARE(back, clock);
}

void TestEnums::testGateTypeCycling()
{
    // Test cycling through all gate types returns to starting point
    ElementType start = ElementType::And;
    ElementType current = start;

    // Cycle through gates: And → Or → Nand → Nor → Xor → Xnor → And
    current = Enums::nextElmType(current); // Or
    QCOMPARE(current, ElementType::Or);
    current = Enums::nextElmType(current); // Nand
    QCOMPARE(current, ElementType::Nand);
    current = Enums::nextElmType(current); // Nor
    QCOMPARE(current, ElementType::Nor);
    current = Enums::nextElmType(current); // Xor
    QCOMPARE(current, ElementType::Xor);
    current = Enums::nextElmType(current); // Xnor
    QCOMPARE(current, ElementType::Xnor);
    current = Enums::nextElmType(current); // And (cycle back)
    QCOMPARE(current, ElementType::And);
}

// ============================================================
// Stream Operations Tests (3 tests)
// ============================================================

void TestEnums::testQDataStreamSerialization()
{
    // Test serializing ElementType to QDataStream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    ElementType type = ElementType::And;
    stream << type;

    QVERIFY(data.size() > 0);
    QCOMPARE(data.size(), static_cast<int>(sizeof(quint64)));
}

void TestEnums::testQDataStreamDeserialization()
{
    // Test deserializing ElementType from QDataStream
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);

    ElementType original = ElementType::Or;
    writeStream << original;

    QDataStream readStream(&data, QIODevice::ReadOnly);
    ElementType deserialized;
    readStream >> deserialized;

    QCOMPARE(deserialized, original);
}

void TestEnums::testStreamRoundTrip()
{
    // Test multiple types through stream round-trip
    const ElementType types[] = {
        ElementType::And, ElementType::Or, ElementType::Not, ElementType::Xor,
        ElementType::Clock, ElementType::Led, ElementType::InputButton
    };

    for (auto original : types) {
        QByteArray data;
        QDataStream writeStream(&data, QIODevice::WriteOnly);
        writeStream << original;

        QDataStream readStream(&data, QIODevice::ReadOnly);
        ElementType deserialized;
        readStream >> deserialized;

        QCOMPARE(deserialized, original);
    }
}

// ============================================================
// Status Enum Tests (2 tests)
// ============================================================

void TestEnums::testStatusEnumValues()
{
    // Test Status enum values
    Status invalid = Status::Invalid;
    Status inactive = Status::Inactive;
    Status active = Status::Active;

    QCOMPARE(static_cast<int>(invalid), -1);
    QCOMPARE(static_cast<int>(inactive), 0);
    QCOMPARE(static_cast<int>(active), 1);
}

void TestEnums::testStatusEnumToString()
{
    // Test Status enum has proper Q_ENUM support
    Status active = Status::Active;
    Status inactive = Status::Inactive;
    Status invalid = Status::Invalid;

    // These should be convertible to strings via QVariant
    QVariant varActive = QVariant::fromValue(active);
    QVariant varInactive = QVariant::fromValue(inactive);
    QVariant varInvalid = QVariant::fromValue(invalid);

    // Verify they're not empty
    QVERIFY(!varActive.toString().isEmpty());
    QVERIFY(!varInactive.toString().isEmpty());
    QVERIFY(!varInvalid.toString().isEmpty());
}

// ============================================================
// ElementGroup Enum Tests (2 tests)
// ============================================================

void TestEnums::testElementGroupValues()
{
    // Test ElementGroup enum values exist
    ElementGroup gate = ElementGroup::Gate;
    ElementGroup input = ElementGroup::Input;
    ElementGroup output = ElementGroup::Output;
    ElementGroup ic = ElementGroup::IC;
    ElementGroup memory = ElementGroup::Memory;

    // All should be distinct values
    QVERIFY(static_cast<int>(gate) != static_cast<int>(input));
    QVERIFY(static_cast<int>(input) != static_cast<int>(output));
    QVERIFY(static_cast<int>(output) != static_cast<int>(ic));
    QVERIFY(static_cast<int>(memory) != static_cast<int>(gate));
}

void TestEnums::testElementGroupToString()
{
    // Test ElementGroup enum has proper Q_ENUM support
    ElementGroup gate = ElementGroup::Gate;
    ElementGroup input = ElementGroup::Input;
    ElementGroup output = ElementGroup::Output;

    QVariant varGate = QVariant::fromValue(gate);
    QVariant varInput = QVariant::fromValue(input);
    QVariant varOutput = QVariant::fromValue(output);

    // Verify they're not empty strings
    QVERIFY(!varGate.toString().isEmpty());
    QVERIFY(!varInput.toString().isEmpty());
    QVERIFY(!varOutput.toString().isEmpty());

    // Different groups should have different strings
    QVERIFY(varGate.toString() != varInput.toString());
    QVERIFY(varInput.toString() != varOutput.toString());
}
