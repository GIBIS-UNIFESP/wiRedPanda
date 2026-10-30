// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestIcPreviewSnapshot.h"

#include <algorithm>

#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStringList>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace IcPreviewSnapshot {

/// The colour the preview renderer fills its background with.
constexpr QRgb kPreviewBackground = 0xff2a2a2a;

QString goldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/IcPreview/";
}

/// Whether \a preview draws anything on its background. The renderer fills the preview opaque, so a check for any
/// alpha would always pass.
bool hasInk(const QPixmap &preview)
{
    const QImage image = preview.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixel(x, y) != kPreviewBackground) {
                return true;
            }
        }
    }
    return false;
}

/**
 * \brief One line per pin of \a ic, inputs then outputs, in its order: the boundary element behind it as numbered in
 * \a circuit, its type, its port and the pin's label.
 * \details The loader puts a proxy Node in each boundary element's place, one per port, appended in port order at the
 * element's position, so a pin's element is the one of \a circuit's boundary elements at that position, and its port
 * is the pin's rank among the pins at that position.
 */
QStringList pinLines(const IC *ic, const QVector<Element *> &circuit)
{
    QStringList lines;
    for (const bool inputs : {true, false}) {
        const ElementGroup group = inputs ? ElementGroup::Input : ElementGroup::Output;
        const auto ports = SnapshotAccess::icPinPorts(ic, inputs);
        QList<QPointF> positions;
        for (int pin = 0; pin < ports.size(); ++pin) {
            const QPointF position = ports.at(pin)->element()->pos();
            QStringList matches;
            QString type;
            for (const auto *element : circuit) {
                if ((element->elementGroup() == group) && (element->pos() == position)) {
                    matches << 'E' + QString::number(element->id());
                    type = TestUtils::snapshotType(element->elementType());
                }
            }
            const QString name = (matches.size() == 1) ? matches.constFirst() + ' ' + type
                                                       : QString("%1 matches").arg(matches.size());
            const Port *icPort = inputs ? static_cast<const Port *>(ic->inputPort(pin)) : ic->outputPort(pin);
            lines << QString("%1 %2: %3 port %4 %5")
                         .arg(inputs ? "in" : "out", QString::number(pin), name,
                              QString::number(positions.count(position)),
                              TestUtils::snapshotQuoted(icPort ? icPort->name() : QString("NO PIN")));
            positions << position;
        }
    }
    return lines;
}

/// The component in \a filePath loaded as an IC, as the application loads one: whether it has a preview and whether
/// the preview draws anything, its port counts and its pins.
QString componentDump(const QString &filePath)
{
    // Wires get their geometry, so the preview holds them as the application's does; with rendering off each wire's
    // empty path would add a rectangle at the scene origin to the preview's bounds.
    TestUtils::ScopedRendering rendering;
    WorkSpace icWorkspace;
    auto *ic = new IC();
    icWorkspace.scene()->addItem(ic);
    ic->loadFile(filePath, QFileInfo(filePath).absolutePath());

    // The component loaded as a circuit numbers its elements, which names the one behind each pin.
    WorkSpace circuitWorkspace;
    TestUtils::loadCircuit(circuitWorkspace, filePath);
    if (QTest::currentTestFailed()) {
        return {};
    }

    const QPixmap &preview = ic->preview();
    QStringList lines{QString("preview null=%1 ink=%2 inputs=%3 outputs=%4")
                          .arg(QString::number(preview.isNull()), QString::number(hasInk(preview)),
                               QString::number(ic->inputSize()), QString::number(ic->outputSize()))};
    lines << pinLines(ic, TestUtils::elementsById(circuitWorkspace.scene()));
    return lines.join('\n') + '\n';
}

/// The bounds of \a item and every item below it, summed as QGraphicsScene::itemsBoundingRect() sums a scene's items.
QRectF treeBounds(const QGraphicsItem *item)
{
    QRectF bounds = item->sceneBoundingRect();
    for (const auto *child : item->childItems()) {
        bounds |= treeBounds(child);
    }
    return bounds;
}

/// The bounds the preview renders of \a scene: its elements with their ports and labels, and its wires.
QRectF previewBounds(const Scene *scene)
{
    QRectF bounds;
    for (const auto *element : scene->unsortedElements()) {
        bounds |= treeBounds(element);
    }
    for (const auto *item : scene->items()) {
        if (const auto *wire = qgraphicsitem_cast<const Connection *>(item)) {
            bounds |= wire->sceneBoundingRect();
        }
    }
    return bounds;
}

/// The preview of an IC loaded from \a bytes, as premultiplied ARGB32.
QImage previewFromBlob(WorkSpace &workspace, const QByteArray &bytes, const QString &contextDir)
{
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ic->loadFromBlob(bytes, contextDir);
    return ic->preview().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

} // namespace IcPreviewSnapshot

void TestIcPreviewSnapshot::testComponentMatchesGolden_data()
{
    QTest::addColumn<QString>("path");
    TestUtils::addFileRows(TestUtils::cpuComponentsDir(), {"*.panda"});
}

void TestIcPreviewSnapshot::testComponentMatchesGolden()
{
    QFETCH(QString, path);
    const QString missing = TestUtils::requireInputFile(path);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));

    const QString dump = IcPreviewSnapshot::componentDump(path);
    if (QTest::currentTestFailed()) {
        return;
    }
    const QString golden = IcPreviewSnapshot::goldensDir() + QFileInfo(path).completeBaseName() + ".txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, dump);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestIcPreviewSnapshot::testEveryGoldenHasAComponent()
{
    // A golden whose component was removed or renamed would otherwise stay in the tree, compared with nothing.
    const auto goldens = QDir(IcPreviewSnapshot::goldensDir()).entryInfoList({"*.txt"}, QDir::Files, QDir::Name);
    QVERIFY(!goldens.isEmpty());
    for (const auto &golden : goldens) {
        const QString component = TestUtils::cpuComponentsDir() + golden.completeBaseName() + ".panda";
        QVERIFY2(QFileInfo::exists(component), qPrintable(golden.fileName() + " has no component"));
    }
}

void TestIcPreviewSnapshot::testPreviewPixelsMatchGolden()
{
    // level2_mux_8to1 draws no text but its labels, which stay inside the circuit's bounds, so its preview's size does
    // not follow the font. Each label is masked where it falls on this platform; the test first checks that the
    // preview draws its labels and that the masks hold every pixel they paint.
    TestUtils::ScopedThemeOverride theme;
    ThemeManager::setTheme(Theme::Light);
    TestUtils::ScopedRendering rendering;
    const QString filePath = TestUtils::cpuComponentsDir() + "level2_mux_8to1.panda";
    const QString contextDir = QFileInfo(filePath).absolutePath();

    WorkSpace icWorkspace;
    auto *ic = new IC();
    icWorkspace.scene()->addItem(ic);
    ic->loadFile(filePath, contextDir);
    QImage preview = ic->preview().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QVERIFY(!preview.isNull());

    // The circuit as designed, which is what the preview renders, gives where each label falls in it.
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, filePath);
    if (QTest::currentTestFailed()) {
        return;
    }
    Scene *scene = workspace.scene();
    const QRectF bounds = IcPreviewSnapshot::previewBounds(scene);
    const QRectF source = bounds.adjusted(-16, -16, 16, 16);
    // The renderer sizes the preview from the same rectangle, so another size means these are not the bounds it rendered.
    QCOMPARE(source.size().toSize().scaled(ICPreviewPopup::MaxWidth, ICPreviewPopup::MaxHeight, Qt::KeepAspectRatio),
             preview.size());

    // QGraphicsScene::render() maps the source onto the image at one scale, top left corner to top left corner.
    const qreal scale = std::min(preview.width() / source.width(), preview.height() / source.height());
    QList<QRect> masks;
    for (const auto *element : scene->unsortedElements()) {
        const QRectF label = element->labelSceneBoundingRect();
        if (!label.isEmpty()) {
            masks << QRectF((label.topLeft() - source.topLeft()) * scale, label.size() * scale)
                         .toAlignedRect()
                         .adjusted(-2, -2, 2, 2);
        }
    }
    QVERIFY(!masks.isEmpty());

    // The preview of the same circuit with its labels emptied, loaded the same way, differs from it only in the masks.
    for (auto *element : scene->unsortedElements()) {
        element->setLabel(QString());
        QVERIFY(element->labelSceneBoundingRect().isEmpty());
    }
    // No label reaches the circuit's bounds, so the preview's size and scale do not follow the platform's font.
    QCOMPARE(IcPreviewSnapshot::previewBounds(scene), bounds);
    QByteArray unlabelledFile;
    QDataStream unlabelledStream(&unlabelledFile, QIODevice::WriteOnly);
    Serialization::writePandaHeader(unlabelledStream);
    workspace.save(unlabelledStream);
    const QImage withoutLabels = IcPreviewSnapshot::previewFromBlob(icWorkspace, unlabelledFile, contextDir);
    QCOMPARE(withoutLabels.size(), preview.size());

    qsizetype labelPixels = 0;
    qsizetype outsideMasks = 0;
    for (int y = 0; y < preview.height(); ++y) {
        for (int x = 0; x < preview.width(); ++x) {
            if (preview.pixel(x, y) == withoutLabels.pixel(x, y)) {
                continue;
            }
            ++labelPixels;
            const QPoint point(x, y);
            if (std::none_of(masks.cbegin(), masks.cend(), [point](const QRect &mask) { return mask.contains(point); })) {
                ++outsideMasks;
            }
        }
    }
    QVERIFY2(labelPixels > 0, "the preview draws no labels");
    QVERIFY2(outsideMasks == 0,
             qPrintable(QString("%1 of the %2 pixels the labels paint lie outside the masks").arg(outsideMasks).arg(labelPixels)));

    QPainter painter(&preview);
    for (const QRect &mask : std::as_const(masks)) {
        painter.fillRect(mask, QColor::fromRgb(TestUtils::kMaskColour));
    }
    painter.end();
    const QString mismatch = TestUtils::compareWithGoldenImage(IcPreviewSnapshot::goldensDir() + "level2_mux_8to1.png", preview);
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
