// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestStackingSnapshot.h"

#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QImage>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QTest>
#include <QUndoStack>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// A named namespace: the scene snapshot, which a unity build can put in one translation unit with this file, has
// helpers of the same names.
namespace StackingSnapshot {

QString goldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/StackingSnapshot/";
}

/// A directory of circuits; its goldens are in the subdirectory of goldensDir() named after it.
struct CircuitGroup {
    QString name;
    QString dir;
};

QList<CircuitGroup> circuitGroups()
{
    return {
        {"Examples", TestUtils::examplesDir()},
        {"Fixtures", QString(QUOTE(CURRENTDIR)) + "/Fixtures/"},
    };
}

QString elementName(const GraphicElement *element)
{
    return 'E' + QString::number(element->id()) + ' ' + TestUtils::snapshotType(element->elementType());
}

/// Names an item: an element by its id and type, a port by its element's id and its index, a wire by its id, an
/// element's label or other child by its element, the selection rectangle by its rectangle.
QString itemName(const QGraphicsItem *item)
{
    switch (item->type()) {
    case GraphicElement::Type:
        return elementName(static_cast<const GraphicElement *>(item));
    case Port::Type: {
        const auto *port = static_cast<const Port *>(item);
        const auto *element = port->graphicElement();
        return QString("port %1.%2%3").arg(QString::number(element ? element->id() : -1), QString(port->isInput() ? "i" : "o"),
                                           QString::number(port->index()));
    }
    case Connection::Type:
        return "wire W" + QString::number(static_cast<const Connection *>(item)->id());
    default:
        break;
    }

    if (const auto *parent = item->parentItem(); parent && (parent->type() == GraphicElement::Type)) {
        const auto *element = static_cast<const GraphicElement *>(parent);
        const QString owner = 'E' + QString::number(element->id());
        if (item == SnapshotAccess::label(element)) {
            return "label of " + owner;
        }
        // A Node's wireless indicator is a picture; a Text's hint, a line of text.
        switch (item->type()) {
        case QGraphicsPixmapItem::Type:
            return "picture child of " + owner;
        case QGraphicsSimpleTextItem::Type:
            return "text child of " + owner;
        default:
            return QString("child type %1 of %2").arg(QString::number(item->type()), owner);
        }
    }
    if (const auto *rect = qgraphicsitem_cast<const QGraphicsRectItem *>(item)) {
        const QRectF r = rect->rect();
        return "rectangle " + TestUtils::snapshotPoint(r.topLeft()) + ' ' + TestUtils::snapshotPoint({r.width(), r.height()});
    }
    return "type " + QString::number(item->type());
}

/// The scene's items in the order Qt stacks them, bottom first, each with its z value, and whether it is hidden or
/// selected.
QString stackOrder(const Scene *scene)
{
    QStringList lines;
    for (const auto *item : scene->QGraphicsScene::items(Qt::AscendingOrder)) {
        lines << itemName(item) + " z=" + TestUtils::snapshotNumber(item->zValue()) + (item->isVisible() ? "" : " hidden")
                     + (item->isSelected() ? " selected" : "");
    }
    return lines.join('\n') + '\n';
}

QString stage(const QString &name, const Scene *scene)
{
    return "== " + name + '\n' + stackOrder(scene);
}

constexpr int kWidth = 320;
constexpr int kHeight = 200;

/// The scene's point (8, 8) is the images' top left corner.
constexpr QPointF kOrigin(8, 8);

/// Paints every visible item of \a scene itself, bottom first in the order Qt stacks them, as a view paints them.
QImage paintInStackOrder(const Scene *scene)
{
    QImage image(kWidth, kHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(-kOrigin);

    for (auto *item : scene->QGraphicsScene::items(Qt::AscendingOrder)) {
        if (!item->isVisible()) {
            continue;
        }
        QStyleOptionGraphicsItem option;
        option.state = QStyle::State_Enabled;
        if (item->isSelected()) {
            option.state |= QStyle::State_Selected;
        }
        option.exposedRect = item->boundingRect();
        painter.save();
        painter.setTransform(item->sceneTransform(), true);
        item->paint(&painter, &option, nullptr);
        painter.restore();
    }
    painter.end();
    return image;
}

/// Compares paintInStackOrder() with Qt's own render of the scene, each channel within the pixel tolerance: only the
/// grid's dots, which the scene's background draws at multiples of 16 and the paint leaves out, may differ more.
/// Returns an empty string, or what differs.
QString compareWithSceneRender(Scene *scene)
{
    QImage rendered(kWidth, kHeight, QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::white);
    QPainter painter(&rendered);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(0, 0, kWidth, kHeight), QRectF(kOrigin, QSizeF(kWidth, kHeight)));
    painter.end();

    const QImage painted = paintInStackOrder(scene);
    qsizetype differing = 0;
    qsizetype gridDots = 0;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            if (TestUtils::channelDifference(rendered.pixel(x, y), painted.pixel(x, y)) <= TestUtils::kPixelChannelTolerance) {
                continue;
            }
            if ((x % 16 == 8) && (y % 16 == 8)) {
                ++gridDots;
            } else {
                ++differing;
            }
        }
    }
    if (gridDots == 0) {
        return "the scene's render shows no grid dots, so it was not compared with its background";
    }
    if (differing > 0) {
        return QString::number(differing) + " pixels outside the grid's dots differ from the scene's render";
    }
    return {};
}

/// Three labelled elements overlapping one another, bodies, ports and labels, each added above the last.
QList<GraphicElement *> buildOverlap(Scene *scene)
{
    const QList<std::pair<ElementType, QPointF>> placements{
        {ElementType::And, QPointF(24, 24)},
        {ElementType::Or, QPointF(64, 40)},
        {ElementType::Led, QPointF(40, 40)},
    };
    QList<GraphicElement *> elements;
    for (const auto &[type, position] : placements) {
        auto *element = ElementFactory::buildElement(type);
        element->setPos(position);
        element->setLabel(TestUtils::snapshotType(type));
        scene->addItem(element);
        elements << element;
    }
    return elements;
}

} // namespace StackingSnapshot

void TestStackingSnapshot::testCircuitOrderMatchesGolden_data()
{
    QTest::addColumn<QString>("path");
    for (const auto &group : StackingSnapshot::circuitGroups()) {
        TestUtils::addFileRows(group.dir, {"*.panda"}, false, group.name + '/');
    }
}

void TestStackingSnapshot::testCircuitOrderMatchesGolden()
{
    QFETCH(QString, path);
    const QString missing = TestUtils::requireInputFile(path);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));

    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, path);
    if (QTest::currentTestFailed()) {
        return;
    }

    // The row is "<group>/<file>.panda"; its golden is "<group>/<file>.txt".
    const QString row(QTest::currentDataTag());
    const QString golden = StackingSnapshot::goldensDir() + row.chopped(QStringLiteral(".panda").size()) + ".txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, StackingSnapshot::stackOrder(workspace.scene()));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestStackingSnapshot::testEveryGoldenHasACircuit()
{
    // A golden whose circuit was removed or renamed would otherwise stay in the tree, compared with nothing.
    for (const auto &group : StackingSnapshot::circuitGroups()) {
        const auto goldens = QDir(StackingSnapshot::goldensDir() + group.name).entryInfoList(QDir::Files, QDir::Name);
        QVERIFY2(!goldens.isEmpty(), qPrintable(group.name));
        for (const auto &golden : goldens) {
            const QString circuit = group.dir + golden.completeBaseName() + ".panda";
            QVERIFY2(QFileInfo::exists(circuit), qPrintable(golden.filePath() + " has no circuit " + circuit));
        }
    }
}

void TestStackingSnapshot::testOverlapPaintMatchesSceneRender()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    StackingSnapshot::buildOverlap(scene);

    const QString difference = StackingSnapshot::compareWithSceneRender(scene);
    QVERIFY2(difference.isEmpty(), qPrintable(difference));
}

void TestStackingSnapshot::testOrderAfterUndoMatchesGolden()
{
    // The bottom of three overlapping elements is deleted and the delete undone; in a second scene, so that it starts at
    // the bottom too, it is morphed and the morph undone. On master the undo, the morph and the morph's undo each add
    // the element with its old id, on top.
    QString dump;
    {
        WorkSpace workspace;
        auto *scene = workspace.scene();
        auto *bottom = StackingSnapshot::buildOverlap(scene).constFirst();
        dump += StackingSnapshot::stage("built", scene);

        scene->receiveCommand(new DeleteItemsCommand({bottom}, scene));
        scene->undoStack()->undo();
        dump += StackingSnapshot::stage("delete undone", scene);
    }

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *bottom = StackingSnapshot::buildOverlap(scene).constFirst();
    const int bottomId = bottom->id();
    scene->receiveCommand(new MorphCommand({bottom}, ElementType::Xor, scene));
    auto *morphed = dynamic_cast<GraphicElement *>(scene->itemById(bottomId));
    QVERIFY2(morphed && morphed->elementType() == ElementType::Xor, "the morph did not keep the element's id");
    dump += StackingSnapshot::stage("morphed", scene);

    scene->undoStack()->undo();
    dump += StackingSnapshot::stage("morph undone", scene);

    const QString mismatch = TestUtils::compareWithGolden(StackingSnapshot::goldensDir() + "undo_order.txt", dump);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestStackingSnapshot::testRubberBandOverOverlapMatchesGolden()
{
    // A rubber band dragged from empty space across the overlapping elements, held before the release: where the
    // selection rectangle stacks and what it has selected, and the paint of it all against Qt's render.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    StackingSnapshot::buildOverlap(scene);

    TestUtils::ScenePointer pointer(scene);
    pointer.press(QPointF(12, 12));
    pointer.move(QPointF(60, 60));
    pointer.move(QPointF(100, 84));

    const QString mismatch = TestUtils::compareWithGolden(StackingSnapshot::goldensDir() + "rubber_band.txt",
                                                          StackingSnapshot::stackOrder(scene));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
    const QString difference = StackingSnapshot::compareWithSceneRender(scene);
    QVERIFY2(difference.isEmpty(), qPrintable(difference));
    pointer.release(QPointF(100, 84));
}

void TestStackingSnapshot::testWireRoutedAcrossOverlapMatchesGolden()
{
    // A wire started from a fourth element's output and routed across the overlapping elements, before it ends on a
    // port: where the pending wire stacks, and the paint against Qt's render. Rendering is on, so the wire has a path.
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    StackingSnapshot::buildOverlap(scene);
    auto *source = ElementFactory::buildElement(ElementType::And);
    source->setPos(QPointF(-48, 120));
    scene->addItem(source);

    TestUtils::ScenePointer pointer(scene);
    pointer.click(source->outputPort(0)->scenePos());
    pointer.move(QPointF(80, 60));
    pointer.move(QPointF(160, 72));
    QVERIFY(scene->connectionManager()->hasEditedConnection());

    const QString mismatch = TestUtils::compareWithGolden(StackingSnapshot::goldensDir() + "wire_routing.txt",
                                                          StackingSnapshot::stackOrder(scene));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
    const QString difference = StackingSnapshot::compareWithSceneRender(scene);
    QVERIFY2(difference.isEmpty(), qPrintable(difference));
}
