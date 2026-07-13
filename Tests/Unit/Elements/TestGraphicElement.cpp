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

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
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
