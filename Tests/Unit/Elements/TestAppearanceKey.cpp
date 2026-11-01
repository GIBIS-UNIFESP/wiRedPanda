// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestAppearanceKey.h"

#include <memory>

#include <QCoreApplication>
#include <QHashFunctions>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Presentation/ElementPresenter.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"

namespace TestAppearanceKeyDetail {

AppearanceKey keyOf(const Element &element)
{
    return ElementPresenter::of(element.elementType()).appearanceKey(element);
}

std::unique_ptr<Element> build(const ElementType type)
{
    return std::unique_ptr<Element>(ElementFactory::buildElement(type));
}

} // namespace TestAppearanceKeyDetail

using namespace TestAppearanceKeyDetail;

void TestAppearanceKey::testAnEmptyTextsKeyHoldsItsHintInTheLanguageShown()
{
    const auto empty = build(ElementType::Text);
    const auto first = build(ElementType::Text);
    first->setLabel("A");
    const auto second = build(ElementType::Text);
    second->setLabel("B");
    const AppearanceKey hint = keyOf(*empty);
    const AppearanceKey labelled = keyOf(*first);

    // The label is drawn apart from the body, so labelled texts share one key
    QVERIFY(labelled != hint);
    QVERIFY(keyOf(*second) == labelled);

    TestUtils::TagEveryStringTranslator translator;
    QCoreApplication::installTranslator(&translator);
    const AppearanceKey translatedHint = keyOf(*empty);
    const AppearanceKey translatedLabelled = keyOf(*first);
    QCoreApplication::removeTranslator(&translator);

    QVERIFY(translatedHint != hint);
    QVERIFY(translatedLabelled == labelled);
    QVERIFY(keyOf(*empty) == hint);
}

void TestAppearanceKey::testAnUnlitDisplaysKeyLeavesOutItsColour()
{
    const auto red = build(ElementType::Display7);
    red->setColor("Red");
    const auto white = build(ElementType::Display7);
    white->setColor("White");
    QVERIFY(keyOf(*red) == keyOf(*white));

    // Lit, a segment shows the colour
    red->inputPort(0)->setStatus(Status::Active);
    white->inputPort(0)->setStatus(Status::Active);
    QVERIFY(keyOf(*red) != keyOf(*white));
}

void TestAppearanceKey::testADialsKeyHoldsHowManyPositionsItMarks()
{
    // At one position, a dial of four and one of eight draw their arrow alike and their marks apart
    const auto four = build(ElementType::InputRotary);
    four->setOutputSize(4);
    const auto eight = build(ElementType::InputRotary);
    eight->setOutputSize(8);
    for (auto *rotary : {four.get(), eight.get()}) {
        QVERIFY(rotary->setParam(ParamId::Position, 1));
    }
    QVERIFY(keyOf(*four) != keyOf(*eight));
}

void TestAppearanceKey::testKeysCompareEveryCoordinateExactly()
{
    AppearanceKey a;
    a.footprint = QRectF(-8, 0, 64, 96);
    AppearanceKey b = a;
    b.footprint.setHeight(96 + 1e-12);

    // QRectF compares fuzzily; a footprint that differs at all is another key
    QVERIFY(a.footprint == b.footprint);
    QVERIFY(a != b);

    // A coordinate of -0 is one of 0, in the hash too
    AppearanceKey zero;
    AppearanceKey negativeZero;
    negativeZero.footprint = QRectF(-0.0, 0, 0, 0);
    negativeZero.rotation = -0.0;
    QVERIFY(zero == negativeZero);
    QCOMPARE(qHash(zero, 7), qHash(negativeZero, 7));
}

void TestAppearanceKey::testEquivalentTurnsShareAKey()
{
    // A picture with labels and a footprint whose icon is counter-oriented: a turn of -90 or 630 degrees draws as one
    // of 270
    for (const auto type : {ElementType::DLatch, ElementType::TruthTable}) {
        const auto turned = build(type);
        turned->setRotation(270);
        const AppearanceKey key = keyOf(*turned);
        QCOMPARE(key.rotation, 270.0);
        for (const qreal rotation : {-90.0, 630.0}) {
            const auto other = build(type);
            other->setRotation(rotation);
            QVERIFY2(keyOf(*other) == key, qPrintable(QString::number(rotation)));
        }
    }
}
