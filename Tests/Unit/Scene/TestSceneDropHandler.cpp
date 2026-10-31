// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneDropHandler.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneEvent>
#include <QMimeData>
#include <QPointer>
#include <QScrollBar>

#include "App/Core/DragDropPayload.h"
#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElements/And.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/DragPreview.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/SceneDropHandler.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Sends \a mimeData's drag event of \a type to \a scene at \a scenePos.
void sendDragEvent(Scene *scene, const QEvent::Type type, QMimeData *mimeData, const QPointF scenePos)
{
    QGraphicsSceneDragDropEvent event(type);
    event.setMimeData(mimeData);
    event.setScenePos(scenePos);
    event.setProposedAction(Qt::CopyAction);
    event.setPossibleActions(Qt::CopyAction);
    QCoreApplication::sendEvent(scene, &event);
}

/// The scene's drag picture: the one top-level pixmap item, which no element or wire is.
QGraphicsPixmapItem *dragGhost(Scene *scene)
{
    QGraphicsPixmapItem *ghost = nullptr;
    for (auto *item : scene->items()) {
        if (auto *pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(item); pixmapItem && !item->parentItem()) {
            ghost = pixmapItem;
        }
    }
    return ghost;
}

// A new element's drag payload that reads: an And gate, as the palette writes it
QByteArray andPayload(const ElementType type = ElementType::And)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0) << type << QString();
    return data;
}
} // namespace

void TestSceneDropHandler::testIsSupportedDropFormatRecognizesAllFourMimeTypes()
{
    QMimeData dragDrop;
    dragDrop.setData(MimeType::DragDrop, andPayload());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&dragDrop));

    QMimeData dragDropLegacy;
    dragDropLegacy.setData(MimeType::DragDropLegacy, andPayload());
    QVERIFY(SceneDropHandler::isSupportedDropFormat(&dragDropLegacy));

    // A new element's payload from another program that does not read, or names a type this build lacks, is refused
    // while it is dragged
    QMimeData unreadable;
    unreadable.setData(MimeType::DragDrop, QByteArray());
    QVERIFY(!SceneDropHandler::isSupportedDropFormat(&unreadable));
    QMimeData unknownType;
    unknownType.setData(MimeType::DragDrop, andPayload(static_cast<ElementType>(9999)));
    QVERIFY(!SceneDropHandler::isSupportedDropFormat(&unknownType));

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
    // Element::itemChange()'s grid snap doesn't perturb the expected result below.
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

    Element *clone = nullptr;
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

    const QPixmap pixmap = ElementPictures::typePicture(ElementType::And);
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

    Element *added = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::And) {
            added = elm;
        }
    }
    QVERIFY2(added, "handleNewElementDrop() must add a new And element from the payload");
    QVERIFY2(added->isSelected(), "The newly dropped element must end up selected");
    // setPos() here runs after the element is already in the scene, so
    // Element::itemChange()'s 8px half-grid snap applies to the raw computed position.
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

    const QPixmap pixmap = ElementPictures::typePicture(ElementType::Or);
    ElementLabel label(pixmap, ElementType::Or, "");
    QMimeData *mimeData = label.mimeData(); // addFromMimeData() takes ownership (deleteLater)

    const QPointF targetPos(96, 96); // grid-aligned, no snap surprise
    handler.addFromMimeData(mimeData, targetPos);

    QCOMPARE(scene->elements().size(), countBefore + 1);

    Element *added = nullptr;
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

    const QPixmap pixmap = ElementPictures::typePicture(ElementType::Not);
    ElementLabel label(pixmap, ElementType::Not, "");
    QMimeData *mimeData = label.mimeData();

    handler.addFromMimeData(mimeData);

    Element *added = nullptr;
    for (auto *elm : scene->elements()) {
        if (elm->elementType() == ElementType::Not) {
            added = elm;
        }
    }
    QVERIFY2(added, "addFromMimeData() must add a new Not element from the payload");
    QCOMPARE(added->pos(), QPointF(0, 0));
}

void TestSceneDropHandler::testAddFromMimeDataThatDoesNotReadReclaimsTheMimeData()
{
    // The handler owns the payload from the start: one whose header does not read is still deleted
    WorkSpace workspace;
    SceneDropHandler handler(workspace.scene());
    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::DragDrop, QByteArray("not a payload"));
    QPointer<QMimeData> watcher(mimeData);

    QVERIFY_THROWS_EXCEPTION(std::exception, handler.addFromMimeData(mimeData));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QVERIFY(watcher.isNull());
    QCOMPARE(workspace.scene()->elements().size(), 0);
}

void TestSceneDropHandler::testPaletteDragGhostIsWhereTheDropPutsTheElement()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
    auto mimeData = std::unique_ptr<QMimeData>(label.mimeData());
    const DragPreview::Scope preview(label.dragPicture());

    // Off the grid, so the ghost and the drop both snap
    sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, mimeData.get(), QPointF(150, 133));
    const QPointF dropPos(203, 197);
    sendDragEvent(scene, QEvent::GraphicsSceneDragMove, mimeData.get(), dropPos);
    auto *ghost = dragGhost(scene);
    QVERIFY(ghost);
    const QPointF ghostPos = ghost->pos();
    QCOMPARE(ghost->boundingRect().size(), label.pixmap().deviceIndependentSize());

    sendDragEvent(scene, QEvent::GraphicsSceneDrop, mimeData.get(), dropPos);
    QVERIFY(!dragGhost(scene));
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().constFirst()->pos(), ghostPos);
}

void TestSceneDropHandler::testCloneDragGhostIsWhereTheDropPutsTheCopies()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *gate = new And();
    gate->setPos(48, 48);
    scene->addItem(gate);
    gate->setSelected(true);
    // Zoomed in, so the picture holds more pixels than the selection's size in the scene
    workspace.view()->scale(2.0, 2.0);

    const QPointF pressPos = gate->pos() + QPointF(20, 20);
    const DragPreview::Picture picture = scene->clipboardManager()->selectionDragPicture(pressPos);
    QVERIFY(!picture.pixmap.isNull());
    const QSizeF sceneSize = gate->sceneBoundingRect().adjusted(-8, -8, 8, 8).size();
    QVERIFY(picture.pixmap.width() > sceneSize.width());
    // The picture covers the selection at its size in the scene, whatever the pixels it holds
    QCOMPARE(picture.pixmap.deviceIndependentSize(), sceneSize);
    const DragPreview::Scope preview(picture);

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << pressPos << gate->pos();
    Serialization::serialize({gate}, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);

    // A travel off the grid, so the ghost and the drop both snap
    const QPointF dropPos = pressPos + QPointF(101, 37);
    sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, &mimeData, pressPos);
    sendDragEvent(scene, QEvent::GraphicsSceneDragMove, &mimeData, dropPos);
    auto *ghost = dragGhost(scene);
    QVERIFY(ghost);
    const QPointF ghostPos = ghost->pos();

    sendDragEvent(scene, QEvent::GraphicsSceneDrop, &mimeData, dropPos);
    QVERIFY(!dragGhost(scene));
    QCOMPARE(scene->elements().size(), 2);
    const auto selected = scene->selectedElements();
    QCOMPARE(selected.size(), 1);
    // The ghost's corner is as far from the copy as the picture's from the original
    QCOMPARE(ghostPos - selected.constFirst()->pos(), picture.topLeft - gate->pos());
}

void TestSceneDropHandler::testDragGhostLeavesWithTheDrag()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
    auto mimeData = std::unique_ptr<QMimeData>(label.mimeData());
    const DragPreview::Scope preview(label.dragPicture());

    sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, mimeData.get(), QPointF(100, 100));
    QVERIFY(dragGhost(scene));
    sendDragEvent(scene, QEvent::GraphicsSceneDragLeave, mimeData.get(), QPointF(100, 100));
    QVERIFY(!dragGhost(scene));
    QVERIFY(scene->elements().isEmpty());
}

void TestSceneDropHandler::testDragFromAnotherProcessHasNoGhost()
{
    // Another process's drag sets no picture here; the scene accepts it and draws nothing
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
    auto mimeData = std::unique_ptr<QMimeData>(label.mimeData());

    sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, mimeData.get(), QPointF(100, 100));
    sendDragEvent(scene, QEvent::GraphicsSceneDragMove, mimeData.get(), QPointF(120, 100));
    QVERIFY(!dragGhost(scene));
    sendDragEvent(scene, QEvent::GraphicsSceneDrop, mimeData.get(), QPointF(120, 100));
    QCOMPARE(scene->elements().size(), 1);
}

void TestSceneDropHandler::testPaletteDropGrowsTheSceneOverTheElement()
{
    // The scene's first rect is the view's before it is shown, smaller than the viewport, so Qt centres it
    WorkSpace workspace;
    workspace.resize(800, 600);
    workspace.show();
    QVERIFY(QTest::qWaitForWindowExposed(&workspace));
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    const QPointF corner = view->mapToScene(QPoint(0, 0));

    // Near the viewport's far corner, past the scene's first rect
    ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
    auto mimeData = std::unique_ptr<QMimeData>(label.mimeData());
    const QPointF dropPos = view->mapToScene(QPoint(view->viewport()->width() - 40, view->viewport()->height() - 40));
    sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, mimeData.get(), dropPos);
    sendDragEvent(scene, QEvent::GraphicsSceneDrop, mimeData.get(), dropPos);

    QCOMPARE(scene->elements().size(), 1);
    QVERIFY(scene->sceneRect().contains(scene->elements().constFirst()->sceneBoundingRect()));
    QCOMPARE(view->mapToScene(QPoint(0, 0)), corner);
}

void TestSceneDropHandler::testCloneDropKeepsTheViewStill()
{
    // A palette drop, then a copy-drag of the element: the copy lands under the cursor, the view where it was
    for (const auto direction : {Qt::LeftToRight, Qt::RightToLeft}) {
        WorkSpace workspace;
        workspace.setLayoutDirection(direction);
        workspace.resize(800, 600);
        workspace.show();
        QVERIFY(QTest::qWaitForWindowExposed(&workspace));
        Scene *scene = workspace.scene();
        GraphicsView *view = workspace.view();
        const QPointF corner = view->mapToScene(QPoint(0, 0));

        ElementLabel label(ElementPictures::typePicture(ElementType::And), ElementType::And, "");
        auto paletteData = std::unique_ptr<QMimeData>(label.mimeData());
        const QPointF paletteDrop = view->mapToScene(QPoint(400, 300));
        sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, paletteData.get(), paletteDrop);
        sendDragEvent(scene, QEvent::GraphicsSceneDrop, paletteData.get(), paletteDrop);
        QCOMPARE(scene->elements().size(), 1);
        QCOMPARE(view->mapToScene(QPoint(0, 0)), corner);
        auto *gate = scene->elements().constFirst();

        const QPointF pressPos = gate->sceneBoundingRect().center();
        QByteArray itemData;
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        stream << pressPos << gate->pos();
        Serialization::serialize({gate}, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        QMimeData cloneData;
        cloneData.setData(MimeType::CloneDrag, itemData);
        const QPointF dropPos = pressPos + QPointF(160, 96);
        sendDragEvent(scene, QEvent::GraphicsSceneDragEnter, &cloneData, pressPos);
        sendDragEvent(scene, QEvent::GraphicsSceneDragMove, &cloneData, dropPos);
        sendDragEvent(scene, QEvent::GraphicsSceneDrop, &cloneData, dropPos);

        QCOMPARE(scene->elements().size(), 2);
        QCOMPARE(view->mapToScene(QPoint(0, 0)), corner);
    }
}
