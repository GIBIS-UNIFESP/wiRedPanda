// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneDropHandler.h"

#include <QDataStream>
#include <QGraphicsSceneEvent>
#include <QMimeData>

#include "App/Core/DragDropPayload.h"
#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElements/And.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/SceneDropHandler.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestSceneDropHandler::testIsSupportedDropFormatRecognizesAllFourMimeTypes()
{
    QMimeData dragDrop;
    dragDrop.setData(MimeType::DragDrop, QByteArray());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&dragDrop));

    QMimeData dragDropLegacy;
    dragDropLegacy.setData(MimeType::DragDropLegacy, QByteArray());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&dragDropLegacy));

    QMimeData cloneDrag;
    cloneDrag.setData(MimeType::CloneDrag, QByteArray());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&cloneDrag));

    QMimeData cloneDragLegacy;
    cloneDragLegacy.setData(MimeType::CloneDragLegacy, QByteArray());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&cloneDragLegacy));
}

void TestSceneDropHandler::testIsSupportedDropFormatRejectsUnrecognizedMimeType()
{
    QMimeData plainText;
    plainText.setData("text/plain", QByteArray("hello"));
    QVERIFY(!SceneDropHandler::isSupportedDropFormat(&plainText));
}

void TestSceneDropHandler::testHandleCloneDragClonesSelectionAtOffsetPosition()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    SceneDropHandler handler(scene);

    // Positions/offset are multiples of the grid snap unit (Constants::gridSize / 2 == 8) so
    // GraphicElement::itemChange()'s grid snap doesn't perturb the expected result below.
    auto *gate = new And();
    gate->setPos(48, 48);
    scene->addItem(gate);
    gate->setSelected(true);

    const qsizetype countBefore = scene->elements().size();

    // Build a real clone-drag payload matching ClipboardManager::serializeItems()'s wire
    // format: header + drag-start mouse pos + selection centroid + serialized selection.
    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    const QPointF dragStartPos = gate->pos();
    writeStream << dragStartPos;
    writeStream << gate->pos(); // centroid of the single selected element (unused by the reader)
    Serialization::serialize({gate}, writeStream, {.purpose = SerializationPurpose::InMemorySnapshot});

    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    const QPointF dropOffset(96, 0);
    event.setScenePos(dragStartPos + dropOffset);

    handler.handleCloneDrag(&event);

    QCOMPARE(scene->elements().size(), countBefore + 1);

    GraphicElement *clone = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm != gate) {
            clone = elm;
        }
    }
    QVERIFY2(clone, "handleCloneDrag() must add a new cloned element distinct from the original");
    QVERIFY2(clone->isSelected(), "The newly cloned element must end up selected");
    QCOMPARE(clone->pos(), gate->pos() + dropOffset);

    // The original selected element must be untouched by the clone.
    QCOMPARE(gate->pos(), dragStartPos);
}

void TestSceneDropHandler::testHandleCloneDragAcceptsLegacyMimeFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    SceneDropHandler handler(scene);

    auto *gate = new And();
    gate->setPos(48, 48);
    scene->addItem(gate);
    gate->setSelected(true);

    const qsizetype countBefore = scene->elements().size();

    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    const QPointF dragStartPos = gate->pos();
    writeStream << dragStartPos;
    writeStream << gate->pos();
    Serialization::serialize({gate}, writeStream, {.purpose = SerializationPurpose::InMemorySnapshot});

    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDragLegacy, itemData);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    event.setScenePos(dragStartPos);

    handler.handleCloneDrag(&event);

    QCOMPARE(scene->elements().size(), countBefore + 1);
}

void TestSceneDropHandler::testHandleNewElementDropAddsElementAtScenePosition()
{
    // Real palette-drag payload via ElementLabel::mimeData() -- the actual production
    // producer of this wire format (offset, elementType, icFileName, isEmbedded, ...).
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    SceneDropHandler handler(scene);

    const qsizetype countBefore = scene->elements().size();

    const QPixmap pixmap = ElementFactory::pixmap(ElementType::And);
    ElementLabel label(pixmap, ElementType::And, "");
    auto mimeData = std::unique_ptr<QMimeData>(label.mimeData());

    // Decode the payload's real offset from the same bytes handleNewElementDrop() will read,
    // rather than recomputing it independently -- ElementLabel stores the pixmap in a QLabel,
    // whose reported rect() isn't guaranteed to exactly match a freshly-fetched pixmap's rect().
    QDataStream offsetStream(mimeData->data(MimeType::DragDrop));
    Serialization::readPandaHeader(offsetStream);
    const DragDropPayload payload = readDragDropPayload(offsetStream);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(mimeData.get());
    const QPointF dropScenePos(200, 200);
    event.setScenePos(dropScenePos);

    handler.handleNewElementDrop(&event);


    QCOMPARE(scene->elements().size(), countBefore + 1);

    GraphicElement *added = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::And) {
            added = elm;
        }
    }
    QVERIFY2(added, "handleNewElementDrop() must add a new And element from the payload");
    QVERIFY2(added->isSelected(), "The newly dropped element must end up selected");
    // setPos() here runs after the element is already in the scene, so
    // GraphicElement::itemChange()'s 8px half-grid snap applies to the raw computed position.
    const QPointF rawPos = dropScenePos - payload.offset;
    const QPointF snappedPos(qRound(rawPos.x() / 8.0) * 8, qRound(rawPos.y() / 8.0) * 8);
    QCOMPARE(added->pos(), snappedPos);
}

void TestSceneDropHandler::testAddFromMimeDataAddsElementAtGivenScenePos()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    SceneDropHandler handler(scene);

    const qsizetype countBefore = scene->elements().size();

    const QPixmap pixmap = ElementFactory::pixmap(ElementType::Or);
    ElementLabel label(pixmap, ElementType::Or, "");
    QMimeData *mimeData = label.mimeData(); // addFromMimeData() takes ownership (deleteLater)

    const QPointF targetPos(96, 96); // grid-aligned, no snap surprise
    handler.addFromMimeData(mimeData, targetPos);

    QCOMPARE(scene->elements().size(), countBefore + 1);

    GraphicElement *added = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::Or) {
            added = elm;
        }
    }
    QVERIFY2(added, "addFromMimeData() must add a new Or element from the payload");
    QVERIFY2(added->isSelected(), "The newly added element must end up selected");
    QCOMPARE(added->pos(), targetPos);
}

void TestSceneDropHandler::testAddFromMimeDataKeepsDefaultPositionWhenScenePosOmitted()
{
    // With no scenePos argument, the element keeps whatever default position
    // ElementFactory::buildElement() gives it (the scene origin), rather than being moved.
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    SceneDropHandler handler(scene);

    const QPixmap pixmap = ElementFactory::pixmap(ElementType::Not);
    ElementLabel label(pixmap, ElementType::Not, "");
    QMimeData *mimeData = label.mimeData();

    handler.addFromMimeData(mimeData);

    GraphicElement *added = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::Not) {
            added = elm;
        }
    }
    QVERIFY2(added, "addFromMimeData() must add a new Not element from the payload");
    QCOMPARE(added->pos(), QPointF(0, 0));
}
