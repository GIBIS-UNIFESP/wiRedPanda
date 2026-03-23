// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Factory/TestElementFactory.h"

#include <memory>

#include <QMetaEnum>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/Element/GraphicElement.h"
#include "Tests/Common/TestUtils.h"

void TestElementFactory::initTestCase()
{
    // Initialize Qt translation system if needed
}

// Element Creation Tests

void TestElementFactory::testBuildElement_data()
{
    // Test data: all valid element types from testTextToType_data()
    QTest::addColumn<int>("elementType");

    QTest::newRow("And") << static_cast<int>(ElementType::And);
    QTest::newRow("Or") << static_cast<int>(ElementType::Or);
    QTest::newRow("Not") << static_cast<int>(ElementType::Not);
    QTest::newRow("Nand") << static_cast<int>(ElementType::Nand);
    QTest::newRow("Nor") << static_cast<int>(ElementType::Nor);
    QTest::newRow("Xor") << static_cast<int>(ElementType::Xor);
    QTest::newRow("Xnor") << static_cast<int>(ElementType::Xnor);
    QTest::newRow("InputButton") << static_cast<int>(ElementType::InputButton);
    QTest::newRow("InputSwitch") << static_cast<int>(ElementType::InputSwitch);
    QTest::newRow("InputRotary") << static_cast<int>(ElementType::InputRotary);
    QTest::newRow("Clock") << static_cast<int>(ElementType::Clock);
    QTest::newRow("Led") << static_cast<int>(ElementType::Led);
    QTest::newRow("DFlipFlop") << static_cast<int>(ElementType::DFlipFlop);
    QTest::newRow("JKFlipFlop") << static_cast<int>(ElementType::JKFlipFlop);
    QTest::newRow("Mux") << static_cast<int>(ElementType::Mux);
    QTest::newRow("Demux") << static_cast<int>(ElementType::Demux);
    QTest::newRow("Node") << static_cast<int>(ElementType::Node);
}

void TestElementFactory::testBuildElement()
{
    QFETCH(int, elementType);
    const auto type = static_cast<ElementType>(elementType);

    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(type));
    QVERIFY2(elem != nullptr, qPrintable(QString("Failed to build %1").arg(ElementFactory::typeToText(type))));
    QCOMPARE(elem->elementType(), type);
    QVERIFY2(elem->inputSize() >= elem->minInputSize(),
             qPrintable(QString("%1 has %2 inputs but minimum is %3")
                       .arg(ElementFactory::typeToText(type), QString::number(elem->inputSize()),
                            QString::number(elem->minInputSize()))));
    QVERIFY2(elem->outputSize() >= elem->minOutputSize(),
             qPrintable(QString("%1 has %2 outputs but minimum is %3")
                       .arg(ElementFactory::typeToText(type), QString::number(elem->outputSize()),
                            QString::number(elem->minOutputSize()))));
}

void TestElementFactory::testBuildElementInvalidType()
{
    // Should throw exception for Unknown type
    bool exceptionThrown = false;
    try {
        auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Unknown));
    } catch (const Pandaception &) {
        exceptionThrown = true;
    }
    QVERIFY2(exceptionThrown, "Expected exception for Unknown element type");
}

void TestElementFactory::testMetadataLogicCreator()
{
    // Test logic element creation via metadata logicCreator for various element types
    const QVector<ElementType> typesToTest{
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Nand, ElementType::Nor, ElementType::Xor
    };

    for (const auto type : typesToTest) {
        auto *graphicElem = ElementFactory::buildElement(type);
        QVERIFY2(graphicElem != nullptr, qPrintable(QString("Failed to build graphic element: %1").arg(ElementFactory::typeToText(type))));

        const auto &meta = ElementMetadataRegistry::metadata(type);
        QVERIFY2(meta.logicCreator != nullptr, qPrintable(QString("%1 should have a logicCreator").arg(ElementFactory::typeToText(type))));

        auto logicElem = meta.logicCreator(graphicElem);
        QVERIFY2(logicElem != nullptr, qPrintable(QString("Failed to create logic element: %1").arg(ElementFactory::typeToText(type))));
        QVERIFY2(logicElem.use_count() > 0, "Logic element should have valid reference count");

        delete graphicElem;
    }
}

// Type Conversion Tests

void TestElementFactory::testTextToType_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("expectedType");

    QTest::newRow("And") << "And" << static_cast<int>(ElementType::And);
    QTest::newRow("Or") << "Or" << static_cast<int>(ElementType::Or);
    QTest::newRow("Not") << "Not" << static_cast<int>(ElementType::Not);
    QTest::newRow("Nand") << "Nand" << static_cast<int>(ElementType::Nand);
    QTest::newRow("Nor") << "Nor" << static_cast<int>(ElementType::Nor);
    QTest::newRow("Xor") << "Xor" << static_cast<int>(ElementType::Xor);
    QTest::newRow("Xnor") << "Xnor" << static_cast<int>(ElementType::Xnor);
    QTest::newRow("InputButton") << "InputButton" << static_cast<int>(ElementType::InputButton);
    QTest::newRow("InputSwitch") << "InputSwitch" << static_cast<int>(ElementType::InputSwitch);
    QTest::newRow("InputRotary") << "InputRotary" << static_cast<int>(ElementType::InputRotary);
    QTest::newRow("Clock") << "Clock" << static_cast<int>(ElementType::Clock);
    QTest::newRow("Led") << "Led" << static_cast<int>(ElementType::Led);
    QTest::newRow("DFlipFlop") << "DFlipFlop" << static_cast<int>(ElementType::DFlipFlop);
    QTest::newRow("JKFlipFlop") << "JKFlipFlop" << static_cast<int>(ElementType::JKFlipFlop);
    QTest::newRow("Mux") << "Mux" << static_cast<int>(ElementType::Mux);
    QTest::newRow("Demux") << "Demux" << static_cast<int>(ElementType::Demux);
    QTest::newRow("Node") << "Node" << static_cast<int>(ElementType::Node);
    // Invalid - keyToValue returns -1 for invalid keys
    QTest::newRow("Invalid") << "NonexistentType" << -1;
}

void TestElementFactory::testTextToType()
{
    QFETCH(QString, text);
    QFETCH(int, expectedType);

    const auto result = ElementFactory::textToType(text);
    QCOMPARE(static_cast<int>(result), expectedType);
}

void TestElementFactory::testTypeToTextBidirectional()
{
    // Test round-trip conversion for all valid element types
    // Uses the same type list as testBuildElement_data() for consistency
    const QVector<ElementType> validTypes{
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Nand, ElementType::Nor, ElementType::Xor, ElementType::Xnor,
        ElementType::InputButton, ElementType::InputSwitch, ElementType::InputRotary,
        ElementType::Clock, ElementType::Led,
        ElementType::DFlipFlop, ElementType::JKFlipFlop,
        ElementType::Mux, ElementType::Demux,
        ElementType::Node
    };

    for (const auto type : validTypes) {
        const auto text = ElementFactory::typeToText(type);
        QVERIFY2(!text.isEmpty() && text != "Unknown",
                 qPrintable(QString("Type %1 should have valid name").arg(static_cast<int>(type))));

        // Round-trip: type → text → type
        const auto roundTrip = ElementFactory::textToType(text);
        QCOMPARE(roundTrip, type);
    }
}

// Registry Management Tests

void TestElementFactory::testItemRegistry()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Verify element has ID
    const int id = elem->id();
    QVERIFY2(id > 0, "Element ID should be positive");

    // Verify registry contains item
    QVERIFY2(ElementFactory::contains(id), qPrintable(QString("Registry missing ID: %1").arg(id)));
    QCOMPARE(ElementFactory::itemById(id), elem.get());

    // Remove and verify
    elem.reset();  // Destructor calls removeItem()
    QVERIFY2(!ElementFactory::contains(id), "Item should be removed from registry after deletion");
}

void TestElementFactory::testUniqueIdAssignment()
{
    QSet<int> ids;
    QVector<GraphicElement *> elements;

    // Create multiple elements
    for (int i = 0; i < 20; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::Node);
        QVERIFY(elem != nullptr);
        elements.append(elem);

        const int id = elem->id();
        QVERIFY2(!ids.contains(id), qPrintable(QString("Duplicate ID assigned: %1").arg(id)));
        ids.insert(id);
    }

    // Verify all IDs are unique
    QCOMPARE(ids.size(), 20);

    // Cleanup
    qDeleteAll(elements);
}

void TestElementFactory::testMetadataRegistry()
{
    // Test that element metadata properties are non-empty and consistent
    const QVector<ElementType> typesToTest{
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Led, ElementType::InputSwitch, ElementType::Clock
    };

    // Test 1: Properties should be retrievable and non-empty via metadata registry
    for (const auto type : typesToTest) {
        const auto &meta = ElementMetadataRegistry::metadata(type);
        QVERIFY2(!meta.pixmapPath().isEmpty(),
                 qPrintable(QString("%1 should have non-empty pixmap path").arg(ElementFactory::typeToText(type))));
        QVERIFY2(meta.translatedName != nullptr && meta.translatedName[0] != '\0',
                 qPrintable(QString("%1 should have non-empty translated name").arg(ElementFactory::typeToText(type))));
    }

    // Test 2: Repeated calls return identical values (registry is stable)
    for (const auto type : typesToTest) {
        const auto &meta1 = ElementMetadataRegistry::metadata(type);
        const auto &meta2 = ElementMetadataRegistry::metadata(type);
        QCOMPARE(meta1.pixmapPath(), meta2.pixmapPath());
        QCOMPARE(meta1.translatedName, meta2.translatedName);
    }

    // Test 3: Factory convenience methods match metadata registry values
    for (const auto type : typesToTest) {
        const auto &meta = ElementMetadataRegistry::metadata(type);
        QCOMPARE(ElementFactory::translatedName(type), QString(meta.translatedName));
        QCOMPARE(ElementFactory::typeToTitleText(type), QString(meta.titleText));
    }
}

void TestElementFactory::testAllElementTypesRegistered()
{
    // Verify that all ElementType values (except Unknown and deprecated ones)
    // have both metadata and factory creator registrations.
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));

        // Skip special cases: Unknown type and deprecated JKLatch
        if (type == ElementType::Unknown) {
            continue;
        }
        if (type == ElementType::JKLatch) {
            continue;  // deprecated: JKLatch element no longer exists, kept for backward compatibility
        }

        QVERIFY2(ElementMetadataRegistry::hasMetadata(type),
                 qPrintable(QString("Missing ElementInfo<> for %1").arg(metaEnum.key(i))));

        QVERIFY2(ElementFactory::hasCreator(type),
                 qPrintable(QString("Missing factory creator for %1").arg(metaEnum.key(i))));
    }
}

