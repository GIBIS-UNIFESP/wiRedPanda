// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPresenters.h"

#include <memory>

#include <QCoreApplication>
#include <QFont>
#include <QGraphicsSimpleTextItem>
#include <QImage>
#include <QKeySequence>
#include <QMetaEnum>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/BodyMeasure.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Element/LabelMeasure.h"
#include "App/Presentation/ElementPresenter.h"
#include "Tests/Common/TestUtils.h"

void TestElementPresenters::testEveryTypeHasAPresenter()
{
    // Every type an element can be built as has a presenter; one that cannot (Unknown, the retired JK latch) has none.
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        QCOMPARE_EQ(ElementPresenter::find(type) != nullptr, ElementFactory::hasCreator(type));
    }
}

void TestElementPresenters::testAnElementsBodyIsItsPresentersMeasure()
{
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if (!ElementFactory::hasCreator(type)) {
            continue;
        }
        const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
        const BodyShape measured = ElementPresenter::of(type).measure(*element);
        QVERIFY2(element->body() == measured, qPrintable(ElementFactory::typeToText(type)));
        QVERIFY2(BodyMeasure::installed().measure(*element) == measured, qPrintable(ElementFactory::typeToText(type)));
        QVERIFY2(element->pixmapCenter() == measured.centre, qPrintable(ElementFactory::typeToText(type)));
    }
}

void TestElementPresenters::testTextDrawsItsHintAsASimpleTextItemDid()
{
    // An empty Text's hint was a child QGraphicsSimpleTextItem at the label's anchor: the presenter draws the same text
    // in the same font and colour, pixel for pixel, and keeps its box in the body
    Text text;
    QGraphicsSimpleTextItem item(QCoreApplication::translate("Text", "Double-click to add text"));
    QFont font = item.font();
    font.setItalic(true);
    item.setFont(font);
    QColor color = ThemeManager::attributes().m_elementLabelColor;
    color.setAlpha(140);
    item.setBrush(color);
    const QPointF anchor(0, 64);

    QCOMPARE(text.body().body, QRectF(0, 0, 64, 64).united(item.boundingRect().translated(anchor)));

    // Drawn below the 64x64 picture, which the item did not cover
    const QSize size(400, 160);
    QStyleOptionGraphicsItem option;
    QImage drawn(size, QImage::Format_ARGB32_Premultiplied);
    drawn.fill(Qt::transparent);
    QPainter painter(&drawn);
    text.paint(&painter, &option, nullptr);
    painter.end();
    QImage expected(size, QImage::Format_ARGB32_Premultiplied);
    expected.fill(Qt::transparent);
    painter.begin(&expected);
    painter.translate(anchor);
    item.paint(&painter, &option, nullptr);
    painter.end();
    const QRect below(0, 64, size.width(), size.height() - 64);
    QImage blank(below.size(), QImage::Format_ARGB32_Premultiplied);
    blank.fill(Qt::transparent);
    QVERIFY2(expected.copy(below) != blank, "the item drew its text");
    QCOMPARE(drawn.copy(below), expected.copy(below));
}

void TestElementPresenters::testEachTypePlacesItsLabel()
{
    const auto shapeOf = [](const ElementType type) {
        const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
        return element->labelShape();
    };
    // Below a 64x64 body, upright, by default; below a node's small body; beside an audio output's
    QCOMPARE(shapeOf(ElementType::And).anchor, QPointF(0, 64));
    QCOMPARE(shapeOf(ElementType::And).rotation, 0);
    QCOMPARE(shapeOf(ElementType::Mux).anchor, QPointF(0, 64));
    QCOMPARE(shapeOf(ElementType::Node).anchor, QPointF(0, 32));
    QCOMPARE(shapeOf(ElementType::Buzzer).anchor, QPointF(64, 34));
    QCOMPARE(shapeOf(ElementType::AudioBox).anchor, QPointF(64, 34));

    // Below the footprint of a truth table, and of an IC, whose name turns to stand along the chip
    TruthTable table;
    QCOMPARE(table.labelShape().anchor, QPointF(30, table.renderBodyBounds().bottom() + 5));
    QCOMPARE(table.labelShape().rotation, 0);
    IC ic;
    ic.loadFile(TestUtils::cpuComponentsDir() + "level1_d_latch.panda", TestUtils::cpuComponentsDir());
    QCOMPARE(ic.labelShape().anchor, QPointF(30, ic.renderBodyBounds().bottom() + 5));
    QCOMPARE(ic.labelShape().rotation, 90);

    // The label child item is placed there: at the anchor of an unturned element, turned with an IC
    std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    gate->setLabel(QStringLiteral("Gate"));
    QCOMPARE(gate->labelSceneBoundingRect().topLeft(), QPointF(0, 64));
    ic.setLabel(QStringLiteral("Latch"));
    const QRectF turned = ic.labelSceneBoundingRect();
    QVERIFY2(turned.height() > turned.width(), qPrintable(QStringLiteral("%1x%2").arg(turned.width()).arg(turned.height())));
}

void TestElementPresenters::testTheLabelBoxIsTheTextAsDrawn_data()
{
    QTest::addColumn<QString>("label");
    QTest::newRow("word") << QStringLiteral("Gate");
    QTest::newRow("lines") << QStringLiteral("two\nlines, the second longer");
    QTest::newRow("combining mark") << QStringLiteral("é́ क्ष");
    QTest::newRow("empty") << QString();
}

void TestElementPresenters::testTheLabelBoxIsTheTextAsDrawn()
{
    // The box is the room the label child item's text takes, as it lays it out
    QFETCH(QString, label);
    std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    gate->setLabel(label);
    QCOMPARE(gate->labelShown(), label);
    QGraphicsSimpleTextItem item(label);
    item.setFont(LabelMeasure::installed().font());
    const QSizeF expected = label.isEmpty() ? QSizeF() : item.boundingRect().size();
    QCOMPARE(gate->labelShape().box, expected);
    QCOMPARE(gate->labelBoundingRect(), label.isEmpty() ? QRectF() : gate->labelSceneBoundingRect());
}

void TestElementPresenters::testTheLabelBoxFollowsTheTrigger()
{
    std::unique_ptr<Element> button(ElementFactory::buildElement(ElementType::InputButton));
    button->setLabel(QStringLiteral("Go"));
    const QSizeF labelled = button->labelShape().box;
    button->setTrigger(QKeySequence(Qt::Key_Space));
    QCOMPARE(button->labelShown(), QStringLiteral("Go (Space)"));
    QVERIFY(button->labelShape().box.width() > labelled.width());
    QCOMPARE(button->labelShape(), LabelMeasure::installed().measure(*button));
}

void TestElementPresenters::testTextsBodyHoldsItsLabel()
{
    // A label longer than the hint widens a Text's body to hold it, so a click on any of it hits the element
    Text text;
    text.setLabel(QStringLiteral("A label much longer than the hint an empty text shows in its place"));
    QVERIFY(text.labelBoundingRect().width() > 200);
    QVERIFY(text.body().body.contains(text.labelBoundingRect()));
}
