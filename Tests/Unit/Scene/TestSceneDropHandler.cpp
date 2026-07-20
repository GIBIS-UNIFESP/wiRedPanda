// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneDropHandler.h"

#include <QDataStream>
#include <QGraphicsSceneEvent>
#include <QMimeData>

#include "App/Core/MimeTypes.h"
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
