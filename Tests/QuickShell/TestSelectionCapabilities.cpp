// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSelectionCapabilities.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/UI/SelectionCapabilities.h"

void TestSelectionCapabilities::testSelectionCapabilities()
{
    SelectionCapabilities caps = computeCapabilities({});
    QVERIFY(!caps.hasElements);
    QCOMPARE(caps.elementType, ElementType::Unknown);
}

void TestSelectionCapabilities::testDemuxSelectionCannotChangeInputSize()
{
    // Demux derives its input size from its own output size, so the generic input-size
    // combobox must be hidden even when the selection's min/max input range would otherwise
    // allow it.
    auto *demux1 = ElementFactory::buildElement(ElementType::Demux);
    auto *demux2 = ElementFactory::buildElement(ElementType::Demux);

    SelectionCapabilities caps = computeCapabilities({demux1, demux2});

    QVERIFY(caps.hasSameType);
    QCOMPARE(caps.elementType, ElementType::Demux);
    QVERIFY2(!caps.canChangeInputSize, "Demux selections must never allow changing input size directly");

    delete demux1;
    delete demux2;
}

void TestSelectionCapabilities::testSingleElementReflectsItsOwnFeatureFlags()
{
    // Led has colors but not audio/frequency/trigger -- a single-element selection's feature
    // flags must reflect exactly the element's own metadata.
    auto *led = ElementFactory::buildElement(ElementType::Led);

    SelectionCapabilities caps = computeCapabilities({led});

    QVERIFY(caps.hasElements);
    QCOMPARE(caps.elementType, ElementType::Led);
    QVERIFY(caps.hasColors);
    QVERIFY(!caps.hasAudio);
    QVERIFY(!caps.hasFrequency);
    QVERIFY(!caps.hasTrigger);
    // Trivially true for a single-element selection: everything matches itself.
    QVERIFY(caps.hasSameType);
    QVERIFY(caps.hasSameLabel);
    QVERIFY(caps.hasSameColors);
    QVERIFY(caps.canMorph);

    delete led;
}

void TestSelectionCapabilities::testMixedSelectionAndReducesFeatureFlags()
{
    // hasColors is AND-reduced across the selection: Led has it, And doesn't, so the mixed
    // selection's flag must be false even though one of the two elements has it.
    auto *led = ElementFactory::buildElement(ElementType::Led);
    auto *andGate = ElementFactory::buildElement(ElementType::And);

    SelectionCapabilities caps = computeCapabilities({led, andGate});

    QVERIFY2(!caps.hasColors, "hasColors must be false when not every selected element supports it");

    delete led;
    delete andGate;
}

void TestSelectionCapabilities::testHasSameLabelReflectsWhetherLabelsMatch()
{
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and1->setLabel(QStringLiteral("same"));
    and2->setLabel(QStringLiteral("same"));

    QVERIFY(computeCapabilities({and1, and2}).hasSameLabel);

    and2->setLabel(QStringLiteral("different"));
    QVERIFY2(!computeCapabilities({and1, and2}).hasSameLabel,
              "hasSameLabel must be false once the labels diverge");

    delete and1;
    delete and2;
}

void TestSelectionCapabilities::testHasSameTypeFalseAndElementTypeUnknownForMixedTypes()
{
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    SelectionCapabilities caps = computeCapabilities({and1, or1});

    QVERIFY(!caps.hasSameType);
    QCOMPARE(caps.elementType, ElementType::Unknown);

    delete and1;
    delete or1;
}

void TestSelectionCapabilities::testCanChangeInputSizeFalseWhenInputSizeIsFixed()
{
    // Not's input size has no range (min == max == 1), so the input-size combobox must be
    // hidden. Contrast with And, whose min/maxInputSize (2..8) gives it a real range.
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(!computeCapabilities({notGate}).canChangeInputSize,
              "canChangeInputSize must be false when the selection's input-size range is a single value");
    delete notGate;

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(computeCapabilities({andGate}).canChangeInputSize,
              "canChangeInputSize must be true when the selection's input-size range spans more than one value");
    delete andGate;
}

void TestSelectionCapabilities::testCanChangeOutputSizeFalseForTruthTableRegardlessOfRange()
{
    // TruthTable's own minOutputSize/maxOutputSize (1..8) would otherwise allow the output-size
    // combobox, but its output count is controlled by its own dialog instead.
    auto *truthTable = ElementFactory::buildElement(ElementType::TruthTable);

    SelectionCapabilities caps = computeCapabilities({truthTable});

    QVERIFY(caps.hasTruthTable);
    QVERIFY2(!caps.canChangeOutputSize,
              "canChangeOutputSize must be false for a TruthTable even though its output range spans more than one value");

    delete truthTable;
}

void TestSelectionCapabilities::testHasLatchedValueFalseWhenSelectionIncludesInputButton()
{
    // hasLatchedValue requires every selected element to be an Input-group element other than
    // InputButton (a momentary control has no persisted "latched" value to restore).
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    QVERIFY2(computeCapabilities({sw}).hasLatchedValue,
              "A non-InputButton Input-group element alone must have hasLatchedValue true");

    auto *btn = ElementFactory::buildElement(ElementType::InputButton);
    QVERIFY2(!computeCapabilities({sw, btn}).hasLatchedValue,
              "hasLatchedValue must be false once an InputButton joins the selection");

    delete sw;
    delete btn;
}

void TestSelectionCapabilities::testCanMorphAcrossInputAndStaticInputGroups()
{
    // Static inputs (Vcc/Gnd) and regular inputs occupy the same pin role, so morph
    // compatibility crosses the Input/StaticInput group boundary.
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);

    QVERIFY2(computeCapabilities({sw, vcc}).canMorph,
              "canMorph must be true across the Input/StaticInput group pairing");

    delete sw;
    delete vcc;
}

void TestSelectionCapabilities::testCanMorphFalseAcrossUnrelatedGroups()
{
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);

    QVERIFY2(!computeCapabilities({sw, andGate}).canMorph,
              "canMorph must be false across unrelated element groups");

    delete sw;
    delete andGate;
}

void TestSelectionCapabilities::testSameCheckStateAndSameOutputValueForInputElements()
{
    auto *sw1 = qobject_cast<GraphicElementInput *>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *sw2 = qobject_cast<GraphicElementInput *>(ElementFactory::buildElement(ElementType::InputSwitch));
    QVERIFY(sw1 && sw2);

    sw1->setLocked(false);
    sw2->setLocked(false);
    sw1->setOn(true);
    sw2->setOn(true);

    SelectionCapabilities matching = computeCapabilities({sw1, sw2});
    QVERIFY(matching.sameCheckState);
    QVERIFY(matching.hasSameOutputValue);

    sw2->setLocked(true);
    sw2->setOn(false);

    SelectionCapabilities diverging = computeCapabilities({sw1, sw2});
    QVERIFY2(!diverging.sameCheckState, "sameCheckState must be false once isLocked() diverges");
    QVERIFY2(!diverging.hasSameOutputValue, "hasSameOutputValue must be false once outputValue() diverges");

    delete sw1;
    delete sw2;
}
