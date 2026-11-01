// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Factory/TestElementFactory.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

#include <QList>
#include <QMap>
#include <QMetaEnum>
#include <QSet>
#include <QString>
#include <QStringList>

#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/KernelLogic.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestElementFactory::initTestCase()
{
    // Initialize Qt translation system if needed
}

namespace {
/// Every real, buildable ElementType (excludes the Unknown placeholder and the deprecated,
/// creator-less JKLatch) -- single source of truth shared by testBuildElement_data() and
/// testTypeToTextBidirectional() instead of two independently-maintained hard-coded lists.
QVector<ElementType> buildableElementTypes()
{
    QVector<ElementType> types;
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (type == ElementType::Unknown || type == ElementType::JKLatch) {
            continue;
        }
        types.append(type);
    }
    return types;
}
} // namespace

// Element Creation Tests

void TestElementFactory::testBuildElement_data()
{
    // Every real, buildable ElementType gets this build-time invariant check, not just a
    // hand-picked subset (mirrors testAllElementTypesRegistered()'s own Unknown/JKLatch skip).
    QTest::addColumn<int>("elementType");

    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (type == ElementType::Unknown || type == ElementType::JKLatch) {
            continue;
        }
        QTest::newRow(metaEnum.key(i)) << metaEnum.value(i);
    }
}

void TestElementFactory::testBuildElement()
{
    QFETCH(int, elementType);
    const auto type = static_cast<ElementType>(elementType);

    auto elem = std::unique_ptr<Element>(ElementFactory::buildElement(type));
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
    // A type with no creator is a broken invariant: an assertion, and in a release build a throw to the boundary above,
    // since no element can stand in for it
#ifndef QT_NO_DEBUG
    QSKIP("The assertion stops a debug build; CI's release builds run this");
#endif
    bool exceptionThrown = false;
    try {
        auto elem = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Unknown));
    } catch (const Pandaception &) {
        exceptionThrown = true;
    }
    QVERIFY2(exceptionThrown, "Expected exception for Unknown element type");
}

void TestElementFactory::testDescriptorLogicCreator()
{
    // Verify elements can be built and their descriptor has the type
    const QVector<ElementType> typesToTest{
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Nand, ElementType::Nor, ElementType::Xor
    };

    for (const auto type : typesToTest) {
        auto graphicElem = std::unique_ptr<Element>(ElementFactory::buildElement(type));
        QVERIFY2(graphicElem != nullptr, qPrintable(QString("Failed to build graphic element: %1").arg(ElementFactory::typeToText(type))));

        const auto &descriptor = ElementTypeDescriptor::of(type);
        QVERIFY2(descriptor.type == type, "Descriptor type should match requested type");
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
    // Invalid input maps to Unknown (F9) so callers' `== Unknown` checks
    // catch it — keyToValue's raw -1 is not a valid ElementType.
    QTest::newRow("Invalid") << "NonexistentType" << static_cast<int>(ElementType::Unknown);
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
    // Test round-trip conversion for all valid element types -- shares buildableElementTypes()
    // with testBuildElement_data() instead of an independently-maintained duplicate list.
    for (const auto type : buildableElementTypes()) {
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
    Scene scene;
    auto elem = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Before adding to scene, ID is -1 (unassigned)
    QCOMPARE(elem->id(), -1);

    scene.addItem(elem.get());
    const int id = elem->id();
    QVERIFY2(id > 0, "Element ID should be positive after adding to scene");

    // Verify scene registry contains item
    QVERIFY2(scene.contains(id), qPrintable(QString("Registry missing ID: %1").arg(id)));
    QCOMPARE(scene.itemById(id), elem.get());

    // Remove and verify
    scene.removeItem(elem.get());
    QVERIFY2(!scene.contains(id), "Item should be removed from registry after removal from scene");
}

void TestElementFactory::testUniqueIdAssignment()
{
    Scene scene;
    QSet<int> ids;
    QVector<Element *> elements;

    // Create elements and add to scene to get IDs
    for (int i = 0; i < 20; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::Node);
        QVERIFY(elem != nullptr);
        scene.addItem(elem);
        elements.append(elem);

        const int id = elem->id();
        QVERIFY2(id > 0, "ID should be positive after adding to scene");
        QVERIFY2(!ids.contains(id), qPrintable(QString("Duplicate ID assigned: %1").arg(id)));
        ids.insert(id);
    }

    // Verify all IDs are unique
    QCOMPARE(ids.size(), 20);

    // Cleanup
    qDeleteAll(elements);
}

void TestElementFactory::testCrossSceneIsolation()
{
    // Core regression test for the per-scene ID refactoring.
    // Each scene has its own counter starting from 1, so two scenes will assign
    // the SAME numeric IDs to different elements. The isolation guarantee is:
    //   (a) A scene returns only ITS OWN element for a given ID.
    //   (b) A scene returns null for IDs that only exist in another scene.
    Scene scene1;
    Scene scene2;

    // Add two elements to scene1 (IDs 1 and 2)
    auto *e1a = ElementFactory::buildElement(ElementType::And);
    auto *e1b = ElementFactory::buildElement(ElementType::And);
    scene1.addItem(e1a);
    scene1.addItem(e1b);

    // Add one element to scene2 — its ID will also be 1 (independent counter)
    auto *e2a = ElementFactory::buildElement(ElementType::Or);
    scene2.addItem(e2a);

    const int s1_id1 = e1a->id(); // 1
    const int s1_id2 = e1b->id(); // 2
    const int s2_id1 = e2a->id(); // 1 (same numeric value, different scene)

    QCOMPARE(s1_id1, 1);
    QCOMPARE(s1_id2, 2);
    QCOMPARE(s2_id1, 1); // IDs collide by design — independent counters

    // (a) Each scene resolves its own ID to its own element
    QCOMPARE(scene1.itemById(s1_id1), e1a);
    QCOMPARE(scene1.itemById(s1_id2), e1b);
    QCOMPARE(scene2.itemById(s2_id1), e2a);

    // (a) scene1 resolves ID 1 to e1a, NOT to e2a
    QVERIFY2(scene1.itemById(1) != e2a,
             "scene1 must resolve ID 1 to its own element, not scene2's");

    // (a) scene2 resolves ID 1 to e2a, NOT to e1a
    QVERIFY2(scene2.itemById(1) != e1a,
             "scene2 must resolve ID 1 to its own element, not scene1's");

    // (b) scene2 returns null for ID 2 which only exists in scene1
    QVERIFY2(scene2.itemById(s1_id2) == nullptr,
             "scene2 must return null for IDs that only exist in scene1");
}

void TestElementFactory::testSceneCounterIndependence()
{
    // Each scene maintains its own monotonic ID counter starting from 1.
    // Adding elements to scene1 must not advance scene2's counter.
    Scene scene1;
    Scene scene2;

    // Add several elements to scene1
    QVector<Element *> s1elements;
    for (int i = 0; i < 5; ++i) {
        auto *e = ElementFactory::buildElement(ElementType::And);
        scene1.addItem(e);
        s1elements.append(e);
    }

    // scene1 counter is at 5; scene2 counter is still at 0
    auto elm2 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Or));
    scene2.addItem(elm2.get());

    // scene2 should start its own counter from 1, not from 6
    QCOMPARE(elm2->id(), 1);
}

void TestElementFactory::testDescriptorRegistry()
{
    // Test that the descriptors' properties are non-empty and consistent
    const QVector<ElementType> typesToTest{
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::Led, ElementType::InputSwitch, ElementType::Clock
    };

    // Test 1: Properties should be retrievable and non-empty via the descriptor registry
    for (const auto type : typesToTest) {
        const auto &descriptor = ElementTypeDescriptor::of(type);
        QVERIFY2(!descriptor.pixmapPath.isEmpty(),
                 qPrintable(QString("%1 should have non-empty pixmap path").arg(ElementFactory::typeToText(type))));
        QVERIFY2(descriptor.translatedName != nullptr && descriptor.translatedName[0] != '\0',
                 qPrintable(QString("%1 should have non-empty translated name").arg(ElementFactory::typeToText(type))));
    }

    // Test 2: Repeated calls return identical values (registry is stable)
    for (const auto type : typesToTest) {
        const auto &descriptor1 = ElementTypeDescriptor::of(type);
        const auto &descriptor2 = ElementTypeDescriptor::of(type);
        QCOMPARE(descriptor1.pixmapPath, descriptor2.pixmapPath);
        QCOMPARE(descriptor1.translatedName, descriptor2.translatedName);
    }

    // Test 3: Factory convenience methods match the descriptors' values
    for (const auto type : typesToTest) {
        const auto &descriptor = ElementTypeDescriptor::of(type);
        QCOMPARE(ElementFactory::translatedName(type), QString(descriptor.translatedName));
        QCOMPARE(ElementFactory::typeToTitleText(type), QString(descriptor.titleText));
    }
}

void TestElementFactory::testAllElementTypesRegistered()
{
    // Verify that all ElementType values (except Unknown and deprecated ones)
    // have both a descriptor and a factory creator.
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

        // Verify the descriptor is added
        QVERIFY2(ElementTypeDescriptor::has(type),
                 qPrintable(QString("Missing ElementInfo<> for %1").arg(metaEnum.key(i))));

        // Verify factory creator is registered
        QVERIFY2(ElementFactory::hasCreator(type),
                 qPrintable(QString("Missing factory creator for %1").arg(metaEnum.key(i))));
    }
}

void TestElementFactory::testEachTypeNamesItsKernel_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<int>("kernel");
    QTest::addColumn<QList<int>>("parameters");
    QTest::addColumn<bool>("readsLabel");

    const auto ids = [](const std::initializer_list<ParamId> list) {
        QList<int> result;
        for (const auto id : list) {
            result.append(static_cast<int>(id));
        }
        return result;
    };
    const QMap<ElementType, std::pair<KernelKind, QList<int>>> kernels{
        {ElementType::And, {KernelKind::And, {}}},
        {ElementType::AudioBox, {KernelKind::None, {}}},
        {ElementType::Buzzer, {KernelKind::None, {}}},
        {ElementType::Clock, {KernelKind::Clock, ids({ParamId::Frequency, ParamId::Delay, ParamId::Locked})}},
        {ElementType::DFlipFlop, {KernelKind::DFlipFlop, {}}},
        {ElementType::DLatch, {KernelKind::DLatch, {}}},
        {ElementType::Demux, {KernelKind::Demux, {}}},
        {ElementType::Display14, {KernelKind::None, {}}},
        {ElementType::Display16, {KernelKind::None, {}}},
        {ElementType::Display7, {KernelKind::None, {}}},
        {ElementType::IC, {KernelKind::Instance, {}}},
        {ElementType::InputButton, {KernelKind::Input, ids({ParamId::On})}},
        {ElementType::InputGnd, {KernelKind::Constant, {}}},
        {ElementType::InputRotary, {KernelKind::Input, ids({ParamId::Position})}},
        {ElementType::InputSwitch, {KernelKind::Input, ids({ParamId::On})}},
        {ElementType::InputVcc, {KernelKind::Constant, {}}},
        {ElementType::JKFlipFlop, {KernelKind::JKFlipFlop, {}}},
        {ElementType::Led, {KernelKind::None, {}}},
        {ElementType::Line, {KernelKind::None, {}}},
        {ElementType::Mux, {KernelKind::Mux, {}}},
        {ElementType::Nand, {KernelKind::Nand, {}}},
        {ElementType::Node, {KernelKind::Node, ids({ParamId::Wireless})}},
        {ElementType::Nor, {KernelKind::Nor, {}}},
        {ElementType::Not, {KernelKind::Not, {}}},
        {ElementType::Or, {KernelKind::Or, {}}},
        {ElementType::SRFlipFlop, {KernelKind::SRFlipFlop, {}}},
        {ElementType::SRLatch, {KernelKind::SRLatch, {}}},
        {ElementType::TFlipFlop, {KernelKind::TFlipFlop, {}}},
        {ElementType::Text, {KernelKind::None, {}}},
        {ElementType::TruthTable, {KernelKind::TruthTable, ids({ParamId::Table})}},
        {ElementType::Xnor, {KernelKind::Xnor, {}}},
        {ElementType::Xor, {KernelKind::Xor, {}}},
    };

    // Every registered type is in the table, so a new type is given its kernel here
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (ElementTypeDescriptor::has(type)) {
            QVERIFY2(kernels.contains(type), metaEnum.key(i));
            const auto &[kernel, parameters] = kernels.value(type);
            // A wireless node pairs with the nodes its label names
            QTest::newRow(metaEnum.key(i)) << type << static_cast<int>(kernel) << parameters << (type == ElementType::Node);
        }
    }
}

void TestElementFactory::testEachTypeNamesItsKernel()
{
    QFETCH(ElementType, type);
    QFETCH(int, kernel);
    QFETCH(QList<int>, parameters);
    QFETCH(bool, readsLabel);

    const auto &descriptor = ElementTypeDescriptor::of(type);
    QCOMPARE(static_cast<int>(descriptor.kernel), kernel);
    QList<int> read;
    for (const auto id : descriptor.kernelParameters) {
        read.append(static_cast<int>(id));
    }
    QCOMPARE(read, parameters);
    QCOMPARE(descriptor.kernelReadsLabel, readsLabel);
    QCOMPARE(descriptor.sourceLevel != nullptr, descriptor.kernel == KernelKind::Input);

    // Each parameter the kernel reads is one the type has
    for (const auto id : descriptor.kernelParameters) {
        QVERIFY(std::any_of(descriptor.parameters.cbegin(), descriptor.parameters.cend(),
                            [id](const ParamSpec &spec) { return spec.id == id; }));
    }

    // A type whose elements have outputs computes them, an IC through its definition; one without computes nothing
    const auto element = std::unique_ptr<Element>(ElementFactory::buildElement(type));
    QCOMPARE(descriptor.kernel != KernelKind::None, (element->outputSize() > 0) || (type == ElementType::IC));
}

void TestElementFactory::testEachKernelIsComputedByItsLogicOrDrivesItself()
{
    // A kind updateLogic() computes has its logic (KernelLogic), or, until its family's subclass goes (P3.23), a
    // subclass that computes it; a source drives its outputs itself, by its rule, with no logic: a constant by its
    // outputs' defaults, each a level, an input by its source level (testEachTypeNamesItsKernel), a clock by its phase.
    // None and Instance have no logic: a decoration or an output computes nothing, an IC its definition.
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (!ElementTypeDescriptor::has(type)) {
            continue;
        }
        const char *key = metaEnum.key(i);
        const KernelKind kind = ElementTypeDescriptor::of(type).kernel;
        const auto element = std::unique_ptr<Element>(ElementFactory::buildElement(type));
        const bool ownClass = (element->metaObject() != &Element::staticMetaObject);

        if (KernelLogic::isSource(kind) || (kind == KernelKind::None) || (kind == KernelKind::Instance)) {
            QVERIFY2(!KernelLogic::of(kind), key);
        } else {
            QVERIFY2(KernelLogic::of(kind) || ownClass, key);
        }
        if (kind == KernelKind::Constant) {
            for (const auto *port : element->outputs()) {
                QVERIFY2((port->defaultValue() == Status::Active) || (port->defaultValue() == Status::Inactive), key);
            }
        }

        // A flip-flop's kernel keeps its last clock and the data inputs it last sampled, which its logic reads by
        // place; no other kind keeps a history
        const QMap<KernelKind, qsizetype> history{
            {KernelKind::DFlipFlop, 2},
            {KernelKind::TFlipFlop, 2},
            {KernelKind::JKFlipFlop, 3},
            {KernelKind::SRFlipFlop, 3},
        };
        QCOMPARE(ElementTypeDescriptor::of(type).kernelState.size(), history.value(kind, 0));
    }
}

void TestElementFactory::testAnOlderFilesPositionsReadAsTheSlotsTheyNamed_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<QList<int>>("dropped");
    QTest::addColumn<QStringList>("pictures");

    // The picture at each position of each type's appearance list when files saved it by position, before slots had
    // keys (plan 3's P3.17), and the positions of the slots dropped since. This table is the released format: a row
    // extended is a slot added, which every older version refuses a file customising (PLAN_03 §3.7), not a stale
    // fixture brought up to date
    const QMap<ElementType, std::pair<QList<int>, QStringList>> positions{
        {ElementType::And, {{}, {":/Components/Logic/and.svg"}}},
        {ElementType::AudioBox, {{}, {":/Components/Output/AudioBox/audioboxOff.svg",
            ":/Components/Output/AudioBox/audioboxOn.svg"}}},
        {ElementType::Buzzer, {{}, {":/Components/Output/Buzzer/BuzzerOff.svg",
            ":/Components/Output/Buzzer/BuzzerOn.svg"}}},
        {ElementType::Clock, {{}, {":/Components/Input/clock0.svg", ":/Components/Input/clock1.svg"}}},
        {ElementType::DFlipFlop, {{}, {":/Components/Memory/Light/D-flipflop.svg"}}},
        {ElementType::DLatch, {{}, {":/Components/Memory/Light/D-latch.svg"}}},
        {ElementType::Demux, {{}, {":/Components/Logic/demux.svg"}}},
        {ElementType::Display14, {{}, {":/Components/Output/Counter/counter_14_off.svg",
            ":/Components/Output/Counter/counter_a.svg", ":/Components/Output/Counter/counter_b.svg",
            ":/Components/Output/Counter/counter_c.svg", ":/Components/Output/Counter/counter_d.svg",
            ":/Components/Output/Counter/counter_e.svg", ":/Components/Output/Counter/counter_f.svg",
            ":/Components/Output/Counter/counter_g1.svg", ":/Components/Output/Counter/counter_g2.svg",
            ":/Components/Output/Counter/counter_h.svg", ":/Components/Output/Counter/counter_j.svg",
            ":/Components/Output/Counter/counter_k.svg", ":/Components/Output/Counter/counter_l.svg",
            ":/Components/Output/Counter/counter_m.svg", ":/Components/Output/Counter/counter_n.svg",
            ":/Components/Output/Counter/counter_dp.svg"}}},
        {ElementType::Display16, {{}, {":/Components/Output/Counter/counter_16_off.svg",
            ":/Components/Output/Counter/counter_a1.svg", ":/Components/Output/Counter/counter_a2.svg",
            ":/Components/Output/Counter/counter_b.svg", ":/Components/Output/Counter/counter_c.svg",
            ":/Components/Output/Counter/counter_d1.svg", ":/Components/Output/Counter/counter_d2.svg",
            ":/Components/Output/Counter/counter_e.svg", ":/Components/Output/Counter/counter_f.svg",
            ":/Components/Output/Counter/counter_g1.svg", ":/Components/Output/Counter/counter_g2.svg",
            ":/Components/Output/Counter/counter_h.svg", ":/Components/Output/Counter/counter_j.svg",
            ":/Components/Output/Counter/counter_k.svg", ":/Components/Output/Counter/counter_l.svg",
            ":/Components/Output/Counter/counter_m.svg", ":/Components/Output/Counter/counter_n.svg",
            ":/Components/Output/Counter/counter_dp.svg"}}},
        {ElementType::Display7, {{}, {":/Components/Output/Counter/counter_off.svg",
            ":/Components/Output/Counter/counter_a.svg", ":/Components/Output/Counter/counter_b.svg",
            ":/Components/Output/Counter/counter_c.svg", ":/Components/Output/Counter/counter_d.svg",
            ":/Components/Output/Counter/counter_e.svg", ":/Components/Output/Counter/counter_f.svg",
            ":/Components/Output/Counter/counter_g.svg", ":/Components/Output/Counter/counter_dp.svg"}}},
        {ElementType::IC, {{}, {}}},
        {ElementType::InputButton, {{}, {":/Components/Input/buttonOff.svg", ":/Components/Input/buttonOn.svg"}}},
        {ElementType::InputGnd, {{}, {":/Components/Input/0.svg"}}},
        {ElementType::InputRotary, {{1}, {":/Components/Input/rotary.svg", ":/Components/Input/rotary_arrow.svg"}}},
        {ElementType::InputSwitch, {{}, {":/Components/Input/switchOff.svg", ":/Components/Input/switchOn.svg"}}},
        {ElementType::InputVcc, {{}, {":/Components/Input/1.svg"}}},
        {ElementType::JKFlipFlop, {{}, {":/Components/Memory/Light/JK-flipflop.svg"}}},
        {ElementType::Led, {{}, {":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/WhiteLed.svg",
            ":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/RedLed.svg",
            ":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/GreenLed.svg",
            ":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/BlueLed.svg",
            ":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/PurpleLed.svg",
            ":/Components/Output/Led//BlackLed.png", ":/Components/Output/Led//NavyBlueLed.png",
            ":/Components/Output/Led//GreenLed.png", ":/Components/Output/Led//TealLed.png",
            ":/Components/Output/Led//DarkRedLed.png", ":/Components/Output/Led//MagentaLed.png",
            ":/Components/Output/Led//OrangeLed.png", ":/Components/Output/Led//LightGrayLed.png",
            ":/Components/Output/Led/LedOff.svg", ":/Components/Output/Led/RoyalLed.png",
            ":/Components/Output/Led/LimeGreenLed.png", ":/Components/Output/Led/AquaLightLed.png",
            ":/Components/Output/Led/RedLed.png", ":/Components/Output/Led/HotPinkLed.png",
            ":/Components/Output/Led/YellowLed.png", ":/Components/Output/Led/WhiteLed.png"}}},
        {ElementType::Line, {{}, {":/Components/Misc/line.svg"}}},
        {ElementType::Mux, {{}, {":/Components/Logic/mux.svg"}}},
        {ElementType::Nand, {{}, {":/Components/Logic/nand.svg"}}},
        {ElementType::Node, {{}, {":/Components/Logic/node.svg"}}},
        {ElementType::Nor, {{}, {":/Components/Logic/nor.svg"}}},
        {ElementType::Not, {{}, {":/Components/Logic/not.svg"}}},
        {ElementType::Or, {{}, {":/Components/Logic/or.svg"}}},
        {ElementType::SRFlipFlop, {{}, {":/Components/Memory/Light/SR-flipflop.svg"}}},
        {ElementType::SRLatch, {{}, {":/Components/Memory/Light/SR-latch.svg"}}},
        {ElementType::TFlipFlop, {{}, {":/Components/Memory/Light/T-flipflop.svg"}}},
        {ElementType::Text, {{1}, {":/Components/Misc/no_text.png", ":/Components/Misc/text.png"}}},
        {ElementType::TruthTable, {{}, {":/Components/Logic/truthtable-rotated.svg"}}},
        {ElementType::Xnor, {{}, {":/Components/Logic/xnor.svg"}}},
        {ElementType::Xor, {{}, {":/Components/Logic/xor.svg"}}},
    };

    // Every registered type is in the table: older files hold its positions
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (ElementTypeDescriptor::has(type)) {
            QVERIFY2(positions.contains(type), metaEnum.key(i));
            const auto &[dropped, pictures] = positions.value(type);
            QTest::newRow(metaEnum.key(i)) << type << dropped << pictures;
        }
    }
}

void TestElementFactory::testAnOlderFilesPositionsReadAsTheSlotsTheyNamed()
{
    QFETCH(ElementType, type);
    QFETCH(QList<int>, dropped);
    QFETCH(QStringList, pictures);

    // Each position reads as the slot whose built-in picture it held, or as a dropped slot's key, which no slot has;
    // a position past them reads as nothing
    const auto &descriptor = ElementTypeDescriptor::of(type);
    for (qsizetype position = 0; position < pictures.size(); ++position) {
        const std::optional<QString> key = descriptor.appearanceKeyAt(position);
        QVERIFY(key.has_value());
        const int slot = descriptor.appearanceSlot(*key);
        QCOMPARE(slot < 0, dropped.contains(static_cast<int>(position)));
        if (slot >= 0) {
            QCOMPARE(descriptor.appearanceSlots.at(slot).picture, pictures.at(position));
            QCOMPARE(descriptor.appearancePosition(*key), position);
        }
    }
    QVERIFY(!descriptor.appearanceKeyAt(pictures.size()).has_value());

    // Each slot has a key of its own, and a position: a released version refuses a file holding an entry past its
    // type's slots, so a slot added with no position would make every older version refuse a file customising it
    QSet<QString> keys;
    for (const AppearanceSlot &slot : descriptor.appearanceSlots) {
        QVERIFY(!slot.key.isEmpty());
        QVERIFY2(!keys.contains(slot.key), qPrintable(slot.key));
        keys.insert(slot.key);
        QVERIFY2(descriptor.appearancePosition(slot.key) >= 0, qPrintable(slot.key));
    }
}

void TestElementFactory::testEachTypesAppearanceKeysAreFixed_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::addColumn<QStringList>("keys");

    // A file names each slot it saves by its key, so a key renamed loses that slot's picture from every file saved
    // before: the keys are fixed here, in the slots' order
    const QMap<ElementType, QStringList> fixed{
        {ElementType::And, {"body"}},
        {ElementType::AudioBox, {"off", "on"}},
        {ElementType::Buzzer, {"off", "on"}},
        {ElementType::Clock, {"off", "on"}},
        {ElementType::DFlipFlop, {"body"}},
        {ElementType::DLatch, {"body"}},
        {ElementType::Demux, {"body"}},
        {ElementType::Display14, {"off", "a", "b", "c", "d", "e", "f", "g1", "g2", "h", "j", "k", "l", "m", "n",
            "dp"}},
        {ElementType::Display16, {"off", "a1", "a2", "b", "c", "d1", "d2", "e", "f", "g1", "g2", "h", "j", "k",
            "l", "m", "n", "dp"}},
        {ElementType::Display7, {"off", "a", "b", "c", "d", "e", "f", "g", "dp"}},
        {ElementType::IC, {}},
        {ElementType::InputButton, {"off", "on"}},
        {ElementType::InputGnd, {"body"}},
        {ElementType::InputRotary, {"body"}},
        {ElementType::InputSwitch, {"off", "on"}},
        {ElementType::InputVcc, {"body"}},
        {ElementType::JKFlipFlop, {"body"}},
        {ElementType::Led, {"white-off", "white-on", "red-off", "red-on", "green-off", "green-on", "blue-off",
            "blue-on", "purple-off", "purple-on", "code-0", "code-1", "code-2", "code-3", "code-4", "code-5",
            "code-6", "code-7", "code-8", "code-9", "code-10", "code-11", "code-12", "code-13", "code-14", "code-15"}},
        {ElementType::Line, {"body"}},
        {ElementType::Mux, {"body"}},
        {ElementType::Nand, {"body"}},
        {ElementType::Node, {"body"}},
        {ElementType::Nor, {"body"}},
        {ElementType::Not, {"body"}},
        {ElementType::Or, {"body"}},
        {ElementType::SRFlipFlop, {"body"}},
        {ElementType::SRLatch, {"body"}},
        {ElementType::TFlipFlop, {"body"}},
        {ElementType::Text, {"body"}},
        {ElementType::TruthTable, {"body"}},
        {ElementType::Xnor, {"body"}},
        {ElementType::Xor, {"body"}},
    };

    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (ElementTypeDescriptor::has(type)) {
            QVERIFY2(fixed.contains(type), metaEnum.key(i));
            QTest::newRow(metaEnum.key(i)) << type << fixed.value(type);
        }
    }
}

void TestElementFactory::testEachTypesAppearanceKeysAreFixed()
{
    QFETCH(ElementType, type);
    QFETCH(QStringList, keys);

    QStringList named;
    for (const AppearanceSlot &slot : ElementTypeDescriptor::of(type).appearanceSlots) {
        named.append(slot.key);
    }
    QCOMPARE(named, keys);
}

void TestElementFactory::testAnInputsSourceLevelComesFromItsParameters()
{
    // The level of each port is the one the element drives it with and answers for, which the core has to reproduce
    const auto agrees = [](const Element &element, const int port, const bool on) {
        const auto *input = qobject_cast<const ElementInput *>(&element);
        const bool driven = element.outputPort(port)->status() == Status::Active;
        return input && (ElementTypeDescriptor::of(element.elementType()).sourceLevel(element, port) == on)
            && (driven == on) && (input->isOn(port) == on);
    };

    // A switch's and a button's port follows its level
    for (const auto type : {ElementType::InputSwitch, ElementType::InputButton}) {
        const auto element = std::unique_ptr<Element>(ElementFactory::buildElement(type));
        QVERIFY(agrees(*element, 0, false));
        QVERIFY(element->setParam(ParamId::On, true));
        QVERIFY(agrees(*element, 0, true));
        QVERIFY(element->setParam(ParamId::On, false));
        QVERIFY(agrees(*element, 0, false));
    }

    // A rotary switch's ports are on one at a time, at its position
    const auto rotary = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::InputRotary));
    rotary->setOutputSize(4);
    QCOMPARE(rotary->outputSize(), 4);
    for (const int position : {2, 0, 3}) {
        QVERIFY(rotary->setParam(ParamId::Position, position));
        for (int port = 0; port < rotary->outputSize(); ++port) {
            QVERIFY2(agrees(*rotary, port, port == position), qPrintable(QStringLiteral("%1 at %2").arg(port).arg(position)));
        }
    }
}

void TestElementFactory::testDescriptorForUnknownTypeIsEmpty()
{
    // A type with no descriptor is a broken invariant: an assertion, and in a release build an empty descriptor, no
    // throw
    QVERIFY(!ElementTypeDescriptor::has(ElementType::Unknown));
#ifndef QT_NO_DEBUG
    QSKIP("The assertion stops a debug build; CI's release builds run this");
#endif
    try {
        const auto &descriptor = ElementTypeDescriptor::of(ElementType::Unknown);
        QCOMPARE(descriptor.type, ElementType::Unknown);
        QCOMPARE(descriptor.translatedName, nullptr);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("of() threw: %1").arg(e.what())));
    }
}

void TestElementFactory::testTranslatedNameForUnknownType()
{
    QCOMPARE(ElementFactory::translatedName(ElementType::Unknown), QString("Unknown"));
}

void TestElementFactory::testDescriptionForTypeWithoutOwnCase()
{
    // IC is a real, always-registered type, but description()'s switch has no case for it --
    // it falls into the default (empty description), unlike every element type listed there.
    QVERIFY(ElementFactory::description(ElementType::IC).isEmpty());
}

void TestElementFactory::testBuildElementForRegisteredEnumValueWithNoCreatorThrows()
{
    // JKLatch is a real (if deprecated) ElementType enum value kept only for backward
    // compatibility -- no ElementInfo<> registers a creator for it, so, like Unknown, it reaches
    // buildElement()'s no-creator assertion, which a release build answers with a throw
    QVERIFY(!ElementFactory::hasCreator(ElementType::JKLatch));
#ifndef QT_NO_DEBUG
    QSKIP("The assertion stops a debug build; CI's release builds run this");
#endif

    bool threw = false;
    try {
        ElementFactory::buildElement(ElementType::JKLatch);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "buildElement() must throw for a registered enum value with no creator");
}
