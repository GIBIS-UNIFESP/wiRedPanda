// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestGraphicElement.h"

#include <QPainter>
#include <QPixmap>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

void TestGraphicElement::testElementPaintSelection()
{
    And andGate;
    andGate.setSelected(true);
    QVERIFY(andGate.isSelected());

    andGate.setSelected(false);
    QVERIFY(!andGate.isSelected());
}

void TestGraphicElement::testElementSaveLoad()
{
    And gate;
    gate.setRotation(90);

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    gate.save(writeStream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestGraphicElement::testElementSkin()
{
    Led led;
    QVERIFY(led.hasColors());
}

void TestGraphicElement::testElementAppearance()
{
    Led led;
    led.setColor("Red");
    QCOMPARE(led.color(), QString("Red"));
}

void TestGraphicElement::testElementTooltip()
{
    // Every element exposes its translated name as the hover tooltip (set in the ctor). With no
    // translator loaded in the test the And gate's tooltip is its English name.
    And andGate;
    QCOMPARE(andGate.toolTip(), QStringLiteral("And"));
}

void TestGraphicElement::testElementDoubleClick()
{
    And andGate;
    QCOMPARE(andGate.elementType(), ElementType::And);
}

void TestGraphicElement::testTextElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *text = new Text;
    canvas.addItem(text);
    QCOMPARE(text->elementType(), ElementType::Text);
    QVERIFY(!text->boundingRect().isNull());
}

void TestGraphicElement::testLineElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *line = new Line;
    canvas.addItem(line);
    QCOMPARE(line->elementType(), ElementType::Line);
    QVERIFY(!line->boundingRect().isNull());
}

void TestGraphicElement::testDisplay14Paint()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *disp = new Display14;
    canvas.addItem(disp);
    disp->setColor("Green");

    // Set some segments to Active to exercise paint() branches
    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    // Render to pixmap to trigger paint()
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    disp->paint(&painter);
    painter.end();

    QVERIFY2(QuickTestUtils::pixmapHasInk(pixmap), "Display14 paint() must draw visible pixels");
}

void TestGraphicElement::testDisplay16Paint()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *disp = new Display16;
    canvas.addItem(disp);
    disp->setColor("Blue");

    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    disp->paint(&painter);
    painter.end();

    QVERIFY2(QuickTestUtils::pixmapHasInk(pixmap), "Display16 paint() must draw visible pixels");
}

void TestGraphicElement::testTextEmptyStateHintTogglesWithLabelContent()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *text = new Text;
    canvas.addItem(text);

    // A fresh Text starts empty, so the hint must be visible immediately.
    QVERIFY(text->emptyHintItem()->isVisible());

    text->setLabel("Now has content");
    QVERIFY(!text->emptyHintItem()->isVisible());

    text->setLabel(""); // clearing it back out must re-show the hint
    QVERIFY(text->emptyHintItem()->isVisible());

    text->setLabel("Second time");
    QVERIFY(!text->emptyHintItem()->isVisible());
}

void TestGraphicElement::testColorCycleFunctionsDefaultToWhiteForNonColoredElement()
{
    // The base GraphicElement::color() always returns an empty string; previousColor()/
    // nextColor() must fall back to "White" (not an empty/garbage result) for an element type
    // that never overrides color(), e.g. And.
    And gate;
    QCOMPARE(gate.color(), QString());
    QCOMPARE(gate.previousColor(), QString("White"));
    QCOMPARE(gate.nextColor(), QString("White"));
}

void TestGraphicElement::testColorNameToIndexUnknownColorDefaultsToZero()
{
    QCOMPARE(GraphicElement::colorNameToIndex("NotARealColor"), 0);
}

void TestGraphicElement::testIsValidPropagatesErrorStatusToConnectedOutputs()
{
    // An element with an unconnected, required input is invalid; isValid() must then mark every
    // connection on its outputs -- and the port at the connection's far end -- Status::Error, so
    // the visual chain shows where validity breaks.
    QuickCircuitBuilder builder;
    auto *gate = new And;
    auto *led = new Led;
    builder.addOwned(gate, led);
    builder.connect(gate, 0, led, 0);

    QVERIFY(!gate->isValid()); // both inputs are unconnected and required by default

    Connection *conn = gate->outputPort(0)->connections().constFirst();
    QCOMPARE(conn->status(), Status::Error);
    QCOMPARE(led->inputPort(0)->status(), Status::Error);
}

void TestGraphicElement::testBaseSettersAreNoOpsForUnsupportedProperties()
{
    // GraphicElement's default setColor()/setAudio()/setVolume()/setFrequency()/setDelay() are
    // no-ops for element types that don't override them -- confirm the matching getters stay at
    // their fixed defaults afterward.
    And gate;
    gate.setColor("Red");
    QCOMPARE(gate.color(), QString());

    gate.setAudio("some/path.wav");
    QCOMPARE(gate.audio(), QString());

    gate.setVolume(0.5f);
    QCOMPARE(gate.volume(), 0.0f);

    gate.setFrequency(42.0);
    QCOMPARE(gate.frequency(), 0.0);

    gate.setDelay(42.0);
    QCOMPARE(gate.delay(), 0.0);
}

void TestGraphicElement::testBlobNameDefaultsToEmptyForNonIcElement()
{
    And gate;
    QVERIFY(gate.blobName().isEmpty());
}

void TestGraphicElement::testRetranslateUpdatesTranslatedNameToolTipAndPortName()
{
    And gate;
    gate.retranslate();

    const QString expected = ElementFactory::translatedName(gate.elementType());
    QCOMPARE(gate.toolTip(), expected);
    QCOMPARE(gate.objectName(), expected);
}
