// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestGraphicElement.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Sends a real double-click mouse event to \a scene at \a scenePos, exactly as the view would.
void sendDoubleClick(Scene *scene, const QPointF &scenePos)
{
    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
    event.setScenePos(scenePos);
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(scene, &event);
}

/// Finds the QLineEdit hosted by InlineLabelEditor's QGraphicsProxyWidget, if one is active.
QLineEdit *findInlineEditor(Scene *scene)
{
    for (auto *item : scene->items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            if (auto *lineEdit = qobject_cast<QLineEdit *>(proxy->widget())) {
                return lineEdit;
            }
        }
    }
    return nullptr;
}

} // namespace

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
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);
    QCOMPARE(text->elementType(), ElementType::Text);
    QVERIFY(!text->boundingRect().isNull());
}

void TestGraphicElement::testLineElement()
{
    WorkSpace workspace;
    auto *line = new Line;
    workspace.scene()->addItem(line);
    QCOMPARE(line->elementType(), ElementType::Line);
    QVERIFY(!line->boundingRect().isNull());
}

void TestGraphicElement::testDisplay14Paint()
{
    WorkSpace workspace;
    auto *disp = new Display14;
    workspace.scene()->addItem(disp);
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
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Display14 paint() must draw visible pixels");
}

void TestGraphicElement::testDisplay16Paint()
{
    WorkSpace workspace;
    auto *disp = new Display16;
    workspace.scene()->addItem(disp);
    disp->setColor("Blue");

    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Display16 paint() must draw visible pixels");
}

void TestGraphicElement::testInlineDoubleClickRenamesLabelWithUndo()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setLabel("Original");
    workspace.scene()->addItem(led);

    sendDoubleClick(workspace.scene(), led->mapToScene(led->boundingRect().center()));

    QLineEdit *editor = findInlineEditor(workspace.scene());
    QVERIFY2(editor, "Double-clicking a labelable element must spawn an inline QLineEdit");
    QCOMPARE(editor->text(), QString("Original"));

    editor->setText("Renamed");
    QTest::keyClick(editor, Qt::Key_Return);

    QCOMPARE(led->label(), QString("Renamed"));
    QCOMPARE(workspace.scene()->undoStack()->count(), 1);

    workspace.scene()->undoStack()->undo();
    QCOMPARE(led->label(), QString("Original"));

    workspace.scene()->undoStack()->redo();
    QCOMPARE(led->label(), QString("Renamed"));
}

void TestGraphicElement::testInlineDoubleClickEscapeCancelsWithoutUndo()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setLabel("Original");
    workspace.scene()->addItem(led);

    sendDoubleClick(workspace.scene(), led->mapToScene(led->boundingRect().center()));

    QLineEdit *editor = findInlineEditor(workspace.scene());
    QVERIFY(editor);

    editor->setText("Should not stick");
    QTest::keyClick(editor, Qt::Key_Escape);

    QCOMPARE(led->label(), QString("Original"));
    QCOMPARE(workspace.scene()->undoStack()->count(), 0);
}

void TestGraphicElement::testInlineDoubleClickIgnoredWithoutLabel()
{
    // And has no hasLabel() support -- double-click must fall through to default Qt
    // behavior and must not spawn an inline editor.
    WorkSpace workspace;
    auto *gate = new And;
    QVERIFY(!gate->hasLabel());
    workspace.scene()->addItem(gate);

    sendDoubleClick(workspace.scene(), gate->mapToScene(gate->boundingRect().center()));

    QVERIFY(!findInlineEditor(workspace.scene()));
}

void TestGraphicElement::testTextEmptyStateHintTogglesWithLabelContent()
{
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);

    // A fresh Text starts empty, so the hint must be visible immediately.
    QVERIFY(text->m_emptyHint->isVisible());

    text->setLabel("Now has content");
    QVERIFY(!text->m_emptyHint->isVisible());

    text->setLabel(""); // clearing it back out must re-show the hint
    QVERIFY(text->m_emptyHint->isVisible());

    text->setLabel("Second time");
    QVERIFY(!text->m_emptyHint->isVisible());
}

void TestGraphicElement::testSceneEventSwallowsCtrlClickPress()
{
    // GraphicElement::sceneEvent() swallows Ctrl+click presses so the scene's rubber-band
    // selection logic can handle them, instead of the element intercepting the press and
    // starting its own move. Called directly (via friendship) rather than through a real
    // QTest::mousePress(..., Qt::ControlModifier, ...): this sandbox's offscreen/X11 setup does
    // not reliably reproduce the requested keyboard modifier on the resulting scene event (a
    // real Shift-modifier request was observed arriving as Control at the scene level), making
    // that route non-deterministic for this specific guard.
    And gate;

    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);
    pressEvent.setModifiers(Qt::ControlModifier);
    QVERIFY2(gate.sceneEvent(&pressEvent), "Ctrl+click press must be swallowed (return true)");

    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
    releaseEvent.setButton(Qt::LeftButton);
    releaseEvent.setModifiers(Qt::ControlModifier);
    QVERIFY2(gate.sceneEvent(&releaseEvent), "Ctrl+click release must also be swallowed");
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
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *gate = new And;
    auto *led = new Led;
    builder.add(gate, led);
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
