// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPresenters.h"

#include <memory>

#include <QCoreApplication>
#include <QFont>
#include <QGraphicsSimpleTextItem>
#include <QImage>
#include <QMetaEnum>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/BodyMeasure.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Presentation/ElementPresenter.h"

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
